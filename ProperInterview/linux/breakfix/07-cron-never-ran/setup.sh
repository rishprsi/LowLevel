#!/usr/bin/env bash
# BREAKFIX-LAB 07-cron-never-ran :: setup (DISPOSABLE VM / LXD container only)
set -euo pipefail
[[ $EUID -eq 0 ]] || { echo "ERROR: run as root: sudo ./setup.sh"; exit 1; }
. /etc/os-release
[[ "${ID:-}" == "ubuntu" ]] || { echo "ERROR: refusing — not Ubuntu."; exit 1; }

CRON=/etc/cron.d/bf-report
UNIT_SVC=/etc/systemd/system/bf-report.service
UNIT_TMR=/etc/systemd/system/bf-report.timer
REPORT_DIR=/var/lib/breakfix-labs/report

systemctl disable --now bf-report.timer 2>/dev/null || true
systemctl stop bf-report.service 2>/dev/null || true
rm -f "$CRON" "$UNIT_SVC" "$UNIT_TMR" /usr/local/bin/bf-report
rm -rf /opt/breakfix-labs/cronjob "$REPORT_DIR"
systemctl daemon-reload 2>/dev/null || true

mkdir -p /opt/breakfix-labs/cronjob "$REPORT_DIR"
chmod 755 "$REPORT_DIR"

cat > /usr/local/bin/bf-report <<'EOF'
#!/usr/bin/env bash
# BREAKFIX-LAB payload
set -euo pipefail
mkdir -p /var/lib/breakfix-labs/report
echo "bf-report OK at $(date -Is)" > /var/lib/breakfix-labs/report/out.txt
EOF
chmod 755 /usr/local/bin/bf-report

# Broken: short name (not on cron PATH) AND no trailing newline
printf '%s' '# BREAKFIX-LAB — nightly report
SHELL=/bin/bash
PATH=/usr/bin:/bin
*/5 * * * * root bf-report' > "$CRON"
chmod 644 "$CRON"

cat > "$UNIT_SVC" <<'EOF'
# BREAKFIX-LAB
[Unit]
Description=BREAKFIX-LAB bf-report oneshot
[Service]
Type=oneshot
ExecStart=/usr/local/bin/bf-report
EOF

cat > "$UNIT_TMR" <<'EOF'
# BREAKFIX-LAB
[Unit]
Description=BREAKFIX-LAB bf-report timer (disabled on purpose)
[Timer]
OnCalendar=*:0/5
Persistent=true
[Install]
WantedBy=timers.target
EOF
systemctl daemon-reload

echo
echo "Lab 07 armed."
echo "  Symptom: /var/lib/breakfix-labs/report/out.txt never appears."
echo "  Fix, then ./check.sh"
