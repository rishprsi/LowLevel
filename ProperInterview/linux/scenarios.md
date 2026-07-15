# Interview scenario playbooks

Six "walk me through diagnosing X" questions as spoken-answer scripts. For each:
a **60-second structured answer** (first three commands, what each result means),
then a deeper dive. Rehearse the 60-second versions aloud until they're automatic.

The universal frame — say some version of this before diving into any scenario:

> "First I'd confirm the symptom and scope — one box or many, when it started, what
> changed. Then I observe with cheap read-only commands, form a hypothesis, verify
> it with the right tool, fix, and confirm the symptom is gone. I'll narrate as I go."

---

## 1. High load average, but CPU is mostly idle

*Maps to drill 01 part 4.*

### The 60-second answer

> "Load counts runnable **plus uninterruptible-sleep (D-state)** tasks, not CPU use —
> so high load with idle CPU says processes are stuck waiting, almost always on I/O.
>
> First `uptime` — is it 1-minute spike or 15-minute sustained?
> Second `vmstat 1` — the `b` column is blocked tasks and `wa` is iowait; if those
> are high, it's I/O, hypothesis confirmed.
> Third `ps -eo pid,stat,wchan:30,comm | awk '$2~/D/'` — the D-state processes and
> the kernel function they're sleeping in. NFS symbols → hung mount; block-layer
> symbols → check the disk with `iostat -x 1` (util/await) and `dmesg` for I/O errors.
>
> Fix is the underlying I/O — remount or lazy-umount a dead NFS export, or the
> failing disk — not the processes; D-state ignores SIGKILL. Confirm load drains."

### Deeper dive

- Decision tree: `wa` high + `iostat %util` ~100 on one device → that device (failing
  disk, saturated volume). `wa` low but D-states present → network filesystem or
  driver hang; `dmesg -T | tail` and `mount | grep nfs`. Neither → fork storm: check
  `vmstat` `r` column and process count instead.
- Also worth saying: on a VM check `st` (steal) — a starved hypervisor mimics this.
- Trap to acknowledge: killing D-state processes doesn't work; a truly stuck kernel
  path may need a reboot after mitigating the device.

---

## 2. Disk is full (or df/du disagree)

*Maps to lab 02-disk-mystery.*

### The 60-second answer

> "First `df -h` — which filesystem, how bad, and is it *bytes* or **inodes**
> (`df -i`) that's exhausted; 'No space left' with free bytes means inodes.
> Second `sudo du -xh --max-depth=1 / | sort -h | tail` — `-x` stays on one
> filesystem; drill down into the biggest directory iteratively.
> Third, if du can't account for what df reports: `sudo lsof +L1` — a deleted file
> some process still holds open; the space frees only when the fd closes.
>
> Fix by type: runaway log → stop the writer, `truncate -s 0`, never `rm` a live
> log; deleted-open → restart the holder or truncate via `/proc/PID/fd/N`; genuine
> growth → clean apt caches, old journals (`journalctl --vacuum-size=200M`), or grow
> the volume. Confirm with `df -h` and make it durable — logrotate, monitoring."

### Deeper dive

- Sneaky case worth mentioning: files hidden **under a mountpoint** — written to
  `/mnt/data` before the mount existed; `du` on the mounted tree misses them
  (bind-mount `/` elsewhere to see under mounts).
- df≠du is *always* deleted-but-open files (or mountpoint shadowing). Say it that
  confidently.
- Ubuntu specifics: `/var/log/journal` growth (`journalctl --disk-usage`), old snap
  revisions (`snap set system refresh.retain=2`), `/var/cache/apt`.

---

## 3. Processes are dying — OOM kills

### The 60-second answer

> "Random process deaths with exit signal 9 and nobody ran kill — I check the kernel
> log first: `journalctl -k | grep -i oom` (or `dmesg -T`). The OOM-killer logs the
> victim, its RSS, and a full task dump — that tells me who was fat, who died, and
> when.
> Second `free -h` and `vmstat 1` — current pressure, and whether swap is thrashing.
> Third `ps aux --sort=-%mem | head` — today's biggest consumers; compare against
> the OOM dump to see if it's one leaking service or genuine overcommit.
>
> Immediate fix: restart or bound the leaker. Durable fix: memory limits so the
> *right* thing dies — `MemoryMax=` on the unit via cgroups, plus right-sizing or
> fixing the leak. Confirm no new OOM lines over time."

### Deeper dive

- Read the OOM report: `oom_score_adj` explains "why did it kill *sshd's* neighbor
  and not the fat process" cases; critical daemons set negative adjustments.
- systemd angle: `systemd-oomd` exists on Ubuntu desktop (cgroup-level, pressure
  based); a service killed by it shows in `systemctl status` as `oom-kill`.
  `MemoryMax=`/`MemoryHigh=` in the unit is the clean per-service bound.
- Distinguish: kernel OOM (global, logs in kmsg) vs container/cgroup OOM (limit hit
  inside the cgroup — the host may have plenty free). `journalctl -u <svc>` +
  `systemctl show <svc> -p MemoryMax` settles it.
- If memory grows slowly over days: watch `/proc/PID/status` VmRSS over time, or
  `smem`/`pmap` for where it lives.

---

## 4. A service won't start

*Maps to labs 01, 03, 05.*

### The 60-second answer

> "First `systemctl status <svc>` — state, exit code, and the last log lines in one
> shot. The exit code family routes me: **203/EXEC** means systemd couldn't even run
> the binary (bad path/perms/shebang); **plain 1/FAILURE** means the app ran and
> died — read its own error; 'activating (auto-restart)' means crash-loop.
> Second `journalctl -u <svc> -b --no-pager | tail -30` — the actual error:
> EADDRINUSE → find the port squatter with `ss -tlnp`; EACCES → check the unit's
> `User=` against file ownership; config parse error → fix the config.
> Third `systemctl cat <svc>` — the unit *as systemd sees it*, including drop-ins;
> half of 'mysterious' unit behavior is a forgotten override, and if I edited
> anything: `daemon-reload`.
>
> Fix the specific cause, `restart`, verify with `is-active` — and check `is-enabled`
> so it survives reboot. Then watch it for a minute so I know it's stable, not
> between crashes."

### Deeper dive

- Exit-code cheat sheet: 203/EXEC path-or-perms, 217/USER bad `User=`, 226/NAMESPACE
  sandboxing options, 200/CHDIR bad `WorkingDirectory=`.
- Dependency failures: `systemctl list-dependencies <svc>`, and remember `Requires=`
  without `After=` is a race, not an ordering.
- `systemd-analyze verify <unit>` lints typos (misspelled targets, unknown keys).
- Timeouts: `TimeoutStartSec` kills slow starters — status shows `timeout`. For
  `Type=notify` services that never notify, that's the classic cause.

---

## 5. Server is unreachable over the network

*Maps to drill 04 and lab 04-dns-sabotage.*

### The 60-second answer

> "I bisect the path from the box outward — assuming I have console/OOB access since
> SSH is what's broken.
> First `ip a` and `ip r` — is the interface UP with LOWER_UP (carrier), does it
> have the right address, is there a default route? No route or no carrier ends the
> investigation right there.
> Second `ping <gateway>` then `ping 1.1.1.1` — gateway fails: local L2/switch/VLAN
> problem; gateway OK but internet fails: routing/NAT upstream.
> Third, if IPs work but names don't, it's DNS: `resolvectl status` and
> `getent hosts <name>` vs `dig <name>` to split /etc/hosts vs resolved vs upstream.
>
> If others can't reach *us* but outbound works: `ss -tlnp` — is the service
> listening on 0.0.0.0 or only 127.0.0.1 — and the firewall: `ufw status`,
> `nft list ruleset`. Fix the failing layer, confirm end-to-end from the client."

### Deeper dive

- Layer script to narrate: link (`ip -s link` errors/drops) → addressing (DHCP
  lease? `networkctl status`) → routing (`ip r get <dst>`) → firewall → transport
  (`ss`) → application. Name the layer you're testing at each step; interviewers
  score the structure.
- Netplan on 24.04: config in `/etc/netplan/*.yaml`, rendered to systemd-networkd on
  servers; validate with `netplan try` before `apply`, and check
  `networkctl`/`journalctl -u systemd-networkd` when links misbehave.
- On clouds: security groups outrank everything inside the guest — say you'd check
  them early if it's an instance.
- `tcpdump -ni <if> host <client>` answers "do packets even arrive?" — arriving +
  no reply points at firewall or listener; not arriving points upstream.

---

## 6. Server won't boot

### The 60-second answer

> "First, get eyes on the console — physical, or virsh/IPMI/cloud console. The
> failure *stage* tells me where to work: no GRUB → firmware/bootloader/disk; kernel
> panic on load → kernel or initramfs; drops to the **initramfs shell** → it can't
> find or mount root — my most common case; hangs in userspace with systemd messages
> → a failing unit or bad fstab entry.
> Second, at the GRUB menu I take the recovery entry or edit the kernel line —
> previous kernel if a kernel update broke it, or add `systemd.unit=rescue.target`
> (single-user equivalent) for userspace failures.
> Third, from the emergency shell: `journalctl -xb` for what failed. The classic is
> a bad `/etc/fstab` entry — a removed disk without `nofail` blocks boot with a
> 90-second device timeout then emergency mode; comment it out, `mount -a` to test,
> reboot.
>
> If the system can't even reach GRUB, I boot a live/rescue image, mount and chroot
> into the root filesystem, and repair from there — reinstall grub, fix fstab,
> rebuild initramfs with `update-initramfs -u`."

### Deeper dive

- Stage → tool map: firmware (boot order, disk detected) → GRUB (`grub-install`,
  `update-grub` from chroot) → kernel/initramfs (boot older kernel from "Advanced
  options"; `update-initramfs -u -k <ver>`) → mount root (fstab, LUKS, LVM
  activation) → systemd (rescue/emergency targets, `systemctl list-jobs` to see
  what boot is *waiting* on).
- fstab wisdom to volunteer: any non-critical mount gets `nofail` (and
  `x-systemd.device-timeout=10s`) precisely so its absence can't hold the whole
  boot hostage.
- After any successful rescue: `systemd-analyze blame`/`critical-chain` and
  `journalctl -b -1 -p err` to explain the previous bad boot — close the loop on
  root cause, don't just celebrate the reboot.

---

## Closing move for every scenario

End each answer with confirmation and prevention — interviewers listen for it:

> "…then I confirm the original symptom is gone, check the journal is clean, and ask
> what monitoring or guardrail would have caught this earlier — that's the ticket
> follow-up."
