#ifndef PROPERNET_HTTPARSE_H
#define PROPERNET_HTTPARSE_H

#include <stdbool.h>
#include <stddef.h>

enum {
    HTTP_MAX_HEADERS   = 32,
    HTTP_METHOD_MAX    = 16,  /* including NUL */
    HTTP_TARGET_MAX    = 256, /* including NUL */
    HTTP_VERSION_MAX   = 16,  /* including NUL */
    HTTP_HDR_NAME_MAX  = 64,  /* including NUL */
    HTTP_HDR_VALUE_MAX = 256, /* including NUL */
};

typedef struct {
    char name[HTTP_HDR_NAME_MAX];
    char value[HTTP_HDR_VALUE_MAX];
} HttpHeader;

typedef struct {
    char method[HTTP_METHOD_MAX];   /* "GET", "POST", ... */
    char target[HTTP_TARGET_MAX];   /* e.g. "/index.html" */
    char version[HTTP_VERSION_MAX]; /* e.g. "HTTP/1.1" */
    HttpHeader headers[HTTP_MAX_HEADERS];
    int nheaders;
} HttpRequest;

/*
 * Parse a raw HTTP/1.1 request head from buf (which is NOT necessarily
 * NUL-terminated — only the first len bytes may be read!). Returns true on
 * success, false on any malformed input.
 *
 * Grammar accepted:
 *
 *     METHOD SP TARGET SP VERSION CRLF
 *     ( name ":" [OWS] value CRLF )*
 *     CRLF
 *
 * Rules:
 *  - The header section MUST end with CRLFCRLF ("\r\n\r\n") within the first
 *    len bytes; otherwise reject (the request is truncated/incomplete).
 *    Anything after the CRLFCRLF (a message body) is ignored.
 *  - The request line must be exactly 3 non-empty tokens separated by single
 *    spaces; anything else (2 tokens, 4 tokens, empty tokens) is rejected.
 *  - Every header line must contain a ':' with a non-empty name before it.
 *    The value is everything after the ':' with leading spaces/tabs trimmed
 *    (an empty value is legal).
 *  - More than HTTP_MAX_HEADERS headers is rejected.
 *  - Any NUL byte inside the header section is rejected.
 *  - Any component too long for its fixed-size array is rejected.
 *
 * On failure the contents of *out are unspecified.
 */
bool http_parse_request(const char *buf, size_t len, HttpRequest *out);

/*
 * Return the value of the first header whose name matches `name`
 * CASE-INSENSITIVELY ("Host", "host" and "HOST" are the same header),
 * or NULL if no such header exists.
 */
const char *http_get_header(const HttpRequest *req, const char *name);

#endif /* PROPERNET_HTTPARSE_H */
