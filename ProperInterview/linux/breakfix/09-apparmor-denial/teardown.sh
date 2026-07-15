#!/usr/bin/env bash
# BREAKFIX-LAB 09-apparmor-denial :: teardown
set -euo pipefail
[[ $EUID -eq 0 ]] || { echo "ERROR: sudo ./teardown.sh"; exit 1; }
PROFILE=/etc/apparmor.d/opt.breakfix-labs.aaapp.writer
systemctl disable --now bf-aaapp.service 2>/dev/null || true
rm -f /etc/systemd/system/bf-aaapp.service
apparmor_parser -R "$PROFILE" 2>/dev/null || true
rm -f "$PROFILE"
rm -rf /opt/breakfix-labs/aaapp /var/lib/bf-app
systemctl daemon-reload 2>/dev/null || true
rmdir /opt/breakfix-labs 2>/dev/null || true
echo "Lab 09 torn down."
