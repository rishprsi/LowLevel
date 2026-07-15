// Package dupes — the literally-reported Canonical warm-up, in Go.
//
// Drill protocol: start the timer (target 15 min), implement, then
// `go test ./dupes` until green and finish with `go test -race ./dupes`.
package dupes

// FindDuplicates returns every value that appears more than once in items,
// ordered by the position of its FIRST repeat (i.e., the index where the value
// is seen for the second time), each duplicate listed exactly once.
//
//	FindDuplicates([]int{1, 2, 1, 3, 2, 1}) == []int{1, 2}
//	FindDuplicates([]int{})                 == []int{}   (empty, not nil-panic)
//
// Must be O(n) time. Do not modify items.
func FindDuplicates(items []int) []int {
	dupes := make(map[int]struct{})
	found := make(map[int]struct{})

	for _, item := range items {
		_, ok := found[item]
		_, inDupes := dupes[item]
		if ok && !inDupes {
			dupes[item] = struct{}{}
		}
		found[item] = struct{}{}
	}
	res := []int{}
	for key := range dupes {
		res = append(res, key)
	}
	return res
}

// CountOccurrences returns a map of value -> number of appearances.
//
//	CountOccurrences([]int{1, 2, 1}) == map[int]int{1: 2, 2: 1}
func CountOccurrences(items []int) map[int]int {
	counts := map[int]int{}

	for _, item := range items {
		value, ok := counts[item]
		if !ok {
			counts[item] = 1
		} else {
			counts[item] = value + 1
		}
	}
	return counts
}
