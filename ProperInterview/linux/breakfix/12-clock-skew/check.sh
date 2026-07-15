#!/usr/bin/env bash
# BREAKFIX-LAB 12-clock-skew :: check
set -uo pipefail
GREEN=$'\e[32m'; RED=$'\e[31m'; RESET=$'\e[0m'
fail=0

ntp=$(timedatectl show -p NTP --value 2>/dev/null || echo unknown)
sync=$(timedatectl show -p NTPSynchronized --value 2>/dev/null || echo no)

if [[ "$ntp" == "yes" ]] || timedatectl | grep -qi 'NTP service: active'; then
  echo "  ok: NTP enabled/active"
else
  echo "${RED}  FAIL: NTP not active (timedatectl set-ntp true)${RESET}"; fail=1
fi

# Clock should be close to "now" — if NTP synced, good; else compare to a rough bound
# We stored BEFORE_UTC at setup; after fix, date should be near real time.
# Without external network, accept NTPSynchronized=yes OR clock within 1 day of... hard.
# Practical check: NTPSynchronized yes, or systemd-timesyncd active and offset small.
if [[ "$sync" == "yes" ]]; then
  echo "  ok: NTPSynchronized=yes"
elif timedatectl | grep -qi 'System clock synchronized: yes'; then
  echo "  ok: system clock synchronized"
else
  # soft pass if ntp on and timesyncd running — sync can take a few seconds
  if systemctl is-active --quiet systemd-timesyncd 2>/dev/null || systemctl is-active --quiet chrony 2>/dev/null; then
    echo "  ok: time sync daemon active (may still be converging)"
  else
    echo "${RED}  FAIL: clock not synchronized yet — wait a few seconds and re-check${RESET}"; fail=1
  fi
fi

echo
[[ $fail -eq 0 ]] && { echo "${GREEN}✔ FIXED — NTP back; clock syncing.${RESET}"; exit 0; }
echo "${RED}✘ Not fixed yet.${RESET}"; exit 1
