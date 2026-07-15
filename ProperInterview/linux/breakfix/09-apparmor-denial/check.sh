#!/usr/bin/env bash
# BREAKFIX-LAB 09-apparmor-denial :: check
set -uo pipefail
GREEN=$'\e[32m'; RED=$'\e[31m'; RESET=$'\e[0m'
fail=0
LOG=/var/lib/bf-app/data.log

if ! systemctl is-active --quiet bf-aaapp.service; then
  echo "${RED}  FAIL: bf-aaapp.service not active${RESET}"; fail=1
else
  echo "  ok: service active"
fi

# Give it a moment to write
sleep 3
if [[ -f "$LOG" ]] && [[ -s "$LOG" ]]; then
  echo "  ok: $LOG is being written"
else
  echo "${RED}  FAIL: $LOG missing or empty — writes still denied${RESET}"; fail=1
fi

# Fresh denials while running = still broken
if journalctl -k --since "30 seconds ago" 2>/dev/null | grep -qi 'apparmor.*DENIED.*bf-app\|apparmor.*DENIED.*writer.sh'; then
  echo "${RED}  FAIL: fresh AppArmor DENIED lines in the last 30s${RESET}"; fail=1
else
  echo "  ok: no fresh AppArmor DENIED for this app"
fi

echo
[[ $fail -eq 0 ]] && { echo "${GREEN}✔ FIXED — service writes with no AppArmor denials.${RESET}"; exit 0; }
echo "${RED}✘ Not fixed yet.${RESET}"; exit 1
