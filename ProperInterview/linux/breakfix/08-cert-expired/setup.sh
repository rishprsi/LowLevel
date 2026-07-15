#!/usr/bin/env bash
# BREAKFIX-LAB 08-cert-expired :: setup (DISPOSABLE VM / LXD only)
set -euo pipefail
[[ $EUID -eq 0 ]] || { echo "ERROR: sudo ./setup.sh"; exit 1; }
. /etc/os-release
[[ "${ID:-}" == "ubuntu" ]] || { echo "ERROR: not Ubuntu."; exit 1; }

LAB=/opt/breakfix-labs/tlsapp
UNIT=/etc/systemd/system/bf-tlsapp.service
PORT=18443

systemctl stop bf-tlsapp.service 2>/dev/null || true
systemctl disable bf-tlsapp.service 2>/dev/null || true
rm -f "$UNIT"
rm -rf "$LAB"
systemctl daemon-reload 2>/dev/null || true

mkdir -p "$LAB"
cd "$LAB"

# Expired cert via openssl x509 -startdate/-enddate (OpenSSL 3.x on Ubuntu 24.04+)
openssl req -new -newkey rsa:2048 -nodes \
  -keyout key.pem -out req.pem -subj "/CN=localhost" >/dev/null 2>&1
openssl x509 -req -in req.pem -signkey key.pem -out cert.pem \
  -set_serial 1 \
  -startdate 20200101000000Z \
  -enddate 20200102000000Z >/dev/null 2>&1
rm -f req.pem
chmod 600 key.pem
chmod 644 cert.pem

cat > "$LAB/serve.py" <<'EOF'
#!/usr/bin/env python3
# BREAKFIX-LAB payload — tiny HTTPS server
import http.server, ssl, sys
port = int(sys.argv[1]) if len(sys.argv) > 1 else 18443
httpd = http.server.HTTPServer(("127.0.0.1", port), http.server.SimpleHTTPRequestHandler)
ctx = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
ctx.load_cert_chain(certfile="cert.pem", keyfile="key.pem")
httpd.socket = ctx.wrap_socket(httpd.socket, server_side=True)
print(f"bf-tlsapp listening on https://127.0.0.1:{port}", flush=True)
httpd.serve_forever()
EOF
chmod 755 "$LAB/serve.py"

cat > "$UNIT" <<EOF
# BREAKFIX-LAB
[Unit]
Description=BREAKFIX-LAB expired-cert HTTPS demo
After=network.target

[Service]
Type=simple
WorkingDirectory=$LAB
ExecStart=/usr/bin/python3 $LAB/serve.py $PORT
Restart=on-failure

[Install]
WantedBy=multi-user.target
EOF

systemctl daemon-reload
systemctl enable --now bf-tlsapp.service

echo
echo "Lab 08 armed."
echo "  Symptom: curl https://127.0.0.1:$PORT fails (certificate verify / expired)."
echo "  Diagnose with openssl, renew the cert, restart the service, ./check.sh"
