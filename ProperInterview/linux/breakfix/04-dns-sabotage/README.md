# Lab 04 — DNS sabotage

**Ticket #5433** · Priority: High · Time target: **15 minutes**

> "Since a contractor 'optimized DNS' on this box, everything is weird. `ping 1.1.1.1`
> works fine, but most hostnames won't resolve at all — and bizarrely, `ubuntu.com`
> resolves *instantly* but to an address that can't be right. Please make name
> resolution sane again."

## Your job

Restore normal DNS: hostnames resolve correctly through systemd-resolved's normal
upstream path, and nothing overrides well-known names. Then run `./check.sh`.

```bash
sudo ./setup.sh
# ... diagnose & fix ...
./check.sh
sudo ./teardown.sh
```

## Hint ladder (only if stuck)

<details><summary>Hint 1 (vague)</summary>

Two separate symptoms usually means two separate breakages. "Resolves instantly but
wrong" and "doesn't resolve at all" go through different parts of the lookup path.
Compare `getent hosts ubuntu.com` with `resolvectl query ubuntu.com` — why would they
disagree?

</details>

<details><summary>Hint 2 (warmer)</summary>

`getent` follows `/etc/nsswitch.conf`, and `files` (= `/etc/hosts`) wins before DNS
is ever asked — read `/etc/hosts` top to bottom. For the "nothing resolves" half:
`resolvectl status` — look hard at the **Global** section's DNS server. Where does
resolved's global config come from, and what directory can *drop-ins* extend it from?

</details>

<details><summary>Hint 3 (nearly the answer)</summary>

(1) `/etc/hosts` has a poisoned `ubuntu.com` line (tagged `# BREAKFIX-LAB`) — delete
it. (2) `/etc/systemd/resolved.conf.d/99-dns-optimizer.conf` points global DNS at
192.0.2.1 (a black-hole test address) with `Domains=~.` so *every* query routes
there. Remove the drop-in, `systemctl restart systemd-resolved`, verify with
`resolvectl status` and `resolvectl query ubuntu.com`.

</details>

---

<details><summary><strong>⚠️ SPOILERS — full diagnosis walkthrough</strong></summary>

1. Split the symptom. **Say aloud:** "IP connectivity works, names don't — this is
   resolution, and I'll bisect the path: /etc/hosts → stub → resolved → upstream."
2. `getent hosts ubuntu.com` → returns 203.0.113.66 instantly. But
   `resolvectl query ubuntu.com` → fails/times out. **Say aloud:** "getent goes
   through nsswitch, so /etc/hosts answered; resolvectl goes to resolved. Two
   different failures."
3. `grep ubuntu /etc/hosts` → `203.0.113.66 ubuntu.com  # BREAKFIX-LAB`. Hosts-file
   precedence: the `hosts:` line in `/etc/nsswitch.conf` lists `files` before
   `resolve`/`dns`. Delete the poisoned line.
4. Now the global failure. `resolvectl status` → Global section shows
   `DNS Servers: 192.0.2.1` (a TEST-NET black hole) and `DNS Domain: ~.` — the `~.`
   routing domain forces **all** queries to the global server, overriding per-link
   DHCP servers.
5. Where is that configured? Not `/etc/systemd/resolved.conf` itself — check drop-ins:
   `ls /etc/systemd/resolved.conf.d/` → `99-dns-optimizer.conf`.
   (`systemd-analyze cat-config systemd/resolved.conf` shows the whole merged view.)
6. `sudo rm /etc/systemd/resolved.conf.d/99-dns-optimizer.conf` then
   `sudo systemctl restart systemd-resolved`.
7. Verify: `resolvectl status` shows sane per-link servers again;
   `resolvectl query ubuntu.com` and `getent hosts ubuntu.com` both return real
   addresses; `sudo resolvectl flush-caches` if something stale lingers.
8. `./check.sh` goes green.

**Interview takeaways:** getent-vs-dig/resolvectl disagreement fingers /etc/hosts;
resolved config = main file + `resolved.conf.d/` drop-ins (`systemd-analyze
cat-config` shows everything); `Domains=~.` makes a global DNS server hijack all
queries; TEST-NET addresses (192.0.2.x, 203.0.113.x) are documentation/black-hole
ranges worth recognizing.

</details>
