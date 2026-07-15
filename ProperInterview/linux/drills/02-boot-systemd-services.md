# Drill 02 — Boot, systemd, services (45 min)

**Goal:** Tell the boot story end to end without notes, drive systemctl/journalctl
fluently, and write → break → diagnose → fix a real unit file.

---

## Part 1 — narrate the boot story

Rehearse this aloud until it's 60 seconds, smooth:

1. **Firmware (UEFI/BIOS)** does POST, picks a boot entry, loads the bootloader from
   the ESP (UEFI) or MBR (legacy).
2. **GRUB** presents the menu, loads the **kernel** (`/boot/vmlinuz-*`) and
   **initramfs** (`/boot/initrd.img-*`) into memory, passes the kernel command line
   (see it later with `cat /proc/cmdline`).
3. **Kernel** initializes hardware, mounts the initramfs as a temporary root. The
   initramfs contains just enough (storage drivers, LVM/crypto/RAID tools) to find and
   mount the **real root filesystem**, then pivots to it.
4. Kernel execs **PID 1 = systemd**, which reads `/etc/fstab` (via generated mount
   units), and walks the dependency graph up to **`default.target`** (normally
   `graphical.target` on desktop, `multi-user.target` on server).

Verify pieces on the VM:

```bash
cat /proc/cmdline
systemctl get-default
systemctl list-dependencies default.target --no-pager | head -30
journalctl -b -k | head -30        # kernel messages from this boot
```

**Say aloud:** "Targets replace runlevels — they're just synchronization points in the
dependency graph. multi-user.target ≈ runlevel 3, graphical.target ≈ runlevel 5,
rescue.target ≈ single-user."

---

## Part 2 — systemctl fluency

```bash
systemctl status ssh                       # state, PID, recent journal lines, cgroup
systemctl cat ssh                          # THE unit file(s) actually in effect, incl. drop-ins
systemctl list-dependencies ssh            # what it pulls in
systemctl list-units --type=service --state=failed
systemctl edit ssh                         # creates an override drop-in (never edit vendor files)
systemctl edit --full ssh                  # copy whole unit to /etc/systemd/system
systemctl show ssh -p ExecStart,Restart,User   # queried, resolved properties
```

**Say aloud:** "`systemctl cat` is my first move on any unit problem — it shows every
drop-in that's layered on top of the vendor file. `/etc/systemd/system` overrides
`/usr/lib/systemd/system`; drop-ins in `<unit>.d/` override individual keys."

---

## Part 3 — journalctl fluency

```bash
journalctl -u ssh                # one unit's logs
journalctl -u ssh -b            # ...this boot only
journalctl -b -1                # previous boot (needs persistent journal — default on 24.04)
journalctl -k                   # kernel ring buffer (like dmesg, but persistent)
journalctl -p err -b            # priority err and worse, this boot
journalctl -u ssh -f            # follow
journalctl -u ssh --since "10 min ago"
journalctl -u ssh -o verbose | head -50   # every field, incl. _PID, _EXE
```

**Say aloud:** "For a failed service my reflex is `systemctl status X` then
`journalctl -u X -b -p warning`. `-b -1` is gold after a crash/reboot — the evidence
is in the *previous* boot."

---

## Part 4 — write a unit, break it, fix it

### Step 1: payload + unit

```bash
sudo tee /opt/heartbeat.sh >/dev/null <<'EOF'
#!/bin/bash
while true; do echo "heartbeat $(date -Is)"; sleep 5; done
EOF
sudo chmod +x /opt/heartbeat.sh

sudo tee /etc/systemd/system/heartbeat.service >/dev/null <<'EOF'
[Unit]
Description=Drill heartbeat service
After=network-online.target
Wants=network-online.target

[Service]
ExecStart=/opt/heartbeat.sh
Restart=on-failure
RestartSec=3

[Install]
WantedBy=multi-user.target
EOF

sudo systemctl daemon-reload
sudo systemctl enable --now heartbeat
systemctl status heartbeat
journalctl -u heartbeat -f     # watch a couple of heartbeats, Ctrl-C
```

**Say aloud:** "`enable` creates the symlink in `multi-user.target.wants/` — that's
literally all enabling is. `--now` also starts it. `Restart=on-failure` plus
`RestartSec` is the standard self-healing pattern; systemd rate-limits restarts via
StartLimitIntervalSec/StartLimitBurst."

### Step 2: break it

```bash
sudo sed -i 's|ExecStart=/opt/heartbeat.sh|ExecStart=/opt/heartbeats.sh|' \
  /etc/systemd/system/heartbeat.service
sudo systemctl daemon-reload
sudo systemctl restart heartbeat
```

### Step 3: diagnose like it's an interview

```bash
systemctl status heartbeat        # status=203/EXEC — memorize this one
journalctl -u heartbeat -b --no-pager | tail -20
systemctl cat heartbeat           # read the ExecStart with your own eyes
ls -l /opt/heartbeat*             # does the path exist? is it executable?
```

**Say aloud:** "`203/EXEC` means systemd couldn't exec the binary — wrong path, missing
execute bit, or a bad shebang. `217/USER` would be a bad User=, `EACCES`-style failures
land as 200-series codes too. The journal plus `systemctl cat` plus `ls -l` settles it
in under a minute."

### Step 4: fix and confirm

```bash
sudo sed -i 's|heartbeats.sh|heartbeat.sh|' /etc/systemd/system/heartbeat.service
sudo systemctl daemon-reload && sudo systemctl restart heartbeat
systemctl is-active heartbeat && echo FIXED
```

Cleanup:

```bash
sudo systemctl disable --now heartbeat
sudo rm /etc/systemd/system/heartbeat.service /opt/heartbeat.sh
sudo systemctl daemon-reload
```

---

## Part 5 — boot performance

```bash
systemd-analyze                  # kernel vs userspace time
systemd-analyze blame | head     # slowest units (careful: parallel, so times overlap)
systemd-analyze critical-chain   # the actual dependency path that gated boot
systemd-analyze verify /etc/systemd/system/heartbeat.service   # lint a unit file
```

**Say aloud:** "`blame` misleads because units start in parallel — `critical-chain`
shows what actually serialized the boot. And `verify` lints a unit before I ever
start it."

---

## Self-check questions

1. What loads the initramfs, and what problem does the initramfs solve?
2. A unit shows `(code=exited, status=203/EXEC)`. Name three distinct causes.
3. What exactly does `systemctl enable` do on disk?
4. What's the difference between `After=` and `Requires=`? What happens if you have
   `Requires=` without `After=`?
5. You edited a unit file and restarted the service, but the old behavior persists.
   What did you forget, and how would `systemctl status` warn you?
6. How do you see logs from the boot *before* the current one, and what must be true
   of the journal for that to work?
7. Map runlevels 1, 3, 5 to systemd targets.
8. Why can `systemd-analyze blame` mislead you, and what should you use instead?

<details>
<summary><strong>Answers</strong></summary>

1. GRUB loads it alongside the kernel. It provides a temporary root with the drivers
   and tools (LVM, dm-crypt, RAID, storage modules) needed to find and mount the real
   root filesystem — solving the chicken-and-egg of "the disk driver is on the disk."
2. ExecStart path doesn't exist; file isn't executable; broken shebang/interpreter
   missing. (Also: binary for the wrong architecture, or blocked by namespace
   sandboxing options.)
3. It creates symlinks according to the `[Install]` section — e.g.
   `/etc/systemd/system/multi-user.target.wants/foo.service → <unit file>`. No
   daemon state changes; that's why it doesn't start the unit without `--now`.
4. `After=` is pure ordering; `Requires=` is dependency (start it with me, fail/stop
   with it) with **no ordering implied**. `Requires=` without `After=` means both
   start simultaneously — your unit can start before its "requirement" is ready.
5. `systemctl daemon-reload`. Status prints a warning: "The unit file ... changed on
   disk. Run 'systemctl daemon-reload' to reload units."
6. `journalctl -b -1` (list boots with `journalctl --list-boots`). The journal must
   be persistent — `/var/log/journal` existing (default `Storage=auto` on Ubuntu
   persists when that directory exists, which it does on 24.04).
7. 1 → rescue.target, 3 → multi-user.target, 5 → graphical.target.
8. Units start in parallel, so a unit can show a large time while never gating
   anything (it was just waiting alongside). `systemd-analyze critical-chain` shows
   the serialized path that actually determined boot time.

</details>
