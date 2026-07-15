#!/usr/bin/env bash
# BREAKFIX-LAB 06-runaway-process :: teardown (fully reverses setup, any state)
set -euo pipefail

[[ $EUID -eq 0 ]] || { echo "ERROR: run as root: sudo ./teardown.sh"; exit 1; }

rm -f /etc/cron.d/mystery-maintenance
pkill -f 'sysd-cache' 2>/dev/null || true
rm -rf /opt/breakfix-labs/hogspawn /tmp/.sysd-cache
rmdir /opt/breakfix-labs 2>/dev/null || true

echo "Lab 06 torn down."
