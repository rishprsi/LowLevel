# Lab 07 — Cron never ran

**Ticket #5103** · Priority: Medium · Time target: **15 minutes**

> "Our nightly inventory report hasn't shown up in three nights. There's supposed
> to be a file at `/var/lib/breakfix-labs/report/out.txt`. Can you get the job
> producing that again?"

## Your job

Valid scheduled entry (fixed cron.d *or* enabled systemd timer) that can produce
`/var/lib/breakfix-labs/report/out.txt`. Then `./check.sh`.

```bash
sudo ./setup.sh
# ... diagnose & fix ...
./check.sh
sudo ./teardown.sh
```

## Hint ladder

<details><summary>Hint 1</summary>
Cron or timer? Check `/etc/cron.d/` and `systemctl list-timers --all`. Did cron ever try? (`journalctl -u cron`)
</details>

<details><summary>Hint 2</summary>
Cron PATH is `/usr/bin:/bin`. Where does the binary live? `cat -A /etc/cron.d/bf-report` — trailing newline?
</details>

<details><summary>Hint 3</summary>
Two bugs: short name `bf-report` (real path `/usr/local/bin/bf-report`) and **no trailing newline**. Or enable the disabled `bf-report.timer`.
</details>

<details><summary><strong>⚠️ SPOILERS</strong></summary>

1. Missing `out.txt`. Cron.d entry exists; timer inactive (alt path).
2. `cat -A` shows no `$` on last line; short name not on cron PATH.
3. Fix: absolute path + trailing newline, *or* `systemctl enable --now bf-report.timer`.
4. Run once, `./check.sh`.

**Takeaway:** cron PATH is tiny; cron.d needs a final newline; timers win on logging.

</details>
