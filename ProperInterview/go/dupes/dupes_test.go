package dupes

import (
	"reflect"
	"testing"
)

// Table-driven — the idiom Canonical's Go reviewers expect to see.
func TestFindDuplicates(t *testing.T) {
	tests := []struct {
		name  string
		items []int
		want  []int
	}{
		{"empty", []int{}, []int{}},
		{"nil slice", nil, []int{}},
		{"no duplicates", []int{1, 2, 3}, []int{}},
		{"one duplicate", []int{1, 2, 1}, []int{1}},
		{"order of first repeat", []int{1, 2, 1, 3, 2, 1}, []int{1, 2}},
		{"triple counts once", []int{5, 5, 5, 5}, []int{5}},
		{"repeat order not value order", []int{9, 4, 4, 9}, []int{4, 9}},
		{"negatives and zero", []int{0, -1, 0, -1}, []int{0, -1}},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			got := FindDuplicates(tc.items)
			if got == nil {
				t.Fatalf("FindDuplicates(%v) returned nil; want empty (non-nil) slice", tc.items)
			}
			if !reflect.DeepEqual(got, tc.want) {
				t.Errorf("FindDuplicates(%v) = %v, want %v", tc.items, got, tc.want)
			}
		})
	}
}

func TestFindDuplicatesDoesNotMutateInput(t *testing.T) {
	items := []int{3, 1, 3, 2}
	FindDuplicates(items)
	if !reflect.DeepEqual(items, []int{3, 1, 3, 2}) {
		t.Errorf("input slice was mutated: %v", items)
	}
}

func TestCountOccurrences(t *testing.T) {
	tests := []struct {
		name  string
		items []int
		want  map[int]int
	}{
		{"empty", []int{}, map[int]int{}},
		{"basic", []int{1, 2, 1}, map[int]int{1: 2, 2: 1}},
		{"single", []int{7}, map[int]int{7: 1}},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			got := CountOccurrences(tc.items)
			if !reflect.DeepEqual(got, tc.want) {
				t.Errorf("CountOccurrences(%v) = %v, want %v", tc.items, got, tc.want)
			}
		})
	}
}
