#!/usr/bin/env bash
# BREAKFIX-LAB 06-runaway-process :: setup (run in a DISPOSABLE VM only)
set -euo pipefail

[[ $EUID -eq 0 ]] || { echo "ERROR: run as root: sudo ./setup.sh"; exit 1; }
. /etc/os-release
[[ "${ID:-}" == "ubuntu" ]] || { echo "ERROR: refusing to run — this is not Ubuntu."; exit 1; }

LAB_DIR=/opt/breakfix-labs/hogspawn
CRON_FILE=/etc/cron.d/mystery-maintenance

# Idempotent: remove any previous instance first
rm -f "$CRON_FILE"
pkill -f 'sysd-cache' 2>/dev/null || true
rm -rf "$LAB_DIR" /tmp/.sysd-cache

mkdir -p "$LAB_DIR"

# The hog: pegs one CPU, holds ~150MB RSS, self-destructs after 30 min as a safety net.
cat > "$LAB_DIR/hog.sh" <<'EOF'
#!/usr/bin/env bash
# BREAKFIX-LAB payload: CPU+memory hog wearing a kernel-thread costume
set -u
mem=$(head -c 150M /dev/zero | tr '\0' 'x')
end=$(( $(date +%s) + 1800 ))
while (( $(date +%s) < end )); do
  : "${mem:0:1}"
done
EOF
chmod 755 "$LAB_DIR/hog.sh"

# The spawner: cron runs this every 2 min; it relaunches the hog if it's dead,
# disguised via a fake argv[0] and a hidden /tmp path.
cat > "$LAB_DIR/spawner.sh" <<'EOF'
#!/usr/bin/env bash
# BREAKFIX-LAB payload: respawner
set -u
pgrep -f 'sysd-cache' >/dev/null && exit 0
cp /opt/breakfix-labs/hogspawn/hog.sh /tmp/.sysd-cache
chmod 755 /tmp/.sysd-cache
setsid bash -c 'exec -a "[kswapd1]" bash /tmp/.sysd-cache' </dev/null >/dev/null 2>&1 &
EOF
chmod 755 "$LAB_DIR/spawner.sh"

cat > "$CRON_FILE" <<'EOF'
# BREAKFIX-LAB
*/2 * * * * root /opt/breakfix-labs/hogspawn/spawner.sh
EOF
chmod 644 "$CRON_FILE"

# Don't make the player wait for cron's first tick
"$LAB_DIR/spawner.sh"

echo
echo "Lab armed. Ticket: 'CPU pegged; on-call killed the process twice, it returns.'"
echo "Target: 15 minutes. Then: sudo ./check.sh (takes ~2.5 min — respawn watch)"
