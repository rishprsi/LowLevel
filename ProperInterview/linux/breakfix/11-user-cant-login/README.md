# Lab 11 — User can't log in

**Ticket #5519** · Priority: High · Time target: **15 minutes**

> "`bf-deploy` key auth worked last week. Now `ssh -i /root/bf-lab-key bf-deploy@localhost`
> fails. We didn't change sshd_config. Fix the account."

## Your job

Restore key-based SSH for `bf-deploy` without weakening global sshd settings. `./check.sh`.

## Hint ladder

<details><summary>Hint 1</summary>
`ssh -vv -i /root/bf-lab-key bf-deploy@localhost` and `journalctl -u ssh -b | tail`
</details>

<details><summary>Hint 2</summary>
`ls -la /home/bf-deploy /home/bf-deploy/.ssh` — StrictModes hates group-writable homes.
`getent passwd bf-deploy` — what's the shell?
</details>

<details><summary>Hint 3</summary>
`chmod 755 /home/bf-deploy` (or 700), fix `.ssh` to 700 / `authorized_keys` 600,
`chsh -s /bin/bash bf-deploy`.
</details>

<details><summary><strong>⚠️ SPOILERS</strong></summary>

Two planted bugs:
1. Home mode `775` → sshd rejects authorized_keys ("bad ownership or modes").
2. Shell `/usr/sbin/nologin` → "This account is currently not available."

Fix both, then BatchMode ssh succeeds.

**Takeaway:** key ignored ≠ wrong key; check perms + shell + `chage -l`.

</details>
