#!/usr/bin/env bash
# BREAKFIX-LAB 01-service-wont-start :: check
set -uo pipefail
GREEN=$'\e[32m'; RED=$'\e[31m'; RESET=$'\e[0m'
fail=0

if systemctl is-active --quiet dataflux.service; then
  echo "  ok: dataflux.service is active"
else
  echo "${RED}  FAIL: dataflux.service is not active (systemctl status dataflux)${RESET}"
  fail=1
fi

if systemctl is-enabled --quiet dataflux.service 2>/dev/null; then
  echo "  ok: dataflux.service is enabled (survives reboot)"
else
  echo "${RED}  FAIL: dataflux.service is not enabled — the ticket asked for reboot-safe${RESET}"
  fail=1
fi

if grep -q 'network-onlin\.target' /etc/systemd/system/dataflux.service 2>/dev/null; then
  echo "${RED}  FAIL: unit still references the misspelled 'network-onlin.target'${RESET}"
  fail=1
else
  echo "  ok: After=/Wants= target typo is fixed"
fi

echo
if [[ $fail -eq 0 ]]; then
  echo "${GREEN}✔ FIXED — dataflux is running, enabled, and the unit is clean.${RESET}"
  exit 0
else
  echo "${RED}✘ Not fixed yet — see failures above.${RESET}"
  exit 1
fi
