#!/usr/bin/env bash
# BREAKFIX-LAB 03-port-conflict :: teardown (fully reverses setup, any state)
set -euo pipefail

[[ $EUID -eq 0 ]] || { echo "ERROR: run as root: sudo ./teardown.sh"; exit 1; }

for u in dataflux-api mystery-devserver; do
  systemctl stop "$u.service" 2>/dev/null || true
  systemctl disable "$u.service" 2>/dev/null || true
  rm -f "/etc/systemd/system/$u.service"
  systemctl reset-failed "$u.service" 2>/dev/null || true
done
systemctl daemon-reload
rm -rf /opt/breakfix-labs/portconflict
rmdir /opt/breakfix-labs 2>/dev/null || true

echo "Lab 03 torn down."
