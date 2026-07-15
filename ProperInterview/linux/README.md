# Linux System Skills — Interview Prep Lab

Hands-on prep for a support-engineering style interview (systemd, apt/snaps, netplan,
/proc, diagnosis method). The goal is **method, not trivia**: observe → hypothesize →
verify with the right tool → fix → confirm. Narrate out loud while you work.

## ⚠️ Safety warning

The `breakfix/` labs **deliberately break system state**: they install systemd units,
fill disks, poison DNS, and spawn runaway processes. They all require `sudo`.

**Run them ONLY inside a disposable VM.** Never on your workstation, never on a
machine you care about. Every lab ships a `teardown.sh`, but the whole point of a
throwaway VM is that you don't need to trust it.

## VM setup quickstart

Using Multipass (easiest on macOS/Windows/Linux):

```bash
multipass launch 24.04 --name prep --cpus 2 --memory 4G --disk 20G
multipass shell prep
```

Or with LXD:

```bash
lxc launch ubuntu:24.04 prep
lxc exec prep -- bash
```

Then inside the VM:

```bash
git clone <your-repo-url> lowlevel
cd lowlevel/ProperInterview/linux
```

If a lab wrecks the VM beyond repair: `multipass delete --purge prep` and relaunch.
That's 90 seconds. Don't debug a broken lab VM — that's not the skill being tested.

## How to use this

- **`drills/`** — guided walkthrough exercises. Do one per day. Each has numbered
  command steps with "say this aloud" prompts to train interview narration, plus
  self-check questions at the bottom.
- **`breakfix/`** — timed scenario labs. Run `sudo ./setup.sh` to break the VM,
  start a timer, diagnose and fix it like a customer ticket, then run `./check.sh`
  to confirm. A hint ladder and full walkthrough live in each lab's README (spoilers
  at the bottom — resist!). `sudo ./teardown.sh` resets everything regardless of
  whether you fixed it.
- **`scenarios.md`** — spoken-answer scripts for the classic "walk me through
  diagnosing X" questions (companion vault note has ten playbooks covering labs 01–12).
  Read one before bed; rehearse the 60-second version aloud.

## Contents

### Drills (guided, ~30–45 min each)

| Drill | Topic | Time |
|---|---|---|
| `drills/01-processes-proc-strace.md` | strace, /proc anatomy, lsof, D-state & load average, ps/top | 45 min |
| `drills/02-boot-systemd-services.md` | Boot story, systemctl/journalctl, write & break & fix a unit | 45 min |
| `drills/03-packages-apt-dpkg-snaps.md` | apt/dpkg/pinning/PPAs, recovery, snaps & confinement | 40 min |
| `drills/04-networking-netplan-dns.md` | ip/ss, netplan on 24.04, systemd-resolved, tcpdump, ufw/nft | 45 min |

### Breakfix labs (timed, sudo, disposable VM only)

| Lab | Symptom | Target |
|---|---|---|
| `breakfix/01-service-wont-start/` | New service fails to start after "deployment" | 10 min |
| `breakfix/02-disk-mystery/` | df says disk is filling, du can't find the space | 15 min |
| `breakfix/03-port-conflict/` | Production service crash-loops with EADDRINUSE | 10 min |
| `breakfix/04-dns-sabotage/` | Can ping IPs, hostnames resolve wrong or not at all | 15 min |
| `breakfix/05-permission-denied/` | Service fails with EACCES writing its state dir | 10 min |
| `breakfix/06-runaway-process/` | CPU pegged; the hog comes back after you kill it | 15 min |
| `breakfix/07-cron-never-ran/` | Nightly report file never appears (cron PATH + newline) | 15 min |
| `breakfix/08-cert-expired/` | HTTPS service fails TLS — certificate expired | 15 min |
| `breakfix/09-apparmor-denial/` | Writes denied despite correct POSIX perms (AppArmor) | 15 min |
| `breakfix/10-inode-exhaustion/` | "No space left" but `df -h` looks fine (`df -i`) | 15 min |
| `breakfix/11-user-cant-login/` | SSH key auth silently fails (perms + nologin shell) | 15 min |
| `breakfix/12-clock-skew/` | apt/TLS "not valid yet" — clock in the past (NTP off) | 10 min |

Everything the labs create is tagged: files under `/opt/breakfix-labs/` (and a few
`bf-*` units/users), plus a `# BREAKFIX-LAB` comment in every dropped file.

**Lab caveats (read before you arm):**
- Prefer a **disposable LXD container or VM on the Linux host** (this tree is meant to
  be git-pulled there over Tailscale — not run on your Mac).
- `09-apparmor-denial` needs AppArmor (`aa-status`); skip nested unprivileged containers.
- `12-clock-skew` sets the system clock — requires `sudo BF_YES=1 ./setup.sh`, VM only.
