#!/usr/bin/env bash
# BREAKFIX-LAB 10-inode-exhaustion :: check
set -uo pipefail
GREEN=$'\e[32m'; RED=$'\e[31m'; RESET=$'\e[0m'
fail=0
MNT=/mnt/bf-data

if ! mountpoint -q "$MNT" 2>/dev/null; then
  echo "${RED}  FAIL: $MNT not mounted — did you tear down early? re-run setup${RESET}"; fail=1
else
  echo "  ok: $MNT mounted"
  # inode use should be under 90%
  ipct=$(df -i "$MNT" | awk 'NR==2 {gsub(/%/,""); print $5}')
  if [[ "${ipct:-100}" -lt 90 ]]; then
    echo "  ok: inode use ${ipct}% (<90%)"
  else
    echo "${RED}  FAIL: inode use still ${ipct}% — delete the spray files${RESET}"; fail=1
  fi
  if echo "check $(date -Is)" > "$MNT/check-write.log" 2>/dev/null; then
    echo "  ok: test write succeeded"
    rm -f "$MNT/check-write.log"
  else
    echo "${RED}  FAIL: test write still fails${RESET}"; fail=1
  fi
fi

echo
[[ $fail -eq 0 ]] && { echo "${GREEN}✔ FIXED — inodes freed and writes work.${RESET}"; exit 0; }
echo "${RED}✘ Not fixed yet.${RESET}"; exit 1
