package workerpool

import (
	"reflect"
	"runtime"
	"sync/atomic"
	"testing"
	"time"
)

func square(x int) int { return x * x }

func TestProcessBasic(t *testing.T) {
	tests := []struct {
		name    string
		items   []int
		workers int
		want    []int
	}{
		{"empty", []int{}, 4, []int{}},
		{"single item", []int{3}, 4, []int{9}},
		{"order preserved", []int{1, 2, 3, 4, 5}, 3, []int{1, 4, 9, 16, 25}},
		{"one worker", []int{2, 3}, 1, []int{4, 9}},
		{"more workers than items", []int{2}, 8, []int{4}},
		{"zero workers treated as one", []int{2, 3}, 0, []int{4, 9}},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			got := Process(tc.items, tc.workers, square)
			if got == nil {
				t.Fatalf("Process returned nil; want empty (non-nil) slice")
			}
			if !reflect.DeepEqual(got, tc.want) {
				t.Errorf("Process(%v, %d) = %v, want %v", tc.items, tc.workers, got, tc.want)
			}
		})
	}
}

// Order must hold even when early jobs finish LAST.
func TestProcessOrderUnderSkew(t *testing.T) {
	items := []int{5, 4, 3, 2, 1, 0}
	got := Process(items, 3, func(x int) int {
		time.Sleep(time.Duration(x) * 2 * time.Millisecond) // item 5 slowest
		return x * 10
	})
	want := []int{50, 40, 30, 20, 10, 0}
	if !reflect.DeepEqual(got, want) {
		t.Errorf("order not preserved under skewed job durations: got %v, want %v", got, want)
	}
}

// Verifies real concurrency: with 4 workers, peak in-flight jobs must exceed 1.
func TestProcessActuallyConcurrent(t *testing.T) {
	if runtime.GOMAXPROCS(0) < 2 {
		t.Skip("needs >1 CPU")
	}
	var inflight, peak int64
	items := make([]int, 32)
	Process(items, 4, func(x int) int {
		cur := atomic.AddInt64(&inflight, 1)
		for {
			p := atomic.LoadInt64(&peak)
			if cur <= p || atomic.CompareAndSwapInt64(&peak, p, cur) {
				break
			}
		}
		time.Sleep(5 * time.Millisecond)
		atomic.AddInt64(&inflight, -1)
		return x
	})
	if atomic.LoadInt64(&peak) < 2 {
		t.Errorf("peak concurrency %d; want >= 2 (are you running jobs sequentially?)", peak)
	}
	if atomic.LoadInt64(&peak) > 4 {
		t.Errorf("peak concurrency %d exceeds worker count 4 (spawning per-item goroutines?)", peak)
	}
}

// A large batch through few workers — the shape -race loves to interrogate.
func TestProcessStress(t *testing.T) {
	n := 5000
	items := make([]int, n)
	for i := range items {
		items[i] = i
	}
	got := Process(items, 4, func(x int) int { return x + 1 })
	for i, v := range got {
		if v != i+1 {
			t.Fatalf("got[%d] = %d, want %d", i, v, i+1)
		}
	}
}
