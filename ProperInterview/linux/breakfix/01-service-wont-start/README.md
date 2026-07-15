# Lab 01 — Service won't start

**Ticket #4821** · Priority: High · Time target: **10 minutes**

> "We deployed the new *dataflux* batch worker onto this box last night. The deploy
> script said OK, but the service never came up. We haven't touched anything else.
> Can you get it running and make sure it survives a reboot?"

## Your job

Get `dataflux.service` **active** and **enabled**, and clean up anything wrong in the
unit while you're in there. Then run `./check.sh`.

```bash
sudo ./setup.sh     # break the box
# ... diagnose & fix ...
./check.sh          # green when genuinely fixed
sudo ./teardown.sh  # full reset, whatever state you left it in
```

## Hint ladder (only if stuck)

<details><summary>Hint 1 (vague)</summary>

Start where you always start with a failed unit. Two commands give you 90% of the
story: one shows state + recent log lines, one shows the unit file *as systemd sees
it*.

</details>

<details><summary>Hint 2 (warmer)</summary>

The status output contains an exit code in the 200 range. Look up what that code
family means — it's systemd telling you it failed *before* the program even ran.
Then compare the `ExecStart=` path against what actually exists on disk under
`/opt/breakfix-labs/dataflux/`.

</details>

<details><summary>Hint 3 (nearly the answer)</summary>

`ExecStart=` points at a file that does not exist — the real script is in a `bin/`
subdirectory with a slightly different name. Also read the `After=`/`Wants=` lines
carefully: that target name is misspelled (`systemd-analyze verify` would have told
you). Fix both, `daemon-reload`, `start`, `enable`.

</details>

---

<details><summary><strong>⚠️ SPOILERS — full diagnosis walkthrough</strong></summary>

1. `systemctl status dataflux` → `Active: failed`, and
   `(code=exited, status=203/EXEC)`. **203/EXEC = systemd could not execute the
   ExecStart binary**: path missing, not executable, or bad shebang.
2. `journalctl -u dataflux -b --no-pager | tail` → confirms: "Failed to locate
   executable /opt/breakfix-labs/dataflux/dataflux-worker.sh: No such file or
   directory" (or similar 203 messaging).
3. `systemctl cat dataflux` → read the unit. Two problems:
   - `ExecStart=/opt/breakfix-labs/dataflux/dataflux-worker.sh` — but
     `ls /opt/breakfix-labs/dataflux/` shows the payload is really at
     `bin/dataflux-worker.sh`.
   - `After=network-onlin.target` / `Wants=network-onlin.target` — typo. A missing
     `After=` unit is silently ignored (ordering-only), so it doesn't block startup,
     but it means the intended "wait for network" never happens. `systemd-analyze
     verify /etc/systemd/system/dataflux.service` flags it.
4. Fix the unit (edit `/etc/systemd/system/dataflux.service` or use
   `systemctl edit --full dataflux`):
   - `ExecStart=/opt/breakfix-labs/dataflux/bin/dataflux-worker.sh`
   - `After=network-online.target` and `Wants=network-online.target`
5. `sudo systemctl daemon-reload` — **mandatory** after editing the file directly.
6. `sudo systemctl start dataflux && sudo systemctl enable dataflux`
7. Confirm: `systemctl is-active dataflux`, `journalctl -u dataflux -f` shows the
   worker heartbeat. `./check.sh` goes green.

**Interview takeaways:** 203/EXEC decodes instantly if you've seen it; `systemctl cat`
before editing anything; `daemon-reload` after; enable ≠ start; `systemd-analyze
verify` catches unit typos for free.

</details>
