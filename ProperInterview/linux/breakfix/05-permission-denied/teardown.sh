#!/usr/bin/env bash
# BREAKFIX-LAB 05-permission-denied :: teardown (fully reverses setup, any state)
set -euo pipefail

[[ $EUID -eq 0 ]] || { echo "ERROR: run as root: sudo ./teardown.sh"; exit 1; }

systemctl stop dataflux-agent.service 2>/dev/null || true
systemctl disable dataflux-agent.service 2>/dev/null || true
rm -f /etc/systemd/system/dataflux-agent.service
rm -rf /etc/systemd/system/dataflux-agent.service.d
systemctl daemon-reload
systemctl reset-failed dataflux-agent.service 2>/dev/null || true

rm -rf /var/lib/dataflux-agent /opt/breakfix-labs/permdenied
userdel dataflux 2>/dev/null || true
rmdir /opt/breakfix-labs 2>/dev/null || true

echo "Lab 05 torn down."
