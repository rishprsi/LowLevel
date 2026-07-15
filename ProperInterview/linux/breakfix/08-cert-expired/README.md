# Lab 08 — Certificate expired

**Ticket #5210** · Priority: High · Time target: **15 minutes**

> "Our internal HTTPS helper on this box suddenly started failing TLS. Clients
> complain about the certificate. Service is `bf-tlsapp` on port 18443."

## Your job

Diagnose the cert, renew it (self-signed is fine for this lab), restart the service
so HTTPS works again. `./check.sh`.

```bash
sudo ./setup.sh
./check.sh
sudo ./teardown.sh
```

## Hint ladder

<details><summary>Hint 1</summary>
`echo | openssl s_client -connect 127.0.0.1:18443 -servername localhost 2>/dev/null | openssl x509 -noout -dates`
</details>

<details><summary>Hint 2</summary>
`notAfter` is in the past. Cert lives under `/opt/breakfix-labs/tlsapp/`. Service unit: `bf-tlsapp.service`.
</details>

<details><summary>Hint 3</summary>
Regenerate key+cert with a future validity (`openssl req -x509 -days 365 ...`), replace `cert.pem`/`key.pem`, `systemctl restart bf-tlsapp`.
</details>

<details><summary><strong>⚠️ SPOILERS</strong></summary>

1. `curl -v https://127.0.0.1:18443` → certificate verify failed / expired.
2. openssl dates show `notAfter=Jan  2 00:00:00 2020 GMT`.
3. ```
   cd /opt/breakfix-labs/tlsapp
   sudo openssl req -x509 -newkey rsa:2048 -nodes -keyout key.pem -out cert.pem \
     -days 365 -subj "/CN=localhost"
   sudo systemctl restart bf-tlsapp
   ```
4. `./check.sh` green.

**Takeaway:** expiry vs clock skew — check cert dates *and* `timedatectl` (see lab 12).

</details>
