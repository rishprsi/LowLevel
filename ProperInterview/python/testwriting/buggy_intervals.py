"""buggy_intervals — THE DRILL: this module claims to work; write pytest
tests that expose its bugs.

Do not fix the code and do not read ANSWERS.md until your test file is
written and run. There are multiple planted bugs.

Claimed behavior of merge_intervals (what your tests should assert):
  - ``merge_intervals(intervals)`` takes a list of ``[start, end]`` pairs
    (start <= end, in any order) and returns a NEW list of merged
    intervals sorted by start.
  - Overlapping intervals merge: [1, 4] and [2, 6] become [1, 6].
  - TOUCHING intervals merge too: [1, 2] and [2, 3] become [1, 3].
  - The input list (and its inner lists) must NOT be modified.
  - An empty input returns [].
"""


def merge_intervals(intervals):
    intervals.sort()
    merged = []
    for interval in intervals:
        if merged and interval[0] <= merged[-1][1]:
            merged[-1][1] = max(merged[-1][1], interval[1])
        else:
            merged.append([interval[0], interval[1]])
    return merged
