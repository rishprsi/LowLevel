from collections import Counter

"""duplicates — a literally-reported Canonical screening question.

Drill: write 2-3 of your own test cases first, then implement, then run
`pytest exercises/test_duplicates.py`. Re-drill with
`git checkout -- exercises/duplicates.py`.
"""


def find_duplicates(items):
    """Return the values that appear more than once in *items*.

    Contract:
      - ``items`` is any iterable of hashable values.
      - A value appears in the result at most ONCE, no matter how many
        times it repeats in the input.
      - Order: by *first repeat* — the moment a value is seen for the
        second time determines its position. ``[1, 2, 2, 1]`` returns
        ``[2, 1]`` (2's second occurrence comes before 1's).
      - Values that appear exactly once do not appear at all.
      - An empty input (or one with no repeats) returns ``[]``.
      - The input must NOT be mutated.

    Examples:
        >>> find_duplicates([1, 2, 3, 2, 1, 2])
        [2, 1]
        >>> find_duplicates(["a", "b", "c"])
        []
    """
    res = []
    found = set()
    added = set()
    for item in items:
        if item in found and item not in added:
            res.append(item)
            added.add(item)
        else:
            found.add(item)

    return res
