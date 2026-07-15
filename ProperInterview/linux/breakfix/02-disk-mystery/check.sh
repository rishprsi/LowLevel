#!/usr/bin/env bash
# BREAKFIX-LAB 02-disk-mystery :: check (needs root to scan /proc/*/fd)
set -uo pipefail
GREEN=$'\e[32m'; RED=$'\e[31m'; RESET=$'\e[0m'

[[ $EUID -eq 0 ]] || { echo "Run as root (it must scan all of /proc): sudo ./check.sh"; exit 1; }
fail=0

# 1. No large deleted-but-open files anywhere
big_deleted=""
for fd in /proc/[0-9]*/fd/*; do
  target=$(readlink "$fd" 2>/dev/null) || continue
  [[ "$target" == *'(deleted)'* ]] || continue
  size=$(stat -Lc%s "$fd" 2>/dev/null || echo 0)
  if (( size > 104857600 )); then
    pid=${fd#/proc/}; pid=${pid%%/*}
    big_deleted="PID $pid holds $target ($((size / 1048576))MB)"
    break
  fi
done
if [[ -z "$big_deleted" ]]; then
  echo "  ok: no large deleted-but-open files remain"
else
  echo "${RED}  FAIL: invisible space still held — $big_deleted${RESET}"
  fail=1
fi

# 2. Runaway logger stopped
if systemctl is-active --quiet mystery-applog.service; then
  echo "${RED}  FAIL: mystery-applog.service is still running (still bleeding)${RESET}"
  fail=1
else
  echo "  ok: mystery-applog.service is stopped"
fi

# 3. The runaway log has been shrunk (not just left huge, and not rm'd while open)
size=$(stat -c%s /var/log/mystery-app.log 2>/dev/null || echo 0)
if (( size < 52428800 )); then
  echo "  ok: /var/log/mystery-app.log is under 50MB"
else
  echo "${RED}  FAIL: /var/log/mystery-app.log is still $((size / 1048576))MB — truncate it${RESET}"
  fail=1
fi

echo
if [[ $fail -eq 0 ]]; then
  echo "${GREEN}✔ FIXED — space recovered, bleeding stopped, log shrunk properly.${RESET}"
  exit 0
else
  echo "${RED}✘ Not fixed yet — see failures above.${RESET}"
  exit 1
fi
