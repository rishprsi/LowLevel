"""buggy_binary_search — THE DRILL: this module claims to work; write
pytest tests that expose its bugs.

Do not fix the code and do not read ANSWERS.md until your test file is
written and run. There are multiple planted bugs.

Claimed behavior of binary_search (what your tests should assert):
  - ``binary_search(sorted_list, target)`` returns an index i such that
    ``sorted_list[i] == target``, or -1 if target is not present.
  - ``sorted_list`` is a list sorted in ascending order (may contain
    duplicates; any matching index is acceptable).
  - Works for any size including the empty list.
  - Never raises.
"""


def binary_search(sorted_list, target):
    if not sorted_list:
        return -1
    lo = 0
    hi = len(sorted_list) - 1
    while lo <= hi:
        mid = (lo + hi) // 2
        if sorted_list[mid] == target:
            return mid
        if sorted_list[mid] < target:
            lo = mid + 1
        else:
            hi = mid - 1
    return -1
