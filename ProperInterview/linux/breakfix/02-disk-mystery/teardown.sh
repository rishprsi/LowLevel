#!/usr/bin/env bash
# BREAKFIX-LAB 02-disk-mystery :: teardown (fully reverses setup, any state)
set -euo pipefail

[[ $EUID -eq 0 ]] || { echo "ERROR: run as root: sudo ./teardown.sh"; exit 1; }

for u in mystery-datastore mystery-applog; do
  systemctl stop "$u.service" 2>/dev/null || true
  systemctl disable "$u.service" 2>/dev/null || true
  rm -f "/etc/systemd/system/$u.service"
  systemctl reset-failed "$u.service" 2>/dev/null || true
done
systemctl daemon-reload
pkill -f /opt/breakfix-labs/diskhog/ 2>/dev/null || true
rm -rf /opt/breakfix-labs/diskhog
rm -f /var/log/mystery-app.log /var/log/mystery-datastore.log
rmdir /opt/breakfix-labs 2>/dev/null || true

echo "Lab 02 torn down."
