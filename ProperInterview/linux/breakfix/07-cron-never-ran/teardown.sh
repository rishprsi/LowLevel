#!/usr/bin/env bash
# BREAKFIX-LAB 07-cron-never-ran :: teardown
set -euo pipefail
[[ $EUID -eq 0 ]] || { echo "ERROR: sudo ./teardown.sh"; exit 1; }
systemctl disable --now bf-report.timer 2>/dev/null || true
systemctl stop bf-report.service 2>/dev/null || true
rm -f /etc/cron.d/bf-report /etc/systemd/system/bf-report.service \
      /etc/systemd/system/bf-report.timer /usr/local/bin/bf-report
rm -rf /opt/breakfix-labs/cronjob /var/lib/breakfix-labs/report
systemctl daemon-reload 2>/dev/null || true
rmdir /var/lib/breakfix-labs /opt/breakfix-labs 2>/dev/null || true
echo "Lab 07 torn down."
