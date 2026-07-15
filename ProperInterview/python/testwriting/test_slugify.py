from buggy_slugify import slugify


def test_valid():
    valid_string = "Hello Testing ! @ does this work"
    assert slugify(valid_string) == "hello-testing-does-this-work"


def test_empty():
    assert slugify("") == ""


def test_independence():
    assert slugify("Hello") == "hello"
    assert slugify("Hello") == "hello"


def test_leading():
    assert slugify("   Hello") == "hello"
