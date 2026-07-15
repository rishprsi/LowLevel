#!/usr/bin/env bash
# BREAKFIX-LAB 01-service-wont-start :: setup (run in a DISPOSABLE VM only)
set -euo pipefail

[[ $EUID -eq 0 ]] || { echo "ERROR: run as root: sudo ./setup.sh"; exit 1; }
. /etc/os-release
[[ "${ID:-}" == "ubuntu" ]] || { echo "ERROR: refusing to run — this is not Ubuntu."; exit 1; }

LAB_DIR=/opt/breakfix-labs/dataflux
UNIT=/etc/systemd/system/dataflux.service

# Idempotent: wipe any previous instance of this lab first
systemctl stop dataflux.service 2>/dev/null || true
systemctl disable dataflux.service 2>/dev/null || true
rm -f "$UNIT"
rm -rf "$LAB_DIR"
systemctl daemon-reload
systemctl reset-failed dataflux.service 2>/dev/null || true

# Real payload lives at a slightly different path than the unit references
mkdir -p "$LAB_DIR/bin"
cat > "$LAB_DIR/bin/dataflux-worker.sh" <<'EOF'
#!/usr/bin/env bash
# BREAKFIX-LAB payload
while true; do
  echo "dataflux: processed batch at $(date -Is)"
  sleep 10
done
EOF
chmod +x "$LAB_DIR/bin/dataflux-worker.sh"

# Broken unit: nonexistent ExecStart path AND a typo'd After=/Wants= target
cat > "$UNIT" <<'EOF'
# BREAKFIX-LAB
[Unit]
Description=DataFlux batch worker
After=network-onlin.target
Wants=network-onlin.target

[Service]
ExecStart=/opt/breakfix-labs/dataflux/dataflux-worker.sh
Restart=on-failure
RestartSec=5

[Install]
WantedBy=multi-user.target
EOF

systemctl daemon-reload
systemctl start dataflux.service 2>/dev/null || true

echo
echo "Lab armed. Ticket: 'dataflux was deployed last night and never came up.'"
echo "Target: 10 minutes. Make it active AND enabled, then run ./check.sh"
