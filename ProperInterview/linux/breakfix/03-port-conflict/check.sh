#!/usr/bin/env bash
# BREAKFIX-LAB 03-port-conflict :: check
set -uo pipefail
GREEN=$'\e[32m'; RED=$'\e[31m'; RESET=$'\e[0m'
fail=0

if systemctl is-active --quiet dataflux-api.service; then
  echo "  ok: dataflux-api.service is active"
else
  echo "${RED}  FAIL: dataflux-api.service is not active${RESET}"
  fail=1
fi

if curl -sf --max-time 3 http://127.0.0.1:8080/ 2>/dev/null | grep -q 'dataflux-api ok'; then
  echo "  ok: the REAL service is answering on :8080"
else
  echo "${RED}  FAIL: :8080 is not answering as dataflux-api (squatter still there, or API down)${RESET}"
  fail=1
fi

if systemctl is-active --quiet mystery-devserver.service; then
  echo "${RED}  FAIL: mystery-devserver.service is still running${RESET}"
  fail=1
else
  echo "  ok: mystery-devserver.service is stopped"
fi

if systemctl is-enabled --quiet mystery-devserver.service 2>/dev/null; then
  echo "${RED}  FAIL: mystery-devserver is still ENABLED — it comes back on reboot${RESET}"
  fail=1
else
  echo "  ok: mystery-devserver is disabled (won't return after reboot)"
fi

echo
if [[ $fail -eq 0 ]]; then
  echo "${GREEN}✔ FIXED — dataflux-api owns :8080 and the squatter is gone for good.${RESET}"
  exit 0
else
  echo "${RED}✘ Not fixed yet — see failures above.${RESET}"
  exit 1
fi
