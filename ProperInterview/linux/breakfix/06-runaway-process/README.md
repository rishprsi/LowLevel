# Lab 06 — The runaway that keeps coming back

**Ticket #5702** · Priority: Critical · Time target: **15 minutes**

> "CPU on this box keeps getting pegged and free memory drops. The on-call killed
> some weird process twice already, but it *comes back within a couple of minutes*.
> Nobody remembers deploying anything. Find it, figure out where it's respawning
> from, and make it stay dead."

## Your job

1. Find the hog and kill it.
2. Trace **where it comes from** and remove the source, so it never returns.
3. Run `./check.sh` — it waits long enough to catch a respawn, so a kill-only "fix"
   will fail the check.

```bash
sudo ./setup.sh
# ... diagnose & fix ...
sudo ./check.sh     # takes ~2.5 minutes on purpose (respawn watch)
sudo ./teardown.sh
```

## Hint ladder (only if stuck)

<details><summary>Hint 1 (vague)</summary>

Find the hog with `top` (press `c` for full command lines) or
`ps aux --sort=-%cpu | head`. The name looks like a kernel thread — is it really one?
Kernel threads have PPID 2 and an *empty* `/proc/PID/cmdline`. Check both.

</details>

<details><summary>Hint 2 (warmer)</summary>

It's a userspace process wearing a costume: `/proc/<pid>/cmdline` and
`/proc/<pid>/exe` expose it, and `pstree -ps <pid>` shows its ancestry. Killing it
doesn't help because something *scheduled* relaunches it. Two places to audit:
`systemctl list-timers --all` and cron — not just `crontab -l`, but system-wide:
`/etc/crontab`, `/etc/cron.d/`, `/var/spool/cron/crontabs/`.

</details>

<details><summary>Hint 3 (nearly the answer)</summary>

`/etc/cron.d/mystery-maintenance` runs `/opt/breakfix-labs/hogspawn/spawner.sh`
every 2 minutes, which copies a hog script to `/tmp/.sysd-cache` and launches it
disguised as `[kswapd1]`. Remove the cron file (and the spawner directory), then
kill the running hog: `sudo pkill -f sysd-cache`. Both steps, then check.

</details>

---

<details><summary><strong>⚠️ SPOILERS — full diagnosis walkthrough</strong></summary>

1. `top` → one process near 100% CPU named `[kswapd1]`, plus notable RSS.
   **Say aloud:** "That *looks* like a kernel thread, but kernel threads never burn
   RSS like that — and I can verify: kernel threads have PPID 2 and empty cmdline."
2. Unmask it:
   - `cat /proc/<pid>/cmdline | tr '\0' ' '` → `[kswapd1] /tmp/.sysd-cache` — a real
     kernel thread would return nothing.
   - `ls -l /proc/<pid>/exe` → `/usr/bin/bash`. It's a shell script in a costume.
   - `ps -o ppid= -p <pid>` / `pstree -ps <pid>` → parent is 1 (it was setsid'd and
     orphaned), so ancestry alone won't finger the launcher.
3. Kill it: `sudo pkill -f sysd-cache`. Watch for two minutes — it **returns**.
   **Say aloud:** "Something is respawning it on a schedule. My checklist: systemd
   timers, cron (all of it), and only then weirder things like at-jobs or a watchdog
   process."
4. `systemctl list-timers --all` → nothing unusual. Cron sweep:
   `ls /etc/cron.d/` → `mystery-maintenance` (marked `# BREAKFIX-LAB`), running
   `/opt/breakfix-labs/hogspawn/spawner.sh` every 2 minutes as root.
   (`grep -r breakfix /etc/cron*` also finds it fast.)
5. Read the spawner — it recopies the hog to `/tmp/.sysd-cache` and relaunches it
   with a fake argv[0] (`exec -a "[kswapd1]"`).
6. Remove the source: `sudo rm /etc/cron.d/mystery-maintenance` and
   `sudo rm -rf /opt/breakfix-labs/hogspawn /tmp/.sysd-cache`.
7. Kill the survivor: `sudo pkill -f sysd-cache` (verify with
   `ps aux | grep -i kswapd1` — the real `kswapd0` at PPID 2 stays, obviously).
8. `sudo ./check.sh` — it verifies nothing is running *and* watches ~2.5 minutes to
   confirm no respawn.

**Interview takeaways:** fake kernel-thread names are unmasked by
`/proc/PID/{cmdline,exe}` and PPID; a recurring process means audit *schedulers*
(timers + all cron locations), not just kill harder; "fixed" means the source is
gone, and you prove it by waiting past the respawn interval.

</details>
