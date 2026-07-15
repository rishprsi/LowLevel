from logparse import count_by_status, parse_line, top_paths

GOOD = '127.0.0.1 - - [10/Oct/2025:13:55:36 +0000] "GET /index.html HTTP/1.1" 200 2326'


def test_parse_line_good():
    assert parse_line(GOOD) == {
        "ip": "127.0.0.1",
        "ts": "10/Oct/2025:13:55:36 +0000",
        "method": "GET",
        "path": "/index.html",
        "status": 200,
        "bytes": 2326,
    }


def test_parse_line_trailing_newline():
    assert parse_line(GOOD + "\n")["status"] == 200


def test_parse_line_dash_bytes_is_zero():
    line = '10.0.0.5 - - [10/Oct/2025:14:00:00 +0000] "HEAD /x HTTP/1.1" 304 -'
    parsed = parse_line(line)
    assert parsed["bytes"] == 0
    assert parsed["status"] == 304
    assert parsed["method"] == "HEAD"


def test_parse_line_malformed_returns_none():
    assert parse_line("") is None
    assert parse_line("complete garbage") is None
    # missing [ts] brackets
    assert parse_line('1.2.3.4 - - no-brackets "GET / HTTP/1.1" 200 5') is None
    # request not quoted
    assert parse_line("1.2.3.4 - - [ts] GET / HTTP/1.1 200 5") is None
    # request has only two parts
    assert parse_line('1.2.3.4 - - [ts] "GET /" 200 5') is None
    # non-numeric status
    assert parse_line('1.2.3.4 - - [ts] "GET / HTTP/1.1" abc 5') is None
    # non-numeric bytes (and not "-")
    assert parse_line('1.2.3.4 - - [ts] "GET / HTTP/1.1" 200 xyz') is None
    # missing bytes field entirely
    assert parse_line('1.2.3.4 - - [ts] "GET / HTTP/1.1" 200') is None


def _lines():
    return [
        '1.1.1.1 - - [ts] "GET /a HTTP/1.1" 200 10',
        '2.2.2.2 - - [ts] "GET /b HTTP/1.1" 200 10',
        '3.3.3.3 - - [ts] "GET /a HTTP/1.1" 404 10',
        "not a log line",
        '4.4.4.4 - - [ts] "POST /b HTTP/1.1" 500 10',
        '5.5.5.5 - - [ts] "GET /c HTTP/1.1" 200 10',
        '6.6.6.6 - - [ts] "GET /a HTTP/1.1" 200 10',
    ]


def test_count_by_status():
    assert count_by_status(_lines()) == {200: 4, 404: 1, 500: 1}


def test_count_by_status_empty():
    assert count_by_status([]) == {}
    assert count_by_status(["garbage", ""]) == {}


def test_top_paths():
    # /a: 3, /b: 2, /c: 1
    assert top_paths(_lines(), 2) == [("/a", 3), ("/b", 2)]
    assert top_paths(_lines(), 10) == [("/a", 3), ("/b", 2), ("/c", 1)]


def test_top_paths_tie_broken_alphabetically():
    lines = [
        '1.1.1.1 - - [ts] "GET /z HTTP/1.1" 200 1',
        '1.1.1.1 - - [ts] "GET /m HTTP/1.1" 200 1',
        '1.1.1.1 - - [ts] "GET /a HTTP/1.1" 200 1',
    ]
    assert top_paths(lines, 3) == [("/a", 1), ("/m", 1), ("/z", 1)]


def test_top_paths_n_zero():
    assert top_paths(_lines(), 0) == []
