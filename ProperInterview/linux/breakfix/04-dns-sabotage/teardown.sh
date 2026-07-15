#!/usr/bin/env bash
# BREAKFIX-LAB 04-dns-sabotage :: teardown (fully reverses setup, any state)
set -euo pipefail

[[ $EUID -eq 0 ]] || { echo "ERROR: run as root: sudo ./teardown.sh"; exit 1; }

# Restore pristine hosts file if we have it; otherwise just strip our marker lines
if [[ -f /opt/breakfix-labs/dns/hosts.pristine ]]; then
  cp /opt/breakfix-labs/dns/hosts.pristine /etc/hosts
else
  sed -i '/# BREAKFIX-LAB/d' /etc/hosts
fi

rm -f /etc/systemd/resolved.conf.d/99-dns-optimizer.conf
rmdir /etc/systemd/resolved.conf.d 2>/dev/null || true
systemctl restart systemd-resolved 2>/dev/null || true
resolvectl flush-caches 2>/dev/null || true

rm -rf /opt/breakfix-labs/dns
rmdir /opt/breakfix-labs 2>/dev/null || true

echo "Lab 04 torn down."
