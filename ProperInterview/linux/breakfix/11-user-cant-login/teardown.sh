#!/usr/bin/env bash
# BREAKFIX-LAB 11-user-cant-login :: teardown
set -euo pipefail
[[ $EUID -eq 0 ]] || { echo "ERROR: sudo ./teardown.sh"; exit 1; }
USER=bf-deploy
userdel -r "$USER" 2>/dev/null || userdel "$USER" 2>/dev/null || true
rm -rf /home/"$USER" /opt/breakfix-labs/sshlab /root/bf-lab-key
rmdir /opt/breakfix-labs 2>/dev/null || true
echo "Lab 11 torn down."
