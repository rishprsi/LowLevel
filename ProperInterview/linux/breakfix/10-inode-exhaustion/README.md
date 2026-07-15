# Lab 10 — Inode exhaustion

**Ticket #5402** · Priority: High · Time target: **15 minutes**

> "`bf-inodeapp` can't write to `/mnt/bf-data` — 'No space left on device' — but
> `df -h` shows plenty of free space. What's eating the filesystem?"

## Your job

Free enough inodes on `/mnt/bf-data` that writes succeed. `./check.sh`.

## Hint ladder

<details><summary>Hint 1</summary>
`df -h /mnt/bf-data` vs `df -i /mnt/bf-data`
</details>

<details><summary>Hint 2</summary>
Find the dense directory: `sudo find /mnt/bf-data -xdev -type f | wc -l` and look under `spray/`.
</details>

<details><summary>Hint 3</summary>
Delete the empty-file spray (`rm -rf /mnt/bf-data/spray`), confirm `df -i`, retest write.
</details>

<details><summary><strong>⚠️ SPOILERS</strong></summary>

1. `df -h` free space; `df -i` IUse ~100%.
2. Thousands of empty files in `/mnt/bf-data/spray/`.
3. `sudo rm -rf /mnt/bf-data/spray` (or most of them).
4. `./check.sh` green.

**Takeaway:** "No space left" can mean inodes. Always pair `df -h` with `df -i`.

</details>
