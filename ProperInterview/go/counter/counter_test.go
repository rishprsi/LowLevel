package counter

import (
	"reflect"
	"sync"
	"testing"
)

func TestAtomicCounterBasic(t *testing.T) {
	c := NewAtomicCounter()
	if c.Load() != 0 {
		t.Fatalf("new counter Load() = %d, want 0", c.Load())
	}
	c.Inc()
	c.Inc()
	c.Add(3)
	c.Add(-1)
	if got := c.Load(); got != 4 {
		t.Fatalf("Load() = %d, want 4", got)
	}
}

func TestAtomicCounterCAS(t *testing.T) {
	c := NewAtomicCounter()
	c.Add(10)
	if !c.CompareAndSwap(10, 20) {
		t.Fatal("CAS(10, 20) should succeed")
	}
	if c.Load() != 20 {
		t.Fatalf("Load() = %d, want 20", c.Load())
	}
	if c.CompareAndSwap(10, 99) {
		t.Fatal("CAS(10, 99) should fail when value is 20")
	}
	if c.Load() != 20 {
		t.Fatalf("failed CAS must not change value; Load() = %d", c.Load())
	}
}

func TestAtomicCounterConcurrent(t *testing.T) {
	c := NewAtomicCounter()
	const goroutines = 32
	const per = 1000
	var wg sync.WaitGroup
	wg.Add(goroutines)
	for g := 0; g < goroutines; g++ {
		go func() {
			defer wg.Done()
			for i := 0; i < per; i++ {
				c.Inc()
			}
		}()
	}
	wg.Wait()
	want := int64(goroutines * per)
	if got := c.Load(); got != want {
		t.Fatalf("Load() = %d, want %d (lost updates? use atomic, not plain +=)", got, want)
	}
}

func TestKeyedCounterBasic(t *testing.T) {
	c := NewKeyedCounter()
	if c.Get("a") != 0 {
		t.Fatalf("missing key should be 0")
	}
	c.Inc("a")
	c.Inc("a")
	c.Inc("b")
	c.Inc("")
	if c.Get("a") != 2 || c.Get("b") != 1 || c.Get("") != 1 {
		t.Fatalf("Get mismatch: a=%d b=%d empty=%d", c.Get("a"), c.Get("b"), c.Get(""))
	}
	if c.Total() != 4 {
		t.Fatalf("Total() = %d, want 4", c.Total())
	}
}

func TestKeyedCounterSnapshotIsCopy(t *testing.T) {
	c := NewKeyedCounter()
	c.Inc("x")
	snap := c.Snapshot()
	if !reflect.DeepEqual(snap, map[string]int64{"x": 1}) {
		t.Fatalf("Snapshot() = %v, want map[x:1]", snap)
	}
	snap["x"] = 99
	snap["y"] = 1
	if c.Get("x") != 1 {
		t.Fatalf("mutating Snapshot must not affect counter; Get(x)=%d", c.Get("x"))
	}
	if c.Get("y") != 0 {
		t.Fatalf("mutating Snapshot must not insert keys; Get(y)=%d", c.Get("y"))
	}
	if c.Total() != 1 {
		t.Fatalf("Total() = %d, want 1", c.Total())
	}
}

func TestKeyedCounterSnapshotEmpty(t *testing.T) {
	c := NewKeyedCounter()
	snap := c.Snapshot()
	if snap == nil {
		t.Fatal("Snapshot() returned nil; want empty non-nil map")
	}
	if len(snap) != 0 {
		t.Fatalf("Snapshot() = %v, want empty", snap)
	}
}

func TestKeyedCounterConcurrent(t *testing.T) {
	c := NewKeyedCounter()
	keys := []string{"a", "b", "c", "d"}
	const goroutines = 16
	const per = 500
	var wg sync.WaitGroup
	wg.Add(goroutines)
	for g := 0; g < goroutines; g++ {
		go func(g int) {
			defer wg.Done()
			key := keys[g%len(keys)]
			for i := 0; i < per; i++ {
				c.Inc(key)
			}
		}(g)
	}
	wg.Wait()

	wantTotal := int64(goroutines * per)
	if got := c.Total(); got != wantTotal {
		t.Fatalf("Total() = %d, want %d", got, wantTotal)
	}
	snap := c.Snapshot()
	var sum int64
	for _, v := range snap {
		sum += v
	}
	if sum != wantTotal {
		t.Fatalf("sum(Snapshot) = %d, want %d", sum, wantTotal)
	}
	// 16 goroutines over 4 keys → each key gets 4*500 = 2000
	for _, k := range keys {
		if c.Get(k) != 2000 {
			t.Errorf("Get(%q) = %d, want 2000", k, c.Get(k))
		}
	}
}

// Total must stay correct even when Snapshot is taken concurrently with Inc.
func TestKeyedCounterTotalVsSnapshotRace(t *testing.T) {
	c := NewKeyedCounter()
	var wg sync.WaitGroup
	wg.Add(2)
	go func() {
		defer wg.Done()
		for i := 0; i < 5000; i++ {
			c.Inc("hot")
		}
	}()
	go func() {
		defer wg.Done()
		for i := 0; i < 5000; i++ {
			_ = c.Total()
			_ = c.Snapshot()
			_ = c.Get("hot")
		}
	}()
	wg.Wait()
	if c.Total() != 5000 || c.Get("hot") != 5000 {
		t.Fatalf("Total=%d Get(hot)=%d, want 5000 each", c.Total(), c.Get("hot"))
	}
}
