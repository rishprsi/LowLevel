// Package workerpool — the Go twin of your C thread pool (P3.3) and Rust
// ThreadPool (ProperRust): fan work out to N goroutines, collect results in
// input order.
//
// Drill protocol: target 30 min. Green under `go test ./workerpool`, then
// ALWAYS `go test -race ./workerpool` — an unsynchronized results write is
// exactly the bug the race detector exists for.
package workerpool

// Process applies fn to every item using exactly `workers` concurrent
// goroutines, returning results in the SAME ORDER as items.
//
// Requirements:
//   - Spawn `workers` goroutines (not one per item) reading from a shared
//     jobs channel — this is the worker-pool shape, not fan-out-per-item.
//   - Results must land in input order without sorting afterward (hint: pass
//     each job's index along with its value).
//   - Must be race-free: `go test -race` is part of the spec.
//   - workers <= 0 or empty items: return an empty (non-nil) slice for empty
//     items; treat workers <= 0 as workers == 1.
//
import "sync"

// The classic composition: jobs channel + results-with-index channel +
// sync.WaitGroup to know when to close results (or preallocate the results
// slice and have workers write disjoint indices — both are correct; be ready
// to say why both are race-free).
type job struct {
	index int
	val   int
}

func Process(items []int, workers int, fn func(int) int) []int {
	results := make([]int, len(items))
	if len(items) == 0 {
		return results
	}

	if workers <= 0 {
		workers = 1
	}

	var wg sync.WaitGroup
	wg.Add(workers)

	jobs := make(chan job)

	for i := 0; i < workers; i++ {
		go func() {
			defer wg.Done()
			for job := range jobs {
				results[job.index] = fn(job.val)
			}
		}()
	}

	for index, item := range items {
		jobs <- job{index, item}
	}
	close(jobs)
	wg.Wait()

	return results
}
