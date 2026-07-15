from collections import defaultdict
from collections import Counter
import re

"""logparse — parse and aggregate web-server access logs.

Canonical loves this one: it's exactly the kind of ops-adjacent text
processing their engineers do daily.

Drill: write 2-3 of your own test cases first, then implement, then run
`pytest exercises/test_logparse.py`. Re-drill with
`git checkout -- exercises/logparse.py`.
"""

regex = re.compile(
    r"^(?P<ip>\S+) \S+ \S+ \[(?P<ts>[^\]]*)\] "
    r'"(?P<method>\S+) (?P<path>\S+) (?P<proto>\S+)" '
    r"(?P<status>\d+) (?P<bytes>\d+|-)$"
)


def parse_line(line):
    """Parse one common-log-format line into a dict, or None if malformed.

    The expected shape (Apache/nginx "common log" style):

        IP - - [TIMESTAMP] "METHOD /path HTTP/1.1" STATUS BYTES

    e.g.:

        127.0.0.1 - - [10/Oct/2025:13:55:36 +0000] "GET /index.html HTTP/1.1" 200 2326

    Contract:
      - On success return a dict with exactly these keys:
          "ip":     str, the first field, taken as-is (no validation)
          "ts":     str, the text between [ and ] (not parsed further)
          "method": str, e.g. "GET"
          "path":   str, e.g. "/index.html"
          "status": int, e.g. 200
          "bytes":  int; the literal "-" (no body) becomes 0
      - The request field must be a quoted string with exactly three
        space-separated parts: METHOD PATH PROTOCOL.
      - Return None for anything malformed: missing fields, no [ts]
        brackets, unquoted/short request, non-numeric status, non-numeric
        bytes (other than "-"), or the empty string. Never raise.
      - A trailing newline on the line is fine.
    """
    try:
        logDict = {}
        line.strip()
        ip, dash1, dash2, line = line.split(" ", 3)
        logDict["ip"] = ip
        if dash1 != "-" or dash2 != "-":
            return None

        index = line.find("]")
        if index == -1 or line[0] != "[":
            return None
        logDict["ts"] = line[1:index]

        line = line[index + 3 :]
        endIndex = line.find('"')
        if endIndex == -1:
            return None
        url = line[:endIndex]
        method, path, _ = url.split(" ")
        if method not in ["GET", "PUT", "POST", "DELETE", "HEAD"]:
            return None
        logDict["method"] = method
        logDict["path"] = path
        status, size = [x for x in line[endIndex + 1 :].split(" ") if x]
        logDict["status"] = int(status)
        if size == "-":
            logDict["bytes"] = 0
        else:
            logDict["bytes"] = int(size)

        return logDict

    except Exception as e:
        return None


def count_by_status(lines):
    """Aggregate an iterable of raw log lines into {status: count}.

    Malformed lines are skipped silently. Returns a plain dict mapping
    int status code -> number of lines with that status; empty input
    returns {}.
    """
    status_count = defaultdict(int)
    for line in lines:
        dic = parse_line(line)
        if not dic:
            continue
        status_count[dic["status"]] += 1
    return status_count


def top_paths(lines, n):
    """Return the n most-requested paths as (path, count) pairs.

    Malformed lines are skipped silently. Sorted by count DESCENDING,
    ties broken alphabetically (ascending) on the path. At most n pairs;
    n <= 0 returns [].
    """
    if n <= 0:
        return []
    path_count = defaultdict(int)
    for line in lines:
        dic = parse_line(line)
        if not dic:
            continue
        path_count[dic["path"]] += 1

    return sorted(path_count.items(), key=lambda x: (-x[1], x[0]))[:n]
