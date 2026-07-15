#!/usr/bin/env bash
# BREAKFIX-LAB 11-user-cant-login :: check
set -uo pipefail
GREEN=$'\e[32m'; RED=$'\e[31m'; RESET=$'\e[0m'
fail=0
USER=bf-deploy
KEY=/root/bf-lab-key
[[ -f "$KEY" ]] || KEY=/opt/breakfix-labs/sshlab/bf-lab-key

shell=$(getent passwd "$USER" | cut -d: -f7 || true)
if [[ "$shell" == */nologin ]] || [[ "$shell" == */false ]]; then
  echo "${RED}  FAIL: shell is still $shell${RESET}"; fail=1
else
  echo "  ok: shell is $shell"
fi

mode=$(stat -c '%a' /home/"$USER" 2>/dev/null || echo 777)
# StrictModes: home must not be group- or world-writable
if find /home/"$USER" -maxdepth 0 \( -perm -g=w -o -perm -o=w \) | grep -q .; then
  echo "${RED}  FAIL: /home/$USER is group/world-writable (mode $mode) — StrictModes rejects keys${RESET}"
  fail=1
else
  echo "  ok: home mode $mode (not group/world-writable)"
fi

if [[ ! -f "$KEY" ]]; then
  echo "${RED}  FAIL: test key missing at $KEY${RESET}"; fail=1
else
  if ssh -i "$KEY" -o BatchMode=yes -o StrictHostKeyChecking=no \
       -o UserKnownHostsFile=/dev/null -o ConnectTimeout=5 \
       "$USER"@localhost true 2>/dev/null; then
    echo "  ok: ssh BatchMode login succeeded"
  else
    echo "${RED}  FAIL: ssh -i $KEY $USER@localhost still fails${RESET}"; fail=1
  fi
fi

echo
[[ $fail -eq 0 ]] && { echo "${GREEN}✔ FIXED — key auth works for $USER.${RESET}"; exit 0; }
echo "${RED}✘ Not fixed yet.${RESET}"; exit 1
