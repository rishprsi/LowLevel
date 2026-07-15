# Lab 03 — Port conflict

**Ticket #5290** · Priority: Critical · Time target: **10 minutes**

> "Our API service `dataflux-api` on port 8080 went down after last night's
> maintenance window and now it won't stay up — systemd says it keeps restarting.
> Nothing about the API itself changed, we swear. Get it back up, and make sure
> whatever knocked it over can't do it again after a reboot."

## Your job

Get `dataflux-api.service` **active and stable** on port 8080, and make sure the
root cause is gone **persistently** (survives a reboot). Then run `./check.sh`.

```bash
sudo ./setup.sh
# ... diagnose & fix ...
./check.sh
sudo ./teardown.sh
```

## Hint ladder (only if stuck)

<details><summary>Hint 1 (vague)</summary>

The service crash-loops. Read *why* it exits: `journalctl -u dataflux-api -b`. The
error names a very specific, very classic condition. What single command tells you
who currently owns a TCP port?

</details>

<details><summary>Hint 2 (warmer)</summary>

`EADDRINUSE` / "Address already in use". `sudo ss -tlnp | grep 8080` shows the PID
and process squatting on the port. It's a python process — find which systemd unit
it belongs to (`systemctl status <PID>` maps a PID to its unit).

</details>

<details><summary>Hint 3 (nearly the answer)</summary>

Someone left a debug unit `mystery-devserver.service` running
`python3 -m http.server 8080`, **enabled**. Stop it AND disable it (stopping alone
fails the "after a reboot" requirement), then `systemctl restart dataflux-api` and
watch it stay up.

</details>

---

<details><summary><strong>⚠️ SPOILERS — full diagnosis walkthrough</strong></summary>

1. `systemctl status dataflux-api` → `activating (auto-restart)` /
   `Active: failed`, restart counter climbing. **Say aloud:** "It's crash-looping —
   the unit has `Restart=on-failure`, so systemd keeps retrying."
2. `journalctl -u dataflux-api -b --no-pager | tail -20` →
   `OSError: [Errno 98] Address already in use` on bind to 0.0.0.0:8080.
3. Who owns the port? `sudo ss -tlnp | grep :8080` →
   `users:(("python3",pid=NNNN,...))`. **Say aloud:** "ss with `-p` needs sudo to
   show other users' processes; this is the modern netstat."
4. Map PID to unit: `systemctl status NNNN` → `mystery-devserver.service`, running
   `python3 -m http.server 8080`. Someone's forgotten debug server.
5. Check persistence: `systemctl is-enabled mystery-devserver` → `enabled`. So just
   killing the PID isn't a fix — systemd would restart it, and reboot would revive it.
6. Fix: `sudo systemctl disable --now mystery-devserver`.
7. Restore service: `sudo systemctl restart dataflux-api`, then verify it *stays* up:
   `systemctl is-active dataflux-api`, `sudo ss -tlnp | grep :8080` now shows
   dataflux-api's process, `curl -s localhost:8080` answers.
8. `./check.sh` goes green.

**Interview takeaways:** crash-loop ⇒ read the journal, not just status; EADDRINUSE ⇒
`ss -tlnp`; PID→unit via `systemctl status <PID>`; a fix isn't done until it survives
restart *and* reboot (disable, not just stop/kill).

</details>
