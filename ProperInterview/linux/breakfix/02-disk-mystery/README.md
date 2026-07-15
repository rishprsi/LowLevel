# Lab 02 — The disk space mystery

**Ticket #5117** · Priority: High · Time target: **15 minutes**

> "Monitoring says `/` keeps filling up on this box. We ran
> `du -sh /var/log/*` and the numbers don't add up to what `df` reports at all —
> there's over half a gig we simply cannot find. Also one log file seems to be
> growing like crazy. Please find the missing space, stop the bleeding, and clean
> up **without breaking anything that's writing logs**."

## Your job

1. Find where the "invisible" space went and release it.
2. Stop the runaway log growth and shrink the offending file **the right way**.
3. Run `./check.sh` (needs sudo).

```bash
sudo ./setup.sh     # break the box (writes ~600MB immediately, keeps growing /var/log)
# ... diagnose & fix ...
sudo ./check.sh
sudo ./teardown.sh
```

## Hint ladder (only if stuck)

<details><summary>Hint 1 (vague)</summary>

When `df` and `du` disagree, remember what each one actually measures. `du` walks
directory entries. `df` asks the filesystem for allocated blocks. What kind of file
has blocks but no directory entry?

</details>

<details><summary>Hint 2 (warmer)</summary>

A deleted file's blocks are only freed when the last open file descriptor to it
closes. `sudo lsof +L1` lists open files with zero links. Or without lsof:
`sudo ls -l /proc/*/fd 2>/dev/null | grep deleted`. Separately, `du -sh /var/log/*`
sorted will show you the runaway log — check which process is writing it and *how*
you should shrink a file that a process holds open.

</details>

<details><summary>Hint 3 (nearly the answer)</summary>

Two `mystery-*` systemd services are involved. One (`mystery-datastore`) holds a
~600MB **deleted** file open — stopping that service releases the space. The other
(`mystery-applog`) is spamming `/var/log/mystery-app.log` — stop it, then shrink the
file with `truncate -s 0` (or `> file`), **not** `rm`: deleting a file a writer holds
open just recreates the invisible-space problem you started with.

</details>

---

<details><summary><strong>⚠️ SPOILERS — full diagnosis walkthrough</strong></summary>

1. Confirm the mismatch: `df -h /` vs `sudo du -xsh /` (or
   `sudo du -xsh /var/log`). df reports ~600MB more used than du can account for.
2. **Say aloud:** "df counts allocated blocks; du counts what's reachable via
   directory entries. The gap means a deleted-but-open file."
3. Find it: `sudo lsof +L1` → a process with a large file `(deleted)` under
   `/var/log/`. Pure-/proc alternative:
   `sudo ls -l /proc/*/fd 2>/dev/null | grep '(deleted)'`, then
   `sudo stat -L -c '%s %n' /proc/<pid>/fd/<n>` for the size.
4. Who is it? `ps -p <pid> -o pid,comm,args`, and the owning unit via
   `systemctl status <pid>` → `mystery-datastore.service`.
5. Release the space: `sudo systemctl stop mystery-datastore` (and
   `disable` it). `df -h /` immediately drops ~600MB.
   - If you couldn't restart the process (prod database, say), the surgical option is
     truncating *through* proc: `sudo truncate -s 0 /proc/<pid>/fd/<n>` — frees the
     blocks without killing the writer.
6. The runaway log: `sudo du -sh /var/log/* | sort -h | tail` →
   `/var/log/mystery-app.log` is huge and growing. Find the writer:
   `sudo lsof /var/log/mystery-app.log` → `mystery-applog.service`.
7. Stop it (`sudo systemctl stop mystery-applog && sudo systemctl disable
   mystery-applog`), then shrink correctly: `sudo truncate -s 0
   /var/log/mystery-app.log`.
   **Say aloud:** "Never `rm` a log a live process is writing — the fd stays open and
   the space becomes invisible again. Truncate keeps the same inode. Long-term fix is
   logrotate with `copytruncate` or teaching the app to reopen on SIGHUP."
8. `sudo ./check.sh` goes green.

**Interview takeaways:** df≠du ⇒ deleted-open file, always; `lsof +L1` / `/proc/*/fd`;
`systemctl status <PID>` maps any PID to its unit; truncate vs rm on live logs.

</details>
