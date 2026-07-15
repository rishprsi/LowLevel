#!/usr/bin/env bash
# BREAKFIX-LAB 02-disk-mystery :: setup (run in a DISPOSABLE VM only)
set -euo pipefail

[[ $EUID -eq 0 ]] || { echo "ERROR: run as root: sudo ./setup.sh"; exit 1; }
. /etc/os-release
[[ "${ID:-}" == "ubuntu" ]] || { echo "ERROR: refusing to run — this is not Ubuntu."; exit 1; }

LAB_DIR=/opt/breakfix-labs/diskhog

# Refuse if the disk is already tight — this lab writes ~600MB+.
avail_kb=$(df --output=avail / | tail -1 | tr -d ' ')
(( avail_kb > 4000000 )) || { echo "ERROR: need >4GB free on / to run this lab safely."; exit 1; }

# Idempotent: remove any previous instance first
for u in mystery-datastore mystery-applog; do
  systemctl stop "$u.service" 2>/dev/null || true
  systemctl disable "$u.service" 2>/dev/null || true
  rm -f "/etc/systemd/system/$u.service"
  systemctl reset-failed "$u.service" 2>/dev/null || true
done
systemctl daemon-reload
rm -rf "$LAB_DIR"
rm -f /var/log/mystery-app.log /var/log/mystery-datastore.log

mkdir -p "$LAB_DIR"

# Payload 1: writes 600MB, deletes the file, holds the fd open forever.
cat > "$LAB_DIR/ghostwriter.sh" <<'EOF'
#!/usr/bin/env bash
# BREAKFIX-LAB payload: deleted-but-open file
set -u
LOG=/var/log/mystery-datastore.log
dd if=/dev/zero of="$LOG" bs=1M count=600 status=none
exec 3>>"$LOG"
rm -f "$LOG"
while true; do
  echo "datastore checkpoint $(date -Is)" >&3
  sleep 30
done
EOF
chmod +x "$LAB_DIR/ghostwriter.sh"

# Payload 2: runaway logger (safety-capped at 1.5GB so it can't brick the VM)
cat > "$LAB_DIR/chatterbox.sh" <<'EOF'
#!/usr/bin/env bash
# BREAKFIX-LAB payload: runaway log
set -u
LOG=/var/log/mystery-app.log
while true; do
  size=$(stat -c%s "$LOG" 2>/dev/null || echo 0)
  if (( size < 1500000000 )); then
    for _ in $(seq 1 500); do
      echo "$(date -Is) WARN upstream shard unreachable, retrying with backoff (attempt $RANDOM)" >> "$LOG"
    done
  fi
  sleep 1
done
EOF
chmod +x "$LAB_DIR/chatterbox.sh"

cat > /etc/systemd/system/mystery-datastore.service <<'EOF'
# BREAKFIX-LAB
[Unit]
Description=Mystery datastore checkpoint service

[Service]
ExecStart=/opt/breakfix-labs/diskhog/ghostwriter.sh

[Install]
WantedBy=multi-user.target
EOF

cat > /etc/systemd/system/mystery-applog.service <<'EOF'
# BREAKFIX-LAB
[Unit]
Description=Mystery application logger

[Service]
ExecStart=/opt/breakfix-labs/diskhog/chatterbox.sh

[Install]
WantedBy=multi-user.target
EOF

systemctl daemon-reload
systemctl enable --now mystery-datastore.service mystery-applog.service

echo
echo "Lab armed (600MB just vanished; /var/log is filling up)."
echo "Ticket: 'df and du disagree by ~600MB and a log is growing like crazy.'"
echo "Target: 15 minutes. Then: sudo ./check.sh"
