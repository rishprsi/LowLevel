# Go drills — refresh, not learn

You already ship Go (Boot.dev backend path: MyOwnHTTP, Chirpy, BlogAggregator,
Pokedex, Tubely, rabbitmq/"Peril"). These drills re-warm the muscles Canonical is
most likely to poke: **table-driven tests**, **goroutines/channels**, and
**mutexes/atomics** — with the race detector on, always.

Same red → green pattern as the C repos: the stubs `panic("TODO: ...")` until
you implement them; the `_test.go` files are the spec.

## Commands

```sh
go test ./...            # run everything (red until implemented)
go test -race ./...      # ALWAYS finish with this — it's their -fsanitize=thread
go test -v ./counter     # one drill, verbose
./check-solutions.sh     # run the suite against the reference solutions
git checkout -- dupes/dupes.go workerpool/workerpool.go counter/counter.go   # reset for a re-drill
```

## Drills

| Drill | Target | What it re-warms |
| --- | --- | --- |
| `dupes` | 15 min | slices/maps semantics, table-driven tests — the literally-reported Canonical question, in Go |
| `workerpool` | 30 min | goroutines, channels, `sync.WaitGroup`, ordered fan-out/fan-in — the Go twin of your C/Rust thread pool |
| `counter` | 25–30 min | `sync/atomic` (Inc/Add/Load/CAS) + `sync.Mutex` over a map; when atomic vs mutex; `go test -race` |

## While you're here, speak these aloud (Go semantics list)

- Goroutines vs OS threads; why they're cheap (multiplexed, growable stacks).
- Unbuffered channel = rendezvous; buffered = queue; `select`; closed-channel
  rules (receive → zero value + `ok=false`; send → panic; close twice → panic).
- What `append` really does (backing-array growth + aliasing — you implemented
  exactly this in `../src/dynarr.c`).
- Maps are not concurrency-safe; `sync.Mutex` first, `sync.Map` rarely.
- Single `int64` counter → `atomic`; map or multi-field invariant → `Mutex`.
- The typed-nil-in-interface gotcha; loop-variable capture (pre-1.22 vs now).
- `defer` evaluation order; `errors.Is/As` + `%w` wrapping; `context.Context`
  for cancellation.
- `go test -race` — tie it to your TSan experience when you say it.
