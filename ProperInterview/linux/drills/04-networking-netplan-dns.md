# Drill 04 — Networking, netplan, DNS on Ubuntu 24.04 (45 min)

**Goal:** Drive `ip`/`ss` reflexively, edit netplan safely with `try`, narrate the
modern Ubuntu DNS path, and run the "ping IP works, hostname doesn't" chain without
thinking.

---

## Part 1 — ip and ss, the reflexes

```bash
ip a                        # addresses + link state (UP, LOWER_UP)
ip r                        # routing table; find "default via ..."
ip -s link                  # per-interface counters: errors, dropped, overruns
ip neigh                    # ARP/neighbor table (REACHABLE/STALE/FAILED)
ss -tlnp                    # TCP listeners, numeric, with owning process (sudo for all PIDs)
ss -ulnp                    # UDP
ss -tnp state established   # live connections
ss -s                       # socket summary counts
```

**Say aloud:**
- On `ip a`: "I check state UP and LOWER_UP — LOWER_UP means carrier, cable/virtual
  link is actually up. Then does it have the address I expect?"
- On `ip r`: "No default route explains 'local network fine, internet dead' instantly."
- On `ip -s link`: "Errors/dropped climbing means a layer-1/2 problem — bad cable,
  duplex mismatch, or an overwhelmed NIC — before I blame anything higher."
- On `ss -tlnp`: "Old habit is netstat; `ss` is the modern tool and shows the PID with
  `-p`. Listener on 127.0.0.1 vs 0.0.0.0 is the classic 'works locally, not remotely'."

## Part 2 — netplan on 24.04

Netplan is a YAML front-end that *renders* config for a backend: **systemd-networkd**
on servers, **NetworkManager** on desktops.

```bash
ls /etc/netplan/                     # server default: 50-cloud-init.yaml
sudo cat /etc/netplan/50-cloud-init.yaml
networkctl                           # networkd's view of links (server)
sudo netplan get                     # the merged config netplan sees
```

A typical cloud default:

```yaml
network:
  version: 2
  ethernets:
    ens3:
      dhcp4: true
```

Add a static address on a second interface — write a *new* file (files merge
lexically; higher numbers win):

```bash
sudo tee /etc/netplan/60-second-nic.yaml >/dev/null <<'EOF'
network:
  version: 2
  ethernets:
    ens4:
      dhcp4: false
      addresses: [192.168.50.10/24]
      routes:
        - to: default
          via: 192.168.50.1
      nameservers:
        addresses: [1.1.1.1, 8.8.8.8]
EOF
sudo chmod 600 /etc/netplan/60-second-nic.yaml   # netplan warns on world-readable files
```

(Adjust `ens4` to a real second interface, or practice the YAML on the primary with
extreme care. Note `routes:` with `to: default` — the old `gateway4:` key is
deprecated.)

Apply **safely**:

```bash
sudo netplan try       # applies, then AUTO-REVERTS in 120s unless you press Enter
sudo netplan apply     # applies for real
sudo netplan --debug generate   # dry-run render to /run/systemd/network, no apply
```

**Say aloud:** "On a remote box I always use `netplan try` — if my YAML kills SSH,
it rolls back on its own in two minutes. That habit has saved careers. If cloud-init
manages the file, I either disable its network module or my edits get regenerated."

## Part 3 — DNS on modern Ubuntu: the resolution path

The chain, narrated:

1. App calls `getaddrinfo()` → glibc consults **`/etc/nsswitch.conf`**, `hosts:` line:
   `files` first (**/etc/hosts**), then `resolve` (systemd-resolved via nss-resolve),
   then `dns`.
2. `/etc/resolv.conf` is a symlink to the **stub**: `nameserver 127.0.0.53` —
   systemd-resolved's local listener.
3. **systemd-resolved** caches, and forwards to the real upstream servers it learned
   per-link (from DHCP or netplan).

Verify each hop:

```bash
grep ^hosts /etc/nsswitch.conf
ls -l /etc/resolv.conf; cat /etc/resolv.conf     # → ../run/systemd/resolve/stub-resolv.conf
resolvectl status                                # per-link upstream DNS servers
resolvectl query ubuntu.com                      # resolve THROUGH resolved (shows which link answered)
getent hosts ubuntu.com                          # resolve through the FULL nss chain (includes /etc/hosts)
dig ubuntu.com                                   # talks straight to 127.0.0.53, BYPASSES /etc/hosts
dig @1.1.1.1 ubuntu.com                          # bypass resolved entirely — test upstream directly
resolvectl statistics                            # cache hits
sudo resolvectl flush-caches
```

**Say aloud:** "The key diagnostic distinction: `getent hosts` goes through nsswitch —
it sees `/etc/hosts`. `dig` speaks raw DNS to the stub — it does *not* see /etc/hosts.
When those two disagree, the hosts file is the suspect."

Watch DNS on the wire:

```bash
sudo tcpdump -ni any port 53
# in another terminal: resolvectl query example.com
```

**Say aloud:** "Query to 127.0.0.53 stays on loopback; then I see resolved forwarding
to the real upstream on the outbound interface. If nothing leaves the box, the problem
is local config, not the network."

## Part 4 — the classic: "can ping the IP but not the hostname"

The spoken chain, in order:

```bash
ping -c1 1.1.1.1                 # IP works → routing/link fine, it's name resolution
getent hosts example.com         # full nss path — catches /etc/hosts poisoning
grep example /etc/hosts          # anything hijacking it?
resolvectl query example.com     # does resolved itself resolve it?
resolvectl status                # sane upstream servers on the right link?
dig @<upstream-ip> example.com   # is the upstream itself healthy/reachable?
sudo tcpdump -ni any port 53     # are queries leaving? answers returning? (firewall?)
```

**Say aloud:** "I bisect the chain: hosts file → stub → resolved's upstream → the
upstream itself → the wire. Each command isolates one hop."

## Part 5 — firewall basics

```bash
sudo ufw status verbose          # inactive by default on fresh Ubuntu
sudo ufw allow 22/tcp            # ALWAYS allow SSH before enabling, remotely
sudo ufw enable
sudo ufw status numbered; sudo ufw delete <n>
sudo nft list ruleset | head -40 # what's ACTUALLY loaded in the kernel (nftables)
sudo iptables -L -n | head       # legacy view via iptables-nft translation
```

**Say aloud:** "ufw is a front-end; the kernel truth on 24.04 is nftables —
`nft list ruleset` is the ground truth when ufw's view and reality disagree, e.g.
when Docker has injected its own rules."

---

## Self-check questions

1. What do UP vs LOWER_UP mean on `ip a`, and which one tells you about the cable?
2. Why does `/etc/resolv.conf` point at 127.0.0.53, and what is listening there?
3. `getent hosts foo.example` returns 10.0.0.99 but `dig foo.example` returns
   93.184.216.34. What's going on?
4. What does `netplan try` do that `netplan apply` doesn't, and when is it essential?
5. On a 24.04 server, which daemon actually configures interfaces from netplan YAML?
   And on desktop?
6. How do you see which upstream DNS server each interface uses?
7. A service is reachable from the box itself but not from the network, yet the
   process is running. First command, and the two most likely findings?
8. ufw says port 8080 is allowed, but connections still fail. Name two ways the
   kernel ruleset could differ from ufw's view, and the command that shows the truth.

<details>
<summary><strong>Answers</strong></summary>

1. UP = interface administratively enabled; LOWER_UP = physical/virtual carrier
   detected. LOWER_UP is the cable (or virtual link) answer.
2. It's the systemd-resolved stub resolver — a local caching listener on
   127.0.0.53:53 that forwards to per-link upstream servers. Apps get caching and
   per-link DNS routing without knowing the real upstreams.
3. They use different paths: getent follows nsswitch and reads `/etc/hosts` first;
   dig speaks DNS directly and skips it. An `/etc/hosts` entry (10.0.0.99) is
   overriding DNS.
4. `try` applies the config and automatically reverts after 120 seconds unless you
   confirm — essential when editing network config over SSH, where a bad config
   locks you out.
5. Server: systemd-networkd (netplan renders to `/run/systemd/network`). Desktop:
   NetworkManager.
6. `resolvectl status` — lists DNS servers per link, plus the global config.
7. `ss -tlnp` (with sudo). Most likely: the service listens on 127.0.0.1 instead of
   0.0.0.0, or a firewall blocks the port (then check `nft list ruleset` / ufw).
8. Docker or other software injecting nftables/iptables rules that ufw doesn't
   manage; rules changed manually or a table taking precedence over ufw's chains.
   `sudo nft list ruleset` shows what the kernel actually enforces.

</details>
