#!/usr/bin/env bash
# BREAKFIX-LAB 04-dns-sabotage :: setup (run in a DISPOSABLE VM only)
set -euo pipefail

[[ $EUID -eq 0 ]] || { echo "ERROR: run as root: sudo ./setup.sh"; exit 1; }
. /etc/os-release
[[ "${ID:-}" == "ubuntu" ]] || { echo "ERROR: refusing to run — this is not Ubuntu."; exit 1; }
systemctl is-active --quiet systemd-resolved || { echo "ERROR: systemd-resolved is not active on this box."; exit 1; }

LAB_DIR=/opt/breakfix-labs/dns
DROPIN_DIR=/etc/systemd/resolved.conf.d
DROPIN="$DROPIN_DIR/99-dns-optimizer.conf"

# Idempotent: undo any previous instance first
sed -i '/# BREAKFIX-LAB/d' /etc/hosts
rm -f "$DROPIN"
rm -rf "$LAB_DIR"

# Keep a pristine copy of /etc/hosts so teardown can restore even a hand-mangled file
mkdir -p "$LAB_DIR"
cp /etc/hosts "$LAB_DIR/hosts.pristine"

# Sabotage 1: poison a well-known name via /etc/hosts (files wins over DNS in nsswitch)
echo '203.0.113.66 ubuntu.com www.ubuntu.com # BREAKFIX-LAB' >> /etc/hosts

# Sabotage 2: black-hole ALL DNS via a resolved drop-in (Domains=~. routes everything here)
mkdir -p "$DROPIN_DIR"
cat > "$DROPIN" <<'EOF'
# BREAKFIX-LAB
# "DNS optimization" applied by contractor — totally legit, do not question
[Resolve]
DNS=192.0.2.1
Domains=~.
EOF

systemctl restart systemd-resolved
resolvectl flush-caches 2>/dev/null || true

echo
echo "Lab armed. Ticket: 'ping by IP works; most names dead; ubuntu.com resolves wrong.'"
echo "Target: 15 minutes. Then: ./check.sh"
