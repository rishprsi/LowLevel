#!/usr/bin/env bash
# BREAKFIX-LAB 08-cert-expired :: teardown
set -euo pipefail
[[ $EUID -eq 0 ]] || { echo "ERROR: sudo ./teardown.sh"; exit 1; }
systemctl disable --now bf-tlsapp.service 2>/dev/null || true
rm -f /etc/systemd/system/bf-tlsapp.service
rm -rf /opt/breakfix-labs/tlsapp
systemctl daemon-reload 2>/dev/null || true
systemctl reset-failed bf-tlsapp.service 2>/dev/null || true
rmdir /opt/breakfix-labs 2>/dev/null || true
echo "Lab 08 torn down."
