#ifndef PROPERNET_URLPARSE_H
#define PROPERNET_URLPARSE_H

#include <stdbool.h>
#include <stdint.h>

enum {
    URL_SCHEME_MAX = 8,   /* including the NUL terminator */
    URL_HOST_MAX   = 128, /* including the NUL terminator */
    URL_PATH_MAX   = 256, /* including the NUL terminator */
};

typedef struct {
    char scheme[URL_SCHEME_MAX]; /* "http" or "https" */
    char host[URL_HOST_MAX];     /* never empty on success */
    uint16_t port;               /* explicit port, else 80 (http) / 443 (https) */
    char path[URL_PATH_MAX];     /* always starts with '/'; defaults to "/" */
} UrlParts;

/*
 * Parse an http/https URL of the shape
 *
 *     scheme://host[:port][/path...]
 *
 * into *out. Returns true on success, false on any malformed input.
 *
 * Rules:
 *  - Only the schemes "http" and "https" are accepted; anything else
 *    (including a missing "scheme://" prefix entirely) is rejected.
 *  - host is everything after "://" up to the first ':' or '/' (or end of
 *    string). An empty host is rejected. IPv4 dotted quads are just hosts.
 *  - If ':' follows the host, the port is the digit run after it: it must be
 *    non-empty, all decimal digits, and <= 65535, otherwise reject. If no
 *    port is given, it defaults to 80 for http and 443 for https.
 *  - path is everything from the first '/' after the host to the end of the
 *    string (query strings and fragments are NOT treated specially — they
 *    just stay part of the path). If absent, path is "/".
 *  - Any component too long for its fixed-size array is rejected.
 *
 * On failure the contents of *out are unspecified.
 */
bool url_parse(const char *url, UrlParts *out);

#endif /* PROPERNET_URLPARSE_H */
