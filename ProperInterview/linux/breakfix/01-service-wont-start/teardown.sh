#!/usr/bin/env bash
# BREAKFIX-LAB 01-service-wont-start :: teardown (fully reverses setup, any state)
set -euo pipefail

[[ $EUID -eq 0 ]] || { echo "ERROR: run as root: sudo ./teardown.sh"; exit 1; }

systemctl stop dataflux.service 2>/dev/null || true
systemctl disable dataflux.service 2>/dev/null || true
rm -f /etc/systemd/system/dataflux.service
rm -rf /opt/breakfix-labs/dataflux
systemctl daemon-reload
systemctl reset-failed dataflux.service 2>/dev/null || true
rmdir /opt/breakfix-labs 2>/dev/null || true

echo "Lab 01 torn down."
