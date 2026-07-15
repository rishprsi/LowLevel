import pytest

from lru import LRUCache


def test_capacity_must_be_positive():
    with pytest.raises(ValueError):
        LRUCache(0)
    with pytest.raises(ValueError):
        LRUCache(-1)


def test_miss_on_empty():
    c = LRUCache(3)
    assert c.get(1) == -1
    assert len(c) == 0


def test_leetcode_146_sequence():
    c = LRUCache(2)
    c.put(1, 1)
    c.put(2, 2)
    assert c.get(1) == 1
    c.put(3, 3)  # evicts 2
    assert c.get(2) == -1
    c.put(4, 4)  # evicts 1
    assert c.get(1) == -1
    assert c.get(3) == 3
    assert c.get(4) == 4
    assert len(c) == 2


def test_get_promotes():
    c = LRUCache(3)
    c.put(10, 100)
    c.put(20, 200)
    c.put(30, 300)
    assert c.get(10) == 100  # 10 rescued; 20 is now least-recent
    c.put(40, 400)  # evicts 20
    assert c.get(20) == -1
    assert c.get(10) == 100


def test_miss_does_not_change_recency():
    c = LRUCache(2)
    c.put(1, 1)
    c.put(2, 2)
    assert c.get(999) == -1  # miss: order still 1, 2
    c.put(3, 3)  # evicts 1
    assert c.get(1) == -1
    assert c.get(2) == 2


def test_update_promotes_and_never_evicts():
    c = LRUCache(2)
    c.put(1, 1)
    c.put(2, 2)
    c.put(1, 11)  # update + promote; nothing evicted
    assert len(c) == 2
    c.put(3, 3)  # evicts 2, NOT 1
    assert c.get(2) == -1
    assert c.get(1) == 11
    assert c.get(3) == 3


def test_capacity_one():
    c = LRUCache(1)
    c.put(5, 50)
    assert c.get(5) == 50
    c.put(6, 60)  # evicts 5
    assert c.get(5) == -1
    assert c.get(6) == 60
    c.put(6, 66)  # upsert in place
    assert c.get(6) == 66
    assert len(c) == 1


def test_differential_vs_reference():
    """Random walk vs an O(n) reference (a recency-ordered list)."""
    import random

    rng = random.Random(20260708)
    cap = 16
    c = LRUCache(cap)
    ref = []  # (key, value), index 0 = least recent

    def ref_get(key):
        for i, (k, v) in enumerate(ref):
            if k == key:
                ref.append(ref.pop(i))
                return v
        return -1

    def ref_put(key, value):
        for i, (k, _) in enumerate(ref):
            if k == key:
                ref.pop(i)
                break
        else:
            if len(ref) == cap:
                ref.pop(0)
        ref.append((key, value))

    for op in range(2000):
        key = rng.randrange(48)
        if rng.random() < 0.5:
            assert c.get(key) == ref_get(key), f"op={op} key={key}"
        else:
            value = rng.randrange(100000)
            c.put(key, value)
            ref_put(key, value)
        assert len(c) == len(ref), f"op={op} key={key}"

    for k, v in ref:  # final compare in recency order
        assert c.get(k) == v
