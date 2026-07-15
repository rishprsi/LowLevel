#!/usr/bin/env bash
# BREAKFIX-LAB 09-apparmor-denial :: setup (DISPOSABLE VM; needs AppArmor)
set -euo pipefail
[[ $EUID -eq 0 ]] || { echo "ERROR: sudo ./setup.sh"; exit 1; }
. /etc/os-release
[[ "${ID:-}" == "ubuntu" ]] || { echo "ERROR: not Ubuntu."; exit 1; }

if ! command -v aa-status >/dev/null 2>&1 || ! aa-status >/dev/null 2>&1; then
  echo "ERROR: AppArmor not available (common in nested containers)."
  echo "       Run this lab on the VM/host, not inside an unprivileged LXD container."
  exit 1
fi

LAB=/opt/breakfix-labs/aaapp
DATA=/var/lib/bf-app
UNIT=/etc/systemd/system/bf-aaapp.service
PROFILE=/etc/apparmor.d/opt.breakfix-labs.aaapp.writer

systemctl stop bf-aaapp.service 2>/dev/null || true
systemctl disable bf-aaapp.service 2>/dev/null || true
rm -f "$UNIT" "$PROFILE"
rm -rf "$LAB" "$DATA"
apparmor_parser -R "$PROFILE" 2>/dev/null || true
systemctl daemon-reload 2>/dev/null || true

mkdir -p "$LAB" "$DATA"
chmod 755 "$DATA"

cat > "$LAB/writer.sh" <<'EOF'
#!/usr/bin/env bash
# BREAKFIX-LAB payload
set -euo pipefail
while true; do
  echo "tick $(date -Is)" >> /var/lib/bf-app/data.log || {
    echo "WRITE FAILED: $?" >&2
    sleep 2
    continue
  }
  sleep 2
done
EOF
chmod 755 "$LAB/writer.sh"

# Profile allows execute + read, DENIES write to the data path
cat > "$PROFILE" <<'EOF'
# BREAKFIX-LAB
#include <tunables/global>

/opt/breakfix-labs/aaapp/writer.sh {
  #include <abstractions/base>
  #include <abstractions/bash>

  /opt/breakfix-labs/aaapp/writer.sh r,
  /usr/bin/bash ix,
  /usr/bin/date ix,
  /usr/bin/echo ix,
  /bin/bash ix,
  /bin/date ix,
  /bin/echo ix,

  /var/lib/bf-app/ r,
  # deliberate denial — no write rule for data.log
  deny /var/lib/bf-app/data.log w,
}
EOF

apparmor_parser -r "$PROFILE"

cat > "$UNIT" <<EOF
# BREAKFIX-LAB
[Unit]
Description=BREAKFIX-LAB AppArmor-denied writer
After=network.target

[Service]
Type=simple
ExecStart=$LAB/writer.sh
Restart=always
RestartSec=2

[Install]
WantedBy=multi-user.target
EOF

systemctl daemon-reload
systemctl enable --now bf-aaapp.service
sleep 1

echo
echo "Lab 09 armed."
echo "  Symptom: bf-aaapp can't write /var/lib/bf-app/data.log — POSIX perms look fine."
echo "  Check the kernel journal for DENIED before blaming ownership."
