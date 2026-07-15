# Lab 05 — Permission denied

**Ticket #5561** · Priority: High · Time target: **10 minutes**

> "After a security-hardening pass by another team, our `dataflux-agent` service is
> down. It was running fine for months. The hardening ticket says they 'normalized
> ownership under /var/lib' — surely that couldn't break anything? Service needs to
> be back up and actually writing its state."

## Your job

Get `dataflux-agent.service` **active** and successfully writing its state file
again. Then run `./check.sh`.

```bash
sudo ./setup.sh
# ... diagnose & fix ...
./check.sh
sudo ./teardown.sh
```

## Hint ladder (only if stuck)

<details><summary>Hint 1 (vague)</summary>

`systemctl status dataflux-agent` then `journalctl -u dataflux-agent -b`. The error
is one syscall failing. Then look at *which user* the service runs as — it's right
there in the unit file.

</details>

<details><summary>Hint 2 (warmer)</summary>

Journal shows `Permission denied` writing under `/var/lib/dataflux-agent`. The unit
has `User=dataflux` (see `systemctl cat dataflux-agent`). Now
`ls -ld /var/lib/dataflux-agent` — who owns it, and what's the mode? Can user
`dataflux` even *enter* that directory?

</details>

<details><summary>Hint 3 (nearly the answer)</summary>

The dir is `root:root` mode `700` while the service runs as `dataflux`. Quick fix:
`chown -R dataflux:dataflux /var/lib/dataflux-agent && chmod 750 ...`, restart.
The *durable* fix is `StateDirectory=dataflux-agent` in the unit — systemd then
creates and owns the chown problem for you on every start. Do either; the
walkthrough covers both.

</details>

---

<details><summary><strong>⚠️ SPOILERS — full diagnosis walkthrough</strong></summary>

1. `systemctl status dataflux-agent` → failed/restarting,
   `(code=exited, status=1/FAILURE)` — an *application* error, not a 200-series
   systemd exec error, so the program ran and then died. Read its output.
2. `journalctl -u dataflux-agent -b --no-pager | tail` →
   `cannot create /var/lib/dataflux-agent/state.json: Permission denied` (EACCES).
3. Who does it run as? `systemctl cat dataflux-agent` → `User=dataflux`,
   `Group=dataflux`. **Say aloud:** "Services should never run as root; this one
   correctly uses a dedicated system user — so its writable dirs must be owned
   accordingly."
4. `ls -ld /var/lib/dataflux-agent` → `drwx------ root root`. Mode 700 + root-owned:
   user `dataflux` can't even traverse into it, let alone write.
   (`sudo -u dataflux touch /var/lib/dataflux-agent/t` reproduces the EACCES in one
   line — nice demo.)
5. **Quick fix:**
   `sudo chown -R dataflux:dataflux /var/lib/dataflux-agent && sudo chmod 750
   /var/lib/dataflux-agent`, then `sudo systemctl restart dataflux-agent`.
6. **The "right" fix — say this in the interview:** add `StateDirectory=
   dataflux-agent` to `[Service]` (drop-in via `systemctl edit dataflux-agent`).
   systemd then creates `/var/lib/dataflux-agent` with correct ownership on every
   start — self-healing against exactly this class of "hardening pass" regression.
   Same family: `RuntimeDirectory=` (/run), `LogsDirectory=` (/var/log),
   `CacheDirectory=` (/var/cache). Also worth a mention: `systemd-tmpfiles -d`
   declarations achieve the same for non-systemd-managed paths.
7. Verify: `systemctl is-active dataflux-agent`; the journal shows successful writes;
   `ls -l /var/lib/dataflux-agent/state.json` exists and is fresh. `./check.sh` green.

**Interview takeaways:** 200-series exit codes = systemd couldn't start it; plain
1/FAILURE = the app itself failed, so read *its* logs; check the unit's `User=`
whenever you see EACCES; `StateDirectory=` is the systemd-native durable fix and
mentioning it is the difference between "fixed it" and "fixed the class of bug."

</details>
