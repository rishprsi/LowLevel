#!/usr/bin/env bash
# BREAKFIX-LAB 11-user-cant-login :: setup (DISPOSABLE VM / LXD only)
set -euo pipefail
[[ $EUID -eq 0 ]] || { echo "ERROR: sudo ./setup.sh"; exit 1; }
. /etc/os-release
[[ "${ID:-}" == "ubuntu" ]] || { echo "ERROR: not Ubuntu."; exit 1; }

USER=bf-deploy
LAB=/opt/breakfix-labs/sshlab
KEY="$LAB/bf-lab-key"

# Ensure sshd exists
if ! systemctl is-active --quiet ssh 2>/dev/null && ! systemctl is-active --quiet sshd 2>/dev/null; then
  echo "WARNING: ssh service not active — lab still plants the account; start ssh if needed."
fi

rm -rf "$LAB"
mkdir -p "$LAB"

if id "$USER" >/dev/null 2>&1; then
  userdel -r "$USER" 2>/dev/null || userdel "$USER" 2>/dev/null || true
  rm -rf /home/"$USER"
fi

useradd -m -s /bin/bash "$USER"
ssh-keygen -t ed25519 -N '' -f "$KEY" -C "breakfix-lab" >/dev/null
mkdir -p /home/"$USER"/.ssh
cp "$KEY.pub" /home/"$USER"/.ssh/authorized_keys
# Plant classic StrictModes break: group-writable home (and loose .ssh)
chown -R "$USER:$USER" /home/"$USER"
chmod 775 /home/"$USER"                 # too open — sshd StrictModes rejects keys
chmod 755 /home/"$USER"/.ssh
chmod 644 /home/"$USER"/.ssh/authorized_keys
# Second bug: nologin shell
chsh -s /usr/sbin/nologin "$USER"

# Convenience copy for root to test
cp "$KEY" /root/bf-lab-key
chmod 600 /root/bf-lab-key

echo
echo "Lab 11 armed."
echo "  User: $USER"
echo "  Test key: /root/bf-lab-key  (also $KEY)"
echo "  Symptom: ssh -i /root/bf-lab-key $USER@localhost fails."
echo "  Do NOT weaken sshd_config — fix the account."
