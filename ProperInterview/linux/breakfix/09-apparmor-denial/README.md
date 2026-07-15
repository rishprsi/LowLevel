# Lab 09 — AppArmor denial

**Ticket #5331** · Priority: High · Time target: **15 minutes**

> "After a 'security review' last night, `bf-aaapp` can't write its log anymore.
> Permissions on `/var/lib/bf-app` look fine. What's going on?"

## Caveat

Needs a real AppArmor host. If `setup.sh` says AppArmor unavailable, run on the
VM/host — not an unprivileged nested container.

## Your job

Restore writes to `/var/lib/bf-app/data.log` (fix the profile, complain-mode test,
or remove the profile). `./check.sh`.

## Hint ladder

<details><summary>Hint 1</summary>
`ls -l /var/lib/bf-app` then `journalctl -k -b | grep -i apparmor`
</details>

<details><summary>Hint 2</summary>
Profile: `/etc/apparmor.d/opt.breakfix-labs.aaapp.writer`. `aa-status` — enforce?
</details>

<details><summary>Hint 3</summary>
`sudo aa-complain` that profile — if writes start, AppArmor was the wall. Fix the
`deny ... w,` rule (allow write) and `apparmor_parser -r`, back to enforce.
</details>

<details><summary><strong>⚠️ SPOILERS</strong></summary>

1. POSIX perms OK; journal shows `apparmor="DENIED"` for `data.log`.
2. Profile has `deny /var/lib/bf-app/data.log w,`.
3. Replace deny with allow `w`, `sudo apparmor_parser -r PROFILE`, restart service.
4. Or `aa-complain` / remove profile for a blunt fix (check accepts any path that stops denials).

**Takeaway:** on Ubuntu, AppArmor sits behind the rwx bits — check DENIED before chown.

</details>
