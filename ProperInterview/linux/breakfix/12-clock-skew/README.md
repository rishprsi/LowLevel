# Lab 12 — Clock skew

**Ticket #5620** · Priority: High · Time target: **10 minutes**

> "`apt update` says Release files are 'not valid yet'. TLS to some services is
> failing too. Nothing changed in apt sources."

## ⚠️ Warning

`setup.sh` **sets the system clock backward** and disables NTP. Disposable VM only.
Requires `sudo BF_YES=1 ./setup.sh`. Won't work in containers that can't set time.

## Your job

Restore time sync so the clock is correct. `./check.sh`. Always `teardown` when done.

## Hint ladder

<details><summary>Hint 1</summary>
`timedatectl` — NTP service? synchronized? What's the date?
</details>

<details><summary>Hint 2</summary>
`timedatectl set-ntp true` then `timedatectl timesync-status` / `journalctl -u systemd-timesyncd`
</details>

<details><summary>Hint 3</summary>
If sync is slow: `systemctl restart systemd-timesyncd`. Pair with lab 08's lesson: TLS
errors can be clock, not certs.
</details>

<details><summary><strong>⚠️ SPOILERS</strong></summary>

1. `timedatectl` → NTP inactive, date ~3 days ago.
2. `sudo timedatectl set-ntp true`
3. Wait for `System clock synchronized: yes`.
4. `./check.sh` / `sudo ./teardown.sh`.

**Takeaway:** "not valid yet" on apt/TLS → check the clock before the certs.

</details>
