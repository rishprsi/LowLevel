#ifndef PROPERDS_STRDP_H
#define PROPERDS_STRDP_H

#include <stddef.h>

/*
 * strdp — string algorithms and classic dynamic programming.
 *
 * prefix_function: the KMP failure array. For s of length L, fill
 *   pi[i] = length of the longest PROPER prefix of s[0..i] that is also a
 *           suffix of s[0..i]   (so pi[0] == 0 always).
 * The caller guarantees pi has room for strlen(s) ints. L == 0 is a no-op.
 *
 * kmp_search: index of the FIRST occurrence of needle in haystack, or -1.
 * Edge cases that must work:
 *   - empty needle matches at index 0 (even in an empty haystack)
 *   - needle longer than haystack -> -1
 *   - needle == haystack -> 0
 * Must run in O(|haystack| + |needle|) — that's the point of KMP.
 *
 * edit_distance: Levenshtein distance between a and b — the minimum number
 * of single-character insertions, deletions, and substitutions turning a
 * into b.
 *
 * lis_length: length of the longest STRICTLY increasing subsequence of
 * a[0..n) (not necessarily contiguous). n == 0 -> 0.
 *
 * coin_change_min: fewest coins from coins[0..ncoins) (each usable
 * unlimited times, all values > 0) summing exactly to amount, or -1 if
 * impossible. amount == 0 -> 0.
 *
 * knapsack_01: maximum total value from items 0..n-1 (weights[i],
 * values[i], each item usable AT MOST ONCE) with total weight <= capacity.
 * All weights/values >= 0.
 */

/* KMP failure array for s into pi[0 .. strlen(s)). */
void prefix_function(const char *s, int *pi);

/* First occurrence of needle in haystack, or -1. */
int kmp_search(const char *haystack, const char *needle);

/* Levenshtein distance. */
int edit_distance(const char *a, const char *b);

/* Longest strictly increasing subsequence length. */
int lis_length(const int *a, size_t n);

/* Fewest coins summing to amount, or -1. */
int coin_change_min(const int *coins, size_t ncoins, int amount);

/* 0/1 knapsack: max value within weight capacity. */
int knapsack_01(const int *weights, const int *values, size_t n,
                int capacity);

#endif /* PROPERDS_STRDP_H */
