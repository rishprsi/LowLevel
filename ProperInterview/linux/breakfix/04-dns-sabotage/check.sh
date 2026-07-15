#!/usr/bin/env bash
# BREAKFIX-LAB 04-dns-sabotage :: check
set -uo pipefail
GREEN=$'\e[32m'; RED=$'\e[31m'; RESET=$'\e[0m'
fail=0

if grep -q '203\.0\.113\.66' /etc/hosts; then
  echo "${RED}  FAIL: /etc/hosts still contains the poisoned ubuntu.com entry${RESET}"
  fail=1
else
  echo "  ok: /etc/hosts is clean"
fi

if [[ -f /etc/systemd/resolved.conf.d/99-dns-optimizer.conf ]]; then
  echo "${RED}  FAIL: the resolved drop-in 99-dns-optimizer.conf is still in place${RESET}"
  fail=1
else
  echo "  ok: black-hole resolved drop-in removed"
fi

if resolvectl status 2>/dev/null | grep -q '192\.0\.2\.1'; then
  echo "${RED}  FAIL: resolved still lists 192.0.2.1 — did you restart systemd-resolved?${RESET}"
  fail=1
else
  echo "  ok: resolved no longer points at the black hole"
fi

addr=$(getent hosts ubuntu.com 2>/dev/null | awk '{print $1; exit}')
if [[ -z "$addr" ]]; then
  echo "${RED}  FAIL: ubuntu.com does not resolve at all (check resolvectl status / VM network)${RESET}"
  fail=1
elif [[ "$addr" == "203.0.113.66" ]]; then
  echo "${RED}  FAIL: ubuntu.com still resolves to the poisoned 203.0.113.66${RESET}"
  fail=1
else
  echo "  ok: ubuntu.com resolves to a real address ($addr)"
fi

echo
if [[ $fail -eq 0 ]]; then
  echo "${GREEN}✔ FIXED — hosts file clean, resolved restored, names resolve for real.${RESET}"
  exit 0
else
  echo "${RED}✘ Not fixed yet — see failures above.${RESET}"
  exit 1
fi
