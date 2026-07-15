#!/usr/bin/env bash
# BREAKFIX-LAB 07-cron-never-ran :: check
set -uo pipefail
GREEN=$'\e[32m'; RED=$'\e[31m'; RESET=$'\e[0m'
fail=0
CRON=/etc/cron.d/bf-report
OUT=/var/lib/breakfix-labs/report/out.txt

if [[ -f "$CRON" ]]; then
  if [[ -n "$(tail -c1 "$CRON" 2>/dev/null || true)" ]]; then
    echo "${RED}  FAIL: $CRON missing trailing newline${RESET}"; fail=1
  else
    echo "  ok: $CRON has trailing newline"
  fi
  if grep -q '/usr/local/bin/bf-report' "$CRON"; then
    echo "  ok: cron uses absolute path"
  elif systemctl is-active --quiet bf-report.timer 2>/dev/null; then
    echo "  ok: bf-report.timer active (timer-based fix)"
  else
    echo "${RED}  FAIL: cron still uses short name (or no timer enabled)${RESET}"; fail=1
  fi
else
  if systemctl is-active --quiet bf-report.timer 2>/dev/null; then
    echo "  ok: timer-based fix (no cron.d)"
  else
    echo "${RED}  FAIL: no cron.d and timer not active${RESET}"; fail=1
  fi
fi

[[ -f "$OUT" ]] || /usr/local/bin/bf-report 2>/dev/null || true
if [[ -f "$OUT" ]] && grep -q 'bf-report OK' "$OUT"; then
  echo "  ok: $OUT present"
else
  echo "${RED}  FAIL: $OUT missing — run the fixed job once${RESET}"; fail=1
fi

echo
[[ $fail -eq 0 ]] && { echo "${GREEN}✔ FIXED — report job schedules cleanly.${RESET}"; exit 0; }
echo "${RED}✘ Not fixed yet.${RESET}"; exit 1
