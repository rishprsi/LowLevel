#!/usr/bin/env bash
# BREAKFIX-LAB 10-inode-exhaustion :: setup (DISPOSABLE VM / LXD only)
set -euo pipefail
[[ $EUID -eq 0 ]] || { echo "ERROR: sudo ./setup.sh"; exit 1; }
. /etc/os-release
[[ "${ID:-}" == "ubuntu" ]] || { echo "ERROR: not Ubuntu."; exit 1; }

LAB=/opt/breakfix-labs/inodes
IMG="$LAB/fs.img"
MNT=/mnt/bf-data
UNIT=/etc/systemd/system/bf-inodeapp.service

systemctl stop bf-inodeapp.service 2>/dev/null || true
systemctl disable bf-inodeapp.service 2>/dev/null || true
rm -f "$UNIT"
if mountpoint -q "$MNT" 2>/dev/null; then umount "$MNT" || umount -l "$MNT"; fi
rm -rf "$LAB" "$MNT"
systemctl daemon-reload 2>/dev/null || true

mkdir -p "$LAB" "$MNT"
# Small FS with very few inodes
dd if=/dev/zero of="$IMG" bs=1M count=64 status=none
mkfs.ext4 -q -N 1024 -F "$IMG"
mount -o loop "$IMG" "$MNT"

# Fill inodes with empty files
mkdir -p "$MNT/spray"
# leave a handful free so the FS mounts; fill most
for i in $(seq 1 900); do
  : > "$MNT/spray/f-$i" || break
done

cat > "$LAB/writer.sh" <<'EOF'
#!/usr/bin/env bash
# BREAKFIX-LAB payload — tries to write a new file on the full-inode FS
set -euo pipefail
while true; do
  if echo "ok $(date -Is)" > /mnt/bf-data/app-write.log 2>/tmp/bf-inode-err; then
    echo "wrote ok"
  else
    echo "WRITE FAILED: $(cat /tmp/bf-inode-err 2>/dev/null || true)" >&2
  fi
  sleep 3
done
EOF
chmod 755 "$LAB/writer.sh"

cat > "$UNIT" <<EOF
# BREAKFIX-LAB
[Unit]
Description=BREAKFIX-LAB inode-exhaustion writer
After=local-fs.target

[Service]
Type=simple
ExecStart=$LAB/writer.sh
Restart=always
RestartSec=2

[Install]
WantedBy=multi-user.target
EOF

# Persist mount across teardown of unit only — teardown will umount
# Add fstab-less: remount instruction is in README; for reboot survival we skip.
systemctl daemon-reload
systemctl enable --now bf-inodeapp.service

echo
echo "Lab 10 armed."
echo "  Symptom: writes to /mnt/bf-data fail with 'No space left on device'."
echo "  df -h looks fine — check df -i."
echo "  NOTE: loop mount is live for this session; teardown umounts it."
