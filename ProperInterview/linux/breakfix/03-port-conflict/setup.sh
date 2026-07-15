#!/usr/bin/env bash
# BREAKFIX-LAB 03-port-conflict :: setup (run in a DISPOSABLE VM only)
set -euo pipefail

[[ $EUID -eq 0 ]] || { echo "ERROR: run as root: sudo ./setup.sh"; exit 1; }
. /etc/os-release
[[ "${ID:-}" == "ubuntu" ]] || { echo "ERROR: refusing to run — this is not Ubuntu."; exit 1; }
command -v python3 >/dev/null || { echo "ERROR: python3 required (default on Ubuntu 24.04)."; exit 1; }

LAB_DIR=/opt/breakfix-labs/portconflict

# Idempotent: remove any previous instance first
for u in dataflux-api mystery-devserver; do
  systemctl stop "$u.service" 2>/dev/null || true
  systemctl disable "$u.service" 2>/dev/null || true
  rm -f "/etc/systemd/system/$u.service"
  systemctl reset-failed "$u.service" 2>/dev/null || true
done
systemctl daemon-reload
rm -rf "$LAB_DIR"

mkdir -p "$LAB_DIR"

# The "real" API: binds 8080, dies with EADDRINUSE if the port is taken
cat > "$LAB_DIR/dataflux-api.py" <<'EOF'
# BREAKFIX-LAB payload: the real API service
import http.server


class Handler(http.server.BaseHTTPRequestHandler):
    def do_GET(self):
        self.send_response(200)
        self.send_header("Content-Type", "text/plain")
        self.end_headers()
        self.wfile.write(b"dataflux-api ok\n")

    def log_message(self, *args):
        pass


http.server.HTTPServer(("0.0.0.0", 8080), Handler).serve_forever()
EOF

cat > /etc/systemd/system/dataflux-api.service <<'EOF'
# BREAKFIX-LAB
[Unit]
Description=DataFlux API service (port 8080)
After=network.target

[Service]
ExecStart=/usr/bin/python3 /opt/breakfix-labs/portconflict/dataflux-api.py
Restart=on-failure
RestartSec=3

[Install]
WantedBy=multi-user.target
EOF

# The squatter: a forgotten debug http.server on the same port, ENABLED
cat > /etc/systemd/system/mystery-devserver.service <<'EOF'
# BREAKFIX-LAB
[Unit]
Description=Temporary dev file server (TODO: remove after maintenance)
After=network.target

[Service]
ExecStart=/usr/bin/python3 -m http.server 8080 --directory /tmp
Restart=on-failure

[Install]
WantedBy=multi-user.target
EOF

systemctl daemon-reload
# Squatter grabs the port first, then the real service crash-loops
systemctl enable --now mystery-devserver.service
sleep 2
systemctl enable dataflux-api.service
systemctl start dataflux-api.service 2>/dev/null || true

echo
echo "Lab armed. Ticket: 'dataflux-api on :8080 keeps restarting since maintenance.'"
echo "Target: 10 minutes. Fix must survive a reboot. Then: ./check.sh"
