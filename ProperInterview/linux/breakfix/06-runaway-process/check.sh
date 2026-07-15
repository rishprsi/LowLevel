#!/usr/bin/env bash
# BREAKFIX-LAB 06-runaway-process :: check (waits past the respawn interval)
set -uo pipefail
GREEN=$'\e[32m'; RED=$'\e[31m'; RESET=$'\e[0m'

[[ $EUID -eq 0 ]] || { echo "Run as root: sudo ./check.sh"; exit 1; }
fail=0

if [[ -f /etc/cron.d/mystery-maintenance ]]; then
  echo "${RED}  FAIL: /etc/cron.d/mystery-maintenance still exists — the respawner is armed${RESET}"
  fail=1
else
  echo "  ok: cron respawner entry removed"
fi

if [[ -d /opt/breakfix-labs/hogspawn ]]; then
  echo "${RED}  FAIL: /opt/breakfix-labs/hogspawn still exists — remove the payload too${RESET}"
  fail=1
else
  echo "  ok: spawner payload directory removed"
fi

if pgrep -f 'sysd-cache' >/dev/null; then
  echo "${RED}  FAIL: the hog is currently running (pgrep -f sysd-cache)${RESET}"
  fail=1
else
  echo "  ok: no hog process running right now"
fi

if [[ $fail -ne 0 ]]; then
  echo
  echo "${RED}✘ Not fixed yet — skipping the respawn watch until the above pass.${RESET}"
  exit 1
fi

echo
echo "  Source removed and nothing running. Watching 150s for a respawn"
echo "  (cron fires every 2 minutes — a kill-only fix would resurface here)..."
for i in $(seq 1 15); do
  sleep 10
  if pgrep -f 'sysd-cache' >/dev/null; then
    echo "${RED}✘ FAIL: the hog RESPAWNED after $((i * 10))s — you killed it but missed a source.${RESET}"
    exit 1
  fi
  printf '.'
done
echo

echo "${GREEN}✔ FIXED — hog dead, source removed, and it stayed dead past the respawn window.${RESET}"
exit 0
