"""buggy_slugify — THE DRILL: this module claims to work; write pytest
tests that expose its bugs.

Do not fix the code and do not read ANSWERS.md until your test file is
written and run. There are multiple planted bugs.

Claimed behavior of slugify (what your tests should assert):
  - ``slugify(title)`` returns a URL-safe slug: lowercase, with every RUN
    of non-alphanumeric characters (spaces, punctuation, ...) collapsed
    into a SINGLE hyphen. "Hello,  World!" -> "hello-world".
  - No leading or trailing hyphens: "  Hi  " -> "hi".
  - Independent calls are independent: calling slugify("Hello") twice
    returns "hello" both times.
  - ``slugify(title, used)`` additionally deduplicates against the list
    ``used`` (and records its result in it): if the slug is taken, a
    numeric suffix is appended ("hello" -> "hello-2" -> "hello-3").
  - An empty or all-punctuation title returns "".
"""


def slugify(title, used=None):
    if used is None:
        used = []
    slug = ""
    for ch in title.lower():
        if ch.isalnum():
            slug += ch
        else:
            if slug and slug[-1] != "-":
                slug += "-"
    slug = slug.strip("-")

    if slug in used:
        n = 2
        while f"{slug}-{n}" in used:
            n += 1
        slug = f"{slug}-{n}"
    used.append(slug)
    return slug
