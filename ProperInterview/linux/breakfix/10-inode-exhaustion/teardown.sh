#!/usr/bin/env bash
# BREAKFIX-LAB 10-inode-exhaustion :: teardown
set -euo pipefail
[[ $EUID -eq 0 ]] || { echo "ERROR: sudo ./teardown.sh"; exit 1; }
systemctl disable --now bf-inodeapp.service 2>/dev/null || true
rm -f /etc/systemd/system/bf-inodeapp.service
if mountpoint -q /mnt/bf-data 2>/dev/null; then umount /mnt/bf-data || umount -l /mnt/bf-data; fi
rm -rf /opt/breakfix-labs/inodes /mnt/bf-data
systemctl daemon-reload 2>/dev/null || true
rmdir /opt/breakfix-labs 2>/dev/null || true
echo "Lab 10 torn down."
