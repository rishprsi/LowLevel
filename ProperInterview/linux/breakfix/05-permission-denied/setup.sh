#!/usr/bin/env bash
# BREAKFIX-LAB 05-permission-denied :: setup (run in a DISPOSABLE VM only)
set -euo pipefail

[[ $EUID -eq 0 ]] || { echo "ERROR: run as root: sudo ./setup.sh"; exit 1; }
. /etc/os-release
[[ "${ID:-}" == "ubuntu" ]] || { echo "ERROR: refusing to run — this is not Ubuntu."; exit 1; }

LAB_DIR=/opt/breakfix-labs/permdenied
STATE_DIR=/var/lib/dataflux-agent
UNIT=/etc/systemd/system/dataflux-agent.service

# Idempotent: remove any previous instance first
systemctl stop dataflux-agent.service 2>/dev/null || true
systemctl disable dataflux-agent.service 2>/dev/null || true
rm -f "$UNIT"
rm -rf /etc/systemd/system/dataflux-agent.service.d
systemctl daemon-reload
systemctl reset-failed dataflux-agent.service 2>/dev/null || true
rm -rf "$LAB_DIR" "$STATE_DIR"

# Dedicated system user (idempotent)
id dataflux &>/dev/null || useradd --system --shell /usr/sbin/nologin --home-dir /nonexistent dataflux

mkdir -p "$LAB_DIR"
cat > "$LAB_DIR/agent.sh" <<'EOF'
#!/usr/bin/env bash
# BREAKFIX-LAB payload: writes state every 5s, exits nonzero on failure
set -u
STATE=/var/lib/dataflux-agent/state.json
while true; do
  if ! printf '{"ts":"%s","status":"healthy"}\n' "$(date -Is)" > "$STATE" 2>/dev/null; then
    echo "cannot create $STATE: Permission denied" >&2
    exit 1
  fi
  echo "state checkpoint written"
  sleep 5
done
EOF
chmod 755 "$LAB_DIR/agent.sh"

cat > "$UNIT" <<'EOF'
# BREAKFIX-LAB
[Unit]
Description=DataFlux monitoring agent
After=network.target

[Service]
User=dataflux
Group=dataflux
ExecStart=/opt/breakfix-labs/permdenied/agent.sh
Restart=on-failure
RestartSec=5

[Install]
WantedBy=multi-user.target
EOF

# The "hardening pass": state dir owned by root with mode 700
mkdir -p "$STATE_DIR"
chown root:root "$STATE_DIR"
chmod 700 "$STATE_DIR"

systemctl daemon-reload
systemctl enable dataflux-agent.service
systemctl start dataflux-agent.service 2>/dev/null || true

echo
echo "Lab armed. Ticket: 'dataflux-agent down since the security hardening pass.'"
echo "Target: 10 minutes. Then: ./check.sh"
