# Drill 03 — Packages: apt, dpkg, snaps (40 min)

**Goal:** Be crisp on the apt/dpkg split, survive the classic recovery scenarios, and
hold an informed, respectful opinion on debs vs snaps — this is Canonical.

---

## Part 1 — apt verbs, precisely

```bash
sudo apt update          # refresh package *indexes* only. Changes nothing installed.
sudo apt upgrade         # upgrade installed packages; will INSTALL new deps, but never REMOVES
sudo apt full-upgrade    # upgrade + may remove packages to resolve the new dependency graph
apt list --upgradable
apt show nginx           # metadata: version, deps, description
apt-cache depends nginx  # / rdepends for reverse deps
```

**Say aloud:** "`update` is indexes, `upgrade` is packages — customers conflate them
constantly. `full-upgrade` (= `dist-upgrade`) is the one allowed to remove things,
which is why unattended scripts should think twice before using it."

## Part 2 — apt policy and pinning

```bash
apt policy               # all sources with priorities
apt policy nginx         # installed vs candidate version, and which repo wins
```

Priorities to memorize: **100** = installed, **500** = normal repo, **990** = target
release, **>1000** = pin can force a *downgrade*.

Pin example (`/etc/apt/preferences.d/nginx-pin`):

```
Package: nginx
Pin: origin ppa.launchpadcontent.net
Pin-Priority: 700
```

**Say aloud:** "`apt policy <pkg>` is how I answer 'why is apt picking that version?' —
it shows candidate selection with the priorities right there."

Unattended upgrades (on by default for security on Ubuntu):

```bash
cat /etc/apt/apt.conf.d/20auto-upgrades          # the on/off switches
grep -v '^\s*//' /etc/apt/apt.conf.d/50unattended-upgrades | grep -v '^\s*$' | head
sudo unattended-upgrade --dry-run --debug 2>&1 | tail -20
journalctl -u apt-daily-upgrade.timer            # it's driven by systemd timers
```

## Part 3 — dpkg: when and why

apt = dependency resolver + downloader; dpkg = the actual package database and
installer underneath. Use dpkg for *questions* and *local files*:

```bash
dpkg -l | head                 # installed packages (ii = installed ok)
dpkg -L openssh-server        # what files did this package install?
dpkg -S /usr/sbin/sshd        # which package owns this file?
sudo dpkg -i ./something.deb  # install a local .deb (no dep resolution!)
sudo apt install -f           # ...then fix the missing deps apt-style
# modern alternative that does both at once:
sudo apt install ./something.deb
```

Holds:

```bash
sudo apt-mark hold linux-image-generic     # freeze a package across upgrades
apt-mark showhold
sudo apt-mark unhold linux-image-generic
```

**Say aloud:** "`dpkg -S` on a mystery file, `dpkg -L` on a mystery package — those two
answer most 'where did this come from' tickets."

## Part 4 — PPAs: add, inspect, remove safely

```bash
sudo add-apt-repository ppa:deadsnakes/ppa
ls /etc/apt/sources.list.d/                 # the PPA lands here (.sources file, deb822 format on 24.04)
cat /etc/apt/sources.list.d/deadsnakes-ubuntu-ppa-noble.sources
apt policy | grep -A1 deadsnakes            # confirm priority
```

Remove **safely** — downgrade its packages back to archive versions first:

```bash
sudo apt install ppa-purge
sudo ppa-purge ppa:deadsnakes/ppa    # reverts packages, then disables the PPA
# vs. add-apt-repository --remove: removes the source but LEAVES the PPA's packages
```

**Say aloud:** "On 24.04 sources are deb822 `.sources` files and PPA keys are scoped
via `Signed-By` — no more global apt-key. If a customer removed a PPA but their
packages still misbehave, they likely still run the PPA's versions; `ppa-purge` is
the clean exit."

## Part 5 — recovery story: `dpkg --configure -a`

The classic ticket: an upgrade was interrupted (Ctrl-C, power loss, full disk) and now
every apt command says *"dpkg was interrupted, you must manually run 'sudo dpkg
--configure -a'"*.

The story to tell aloud:

1. dpkg unpacks files, then runs each package's `postinst` script; state is tracked in
   `/var/lib/dpkg/status`. Interruption leaves packages half-configured, and dpkg's
   lock/journal makes apt refuse to proceed.
2. `sudo dpkg --configure -a` — finish configuring everything left unconfigured.
3. `sudo apt install -f` — repair any broken dependencies.
4. If a *lock* is the complaint (`could not get lock /var/lib/dpkg/lock-frontend`):
   find the holder with `sudo fuser -v /var/lib/dpkg/lock-frontend` or `lsof` — it's
   usually unattended-upgrades mid-run. **Wait or let it finish; never delete lock
   files while dpkg runs.**
5. Package's own postinst keeps failing? Read it: `/var/lib/dpkg/info/<pkg>.postinst`.

## Part 6 — snaps

```bash
snap list                     # note: base snaps (core22/24), snapd itself
sudo snap install hello-world
sudo snap refresh --list      # pending updates (snapd auto-refreshes ~4x/day)
snap info hello-world         # channels: latest/stable, latest/candidate, ...
sudo snap refresh hello-world --channel=latest/edge
sudo snap revert hello-world  # roll back to the previous revision — killer feature
snap services                 # snap-shipped daemons
sudo snap set system refresh.hold=48h   # defer auto-refresh
```

Where snaps live — and those `df` loop devices:

```bash
ls /snap                       # mount points per snap revision
ls /var/lib/snapd/snaps/       # the .snap files themselves (squashfs images)
df -h | grep -E '/dev/loop'    # each squashfs mounted read-only via a loop device
```

**Say aloud:** "Every snap is a read-only squashfs image loop-mounted under
`/snap/<name>/<revision>` — that's why `df` shows a pile of 100%-full loop devices.
That's *normal and healthy*: they're read-only images, 100% full by construction.
Writable data lives in `/var/snap` (system) and `~/snap` (user)."

Confinement:

```bash
snap info --verbose hello-world | grep confinement
snap connections firefox 2>/dev/null | head   # interfaces = the permission system
```

- **strict** — sandboxed (AppArmor, seccomp, namespaces); access mediated by
  *interfaces* you can connect/disconnect.
- **classic** — no confinement, full system access (needs `--classic` at install;
  e.g. IDEs, toolchains that must see the whole filesystem).

## Part 7 — deb vs snap, the opinionated minute

Rehearse aloud, balanced but with a stance:

> "Debs are ideal for the base system: shared libraries, tight archive integration,
> security fixes flow through one pipeline, minimal disk and memory overhead. Snaps
> shine for applications: the publisher ships one artifact across releases with its
> dependencies bundled, updates are transactional with `snap revert` as an instant
> rollback, strict confinement is a real security win, and channels give users a
> stable/edge choice per app. Costs are real too — disk from bundling, first-launch
> latency, and auto-refresh timing that needs managing on servers (`refresh.hold`,
> or snap proxies in fleets). My rule of thumb: base system and libraries as debs;
> fast-moving or vendor-shipped apps as snaps."

---

## Self-check questions

1. Exactly what does `apt update` change on the system?
2. When would `apt upgrade` hold a package back that `apt full-upgrade` would upgrade?
3. A file `/usr/bin/mystery` appeared. Two commands to find out where it came from?
4. What does a pin priority above 1000 allow that lower priorities don't?
5. Walk the full recovery from "dpkg was interrupted" including the lock-file variant.
6. Why does `df` on Ubuntu show a dozen loop devices at 100% use, and is it a problem?
7. Difference between strict and classic confinement; how do you know which one a
   snap uses?
8. A customer removed a PPA with `add-apt-repository --remove` but the bug persists.
   Why, and what's the right tool?

<details>
<summary><strong>Answers</strong></summary>

1. Only the package indexes under `/var/lib/apt/lists/` — the local copy of what each
   repo offers. No installed package changes.
2. When the upgrade requires removing an installed package or installing conflicting
   new deps in a way `upgrade`'s no-removal rule forbids — common across major
   transitions (new kernel meta-deps, library renames).
3. `dpkg -S /usr/bin/mystery` (owning package, if any). If dpkg doesn't know it, it
   wasn't installed by a deb — check `snap list`, `pip`, or `ls -l /proc/<pid>/exe`
   if it's running. Then `dpkg -L <pkg>` / `apt policy <pkg>` for the source repo.
4. Downgrades: apt may install a version *older* than the installed one to satisfy
   the pin.
5. `sudo dpkg --configure -a`, then `sudo apt install -f`. If blocked on the lock:
   `fuser -v /var/lib/dpkg/lock-frontend` to identify the holder (often
   unattended-upgrades), wait for it; only remove a lock if the holder is truly dead.
   Persistent postinst failures: read `/var/lib/dpkg/info/<pkg>.postinst`.
6. Each installed snap revision is a read-only squashfs image loop-mounted under
   `/snap/...`. Read-only filesystems are always "100% full". Cosmetic, not a
   capacity issue.
7. Strict = sandboxed by AppArmor/seccomp/namespaces with access granted through
   snap interfaces; classic = unconfined full system access. Check with
   `snap info <name>` (notes/confinement) or `snap list` (classic shows in Notes).
8. Removing the source only stops future updates from it — the PPA's package versions
   remain installed. `ppa-purge` downgrades them back to archive versions before
   disabling the PPA.

</details>
