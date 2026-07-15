"""lru — the LRU cache again, this time in Python.

Same contract as the C `lru` module — do both and compare: what the C
version needed a hand-rolled hash table and doubly-linked list for, Python
can get from a plain dict (insertion-ordered since 3.7) or
collections.OrderedDict. Being able to articulate that difference IS the
interview answer.

Drill: write 2-3 of your own test cases first, then implement, then run
`pytest exercises/test_lru.py`. Re-drill with
`git checkout -- exercises/lru.py`.
"""


class LRUCache:
    """A least-recently-used cache holding at most *capacity* entries.

    Contract (mirrors src/lru.h on the C side):
      - ``LRUCache(capacity)``: capacity is an int >= 1; raise ValueError
        otherwise.
      - ``get(key)``: if key is present, promote it to most-recently-used
        and return its value; otherwise return -1. A miss must NOT change
        the recency order. (Values are assumed to never be -1, matching
        the C module's sentinel convention.)
      - ``put(key, value)``: insert or update (upsert). Both promote the
        key to most-recently-used. If the key was absent and the cache is
        full, evict the least-recently-used entry first. An update of an
        existing key must never evict.
      - ``len(cache)``: number of entries currently cached (<= capacity).
      - Both get and put must be O(1) amortized.
    """

    def __init__(self, capacity):
        if capacity < 1:
            raise ValueError("Minimum capacity of 1 required")
        self.cache = {}
        self.length = 0
        self.capacity = capacity

    def get(self, key):
        print(self.cache)
        res = self.cache.get(key)

        if not res:
            return -1
        value = self.cache.pop(key)
        self.cache[key] = value
        return value

    def put(self, key, value):
        if key not in self.cache:
            if self.length == self.capacity:
                lrKey = next(iter(self.cache))
                self.cache.pop(lrKey)
            else:
                self.length += 1
        else:
            self.cache.pop(key)
        self.cache[key] = value
        print(self.cache)

    def __len__(self):
        return self.length
