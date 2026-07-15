#!/usr/bin/env bash
# BREAKFIX-LAB 08-cert-expired :: check
set -uo pipefail
GREEN=$'\e[32m'; RED=$'\e[31m'; RESET=$'\e[0m'
fail=0
LAB=/opt/breakfix-labs/tlsapp
PORT=18443
CERT="$LAB/cert.pem"

if ! systemctl is-active --quiet bf-tlsapp.service; then
  echo "${RED}  FAIL: bf-tlsapp.service not active${RESET}"; fail=1
else
  echo "  ok: bf-tlsapp.service active"
fi

if [[ -f "$CERT" ]]; then
  end=$(openssl x509 -in "$CERT" -noout -enddate 2>/dev/null | cut -d= -f2 || true)
  if openssl x509 -in "$CERT" -noout -checkend 86400 >/dev/null 2>&1; then
    echo "  ok: cert valid for at least 24h more (end=$end)"
  else
    echo "${RED}  FAIL: cert still expired or expiring within 24h (end=$end)${RESET}"; fail=1
  fi
else
  echo "${RED}  FAIL: $CERT missing${RESET}"; fail=1
fi

# curl with --cacert against our self-signed (or -k if they replaced with new self-signed and we trust file)
if curl -fsS --cacert "$CERT" "https://127.0.0.1:$PORT/" -o /dev/null 2>/dev/null \
   || curl -fsSk "https://127.0.0.1:$PORT/" -o /dev/null 2>/dev/null; then
  # Prefer verify via openssl s_client dates in the future
  if openssl x509 -in "$CERT" -noout -checkend 86400 >/dev/null 2>&1; then
    echo "  ok: HTTPS responds"
  else
    echo "${RED}  FAIL: HTTPS responds but cert still bad${RESET}"; fail=1
  fi
else
  echo "${RED}  FAIL: HTTPS request failed${RESET}"; fail=1
fi

echo
[[ $fail -eq 0 ]] && { echo "${GREEN}✔ FIXED — cert valid and service serving HTTPS.${RESET}"; exit 0; }
echo "${RED}✘ Not fixed yet.${RESET}"; exit 1
