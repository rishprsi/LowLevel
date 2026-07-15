#!/usr/bin/env bash
# BREAKFIX-LAB 12-clock-skew :: teardown
set -euo pipefail
[[ $EUID -eq 0 ]] || { echo "ERROR: sudo ./teardown.sh"; exit 1; }
timedatectl set-ntp true 2>/dev/null || true
# If still skewed and ntp slow, step toward stored BEFORE if present
STATE=/opt/breakfix-labs/clock/state.env
if [[ -f "$STATE" ]]; then
  # shellcheck disable=SC1090
  . "$STATE" || true
  if [[ -n "${BEFORE_UTC:-}" ]]; then
    NOW=$(date -u +%s)
    # if still more than 1 day off from original "before", force set near before+elapsed estimate
    # simpler: just leave NTP to fix; if ntp fails, set to BEFORE_UTC as best effort
    if [[ $((NOW - BEFORE_UTC)) -lt -86400 ]] || [[ $((BEFORE_UTC - NOW)) -gt 86400 ]]; then
      date -u -s "@$BEFORE_UTC" 2>/dev/null || true
      timedatectl set-ntp true 2>/dev/null || true
    fi
  fi
fi
rm -rf /opt/breakfix-labs/clock
rmdir /opt/breakfix-labs 2>/dev/null || true
echo "Lab 12 torn down (NTP re-enabled)."
