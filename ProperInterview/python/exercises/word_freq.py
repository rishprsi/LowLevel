from collections import Counter
import re

"""word_freq — counting with a tie-break, the classic Counter question.

Drill: write 2-3 of your own test cases first, then implement, then run
`pytest exercises/test_word_freq.py`. Re-drill with
`git checkout -- exercises/word_freq.py`.
"""


def top_n_words(text, n):
    """Return the *n* most frequent words in *text* as (word, count) pairs.

    Contract:
      - Case-insensitive: "The" and "the" are the same word, reported in
        lowercase.
      - A word is a maximal run of letters and digits; every other
        character (punctuation, whitespace, hyphens, ...) is a separator.
        "don't" is therefore the two words "don" and "t".
      - Result is sorted by count DESCENDING; ties are broken
        ALPHABETICALLY (ascending) on the word.
      - Returns at most n pairs; if the text has fewer than n distinct
        words, return them all. n <= 0 returns [].
      - Empty text (or text with no letters/digits) returns [].

    Examples:
        >>> top_n_words("the cat and the hat", 2)
        [('the', 2), ('and', 1)]
        >>> top_n_words("B b a A", 2)
        [('a', 2), ('b', 2)]
    """
    if not text:
        return []
    text = re.sub(r"[^A-Za-z0-9]", " ", text)
    text = [word.lower() for word in text.split(" ") if word]
    counts = Counter(text)
    print(text)
    res = []
    for count in sorted(counts, key=lambda x: (-counts[x], x))[:n]:
        res.append((count, counts[count]))

    return res
