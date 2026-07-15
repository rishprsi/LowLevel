#!/usr/bin/env bash
# BREAKFIX-LAB 05-permission-denied :: check
set -uo pipefail
GREEN=$'\e[32m'; RED=$'\e[31m'; RESET=$'\e[0m'
fail=0

if systemctl is-active --quiet dataflux-agent.service; then
  echo "  ok: dataflux-agent.service is active"
else
  echo "${RED}  FAIL: dataflux-agent.service is not active${RESET}"
  fail=1
fi

STATE=/var/lib/dataflux-agent/state.json
if [[ -f "$STATE" ]]; then
  age=$(( $(date +%s) - $(stat -c %Y "$STATE" 2>/dev/null || echo 0) ))
  if (( age <= 30 )); then
    echo "  ok: state.json exists and was written in the last 30s"
  else
    echo "${RED}  FAIL: state.json exists but is stale (${age}s old) — is the agent really writing?${RESET}"
    fail=1
  fi
else
  echo "${RED}  FAIL: $STATE does not exist — the agent still can't write its state${RESET}"
  fail=1
fi

# Accept either fix: correct ownership, or StateDirectory= (systemd chowns it too)
owner=$(stat -c %U /var/lib/dataflux-agent 2>/dev/null || echo missing)
if [[ "$owner" == "dataflux" ]]; then
  echo "  ok: /var/lib/dataflux-agent is owned by the service user"
else
  echo "${RED}  FAIL: /var/lib/dataflux-agent owner is '$owner', service runs as 'dataflux'${RESET}"
  fail=1
fi

echo
if [[ $fail -eq 0 ]]; then
  echo "${GREEN}✔ FIXED — agent is up and writing state as its own user.${RESET}"
  echo "  Bonus points if you used StateDirectory= instead of a manual chown."
  exit 0
else
  echo "${RED}✘ Not fixed yet — see failures above.${RESET}"
  exit 1
fi
