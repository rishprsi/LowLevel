// Package counter — concurrent counting with atomics and mutexes.
//
// Drill protocol: target 25–30 min. Implement, then:
//
//	go test ./counter
//	go test -race ./counter   // part of the spec — always finish with this
//
// Speak aloud when done: when do you reach for atomic vs Mutex?
// (single word / int → atomic; map or multi-field invariant → Mutex.)
package counter

// AtomicCounter is a race-free int64 counter using sync/atomic only.
// Do not use a Mutex here — that is the point of this type.
type AtomicCounter struct {
	// TODO: unexported state
}

// NewAtomicCounter returns a counter starting at 0.
func NewAtomicCounter() *AtomicCounter {
	panic("TODO: NewAtomicCounter")
}

// Inc adds 1.
func (c *AtomicCounter) Inc() {
	panic("TODO: Inc")
}

// Add adds n (n may be negative).
func (c *AtomicCounter) Add(n int64) {
	panic("TODO: Add")
}

// Load returns the current value.
func (c *AtomicCounter) Load() int64 {
	panic("TODO: Load")
}

// CompareAndSwap atomically sets the value to new if it currently equals old.
// Returns whether the swap happened. Use atomic CAS — do not Load+store under a lock.
func (c *AtomicCounter) CompareAndSwap(old, new int64) bool {
	panic("TODO: CompareAndSwap")
}

// KeyedCounter counts string keys under concurrent access.
//
// Requirements:
//   - Per-key counts live in a map protected by sync.Mutex (or RWMutex).
//   - Total() is the sum of all key counts and must be O(1) — maintain a
//     running total with sync/atomic updated inside Inc, so Total() does
//     not need the map lock.
//   - Snapshot() returns an independent copy of the map (mutating the
//     returned map must not affect the counter).
//   - Get missing key → 0. Empty key "" is a normal key.
//   - Must be race-free under go test -race.
type KeyedCounter struct {
	// TODO: unexported state (mutex + map + atomic total)
}

// NewKeyedCounter returns an empty keyed counter.
func NewKeyedCounter() *KeyedCounter {
	panic("TODO: NewKeyedCounter")
}

// Inc adds 1 to key's count (and to the running total).
func (c *KeyedCounter) Inc(key string) {
	panic("TODO: Inc")
}

// Get returns the count for key, or 0 if never seen.
func (c *KeyedCounter) Get(key string) int64 {
	panic("TODO: Get")
}

// Total returns the sum of all key counts (O(1), no map lock required).
func (c *KeyedCounter) Total() int64 {
	panic("TODO: Total")
}

// Snapshot returns a copy of key → count. Safe to mutate the result.
func (c *KeyedCounter) Snapshot() map[string]int64 {
	panic("TODO: Snapshot")
}
