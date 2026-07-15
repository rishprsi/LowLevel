#!/usr/bin/env bash
# BREAKFIX-LAB 12-clock-skew :: setup (DISPOSABLE VM / LXD only — SETS THE CLOCK)
set -euo pipefail
[[ $EUID -eq 0 ]] || { echo "ERROR: sudo ./setup.sh"; exit 1; }
. /etc/os-release
[[ "${ID:-}" == "ubuntu" ]] || { echo "ERROR: not Ubuntu."; exit 1; }

LAB=/opt/breakfix-labs/clock
STATE="$LAB/state.env"
mkdir -p "$LAB"

if [[ "${BF_YES:-}" != "1" ]]; then
  echo "WARNING: This lab disables NTP and sets the system clock 3 days in the past."
  echo "         Only run in a disposable VM/container."
  echo "         Re-run with: sudo BF_YES=1 ./setup.sh"
  exit 1
fi

# Can we set the clock?
if ! timedatectl set-ntp false 2>/dev/null; then
  echo "ERROR: cannot disable NTP (container without time namespace?). Use a VM."
  exit 1
fi

BEFORE=$(date -u +%s)
echo "BEFORE_UTC=$BEFORE" > "$STATE"

# Set clock ~3 days ago
if ! date -u -s "@$((BEFORE - 3*86400))" >/dev/null 2>&1; then
  # try timedatectl
  PAST=$(date -u -d "@$((BEFORE - 3*86400))" '+%Y-%m-%d %H:%M:%S' 2>/dev/null \
    || date -u -r $((BEFORE - 3*86400)) '+%Y-%m-%d %H:%M:%S')
  if ! timedatectl set-time "$PAST" 2>/dev/null; then
    echo "ERROR: cannot set system time in this environment."
    timedatectl set-ntp true 2>/dev/null || true
    exit 1
  fi
fi

echo "SKEWED=1" >> "$STATE"

echo
echo "Lab 12 armed. Clock is ~3 days in the past; NTP off."
echo "  Symptoms to expect: apt 'Release file ... not valid yet'; TLS not-yet-valid."
echo "  Fix: restore NTP sync. Then ./check.sh"
echo "  ALWAYS run teardown (or set-ntp true) before you leave the session."
