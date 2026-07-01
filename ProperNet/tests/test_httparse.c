#include "ctest.h"
#include "httparse.h"

#include <string.h>

int main(void) {
    HttpRequest req;

    /* --- a well-formed GET --- */
    const char *get =
        "GET /index.html HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "User-Agent:  test/1.0\r\n" /* extra whitespace after colon */
        "X-Empty:\r\n"              /* empty value is legal */
        "\r\n";
    CHECK_TRUE(http_parse_request(get, strlen(get), &req));
    CHECK_STR_EQ(req.method, "GET");
    CHECK_STR_EQ(req.target, "/index.html");
    CHECK_STR_EQ(req.version, "HTTP/1.1");
    CHECK_INT_EQ(req.nheaders, 3);
    CHECK_STR_EQ(req.headers[0].name, "Host");
    CHECK_STR_EQ(req.headers[0].value, "example.com");
    CHECK_STR_EQ(req.headers[1].value, "test/1.0"); /* whitespace trimmed */
    CHECK_STR_EQ(req.headers[2].value, "");

    /* case-insensitive lookup */
    CHECK_STR_EQ(http_get_header(&req, "Host"), "example.com");
    CHECK_STR_EQ(http_get_header(&req, "host"), "example.com");
    CHECK_STR_EQ(http_get_header(&req, "HOST"), "example.com");
    CHECK_STR_EQ(http_get_header(&req, "uSeR-aGeNt"), "test/1.0");
    CHECK_PTR_NULL(http_get_header(&req, "Content-Length"));

    /* --- POST with a body: body is ignored, head parses --- */
    const char *post =
        "POST /submit HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Content-Length: 5\r\n"
        "\r\n"
        "hello";
    CHECK_TRUE(http_parse_request(post, strlen(post), &req));
    CHECK_STR_EQ(req.method, "POST");
    CHECK_STR_EQ(req.target, "/submit");
    CHECK_INT_EQ(req.nheaders, 2);
    CHECK_STR_EQ(http_get_header(&req, "content-length"), "5");

    /* --- request with no headers at all --- */
    const char *bare = "GET / HTTP/1.1\r\n\r\n";
    CHECK_TRUE(http_parse_request(bare, strlen(bare), &req));
    CHECK_STR_EQ(req.method, "GET");
    CHECK_INT_EQ(req.nheaders, 0);

    /* --- malformed: missing CRLFCRLF terminator --- */
    const char *unterminated = "GET / HTTP/1.1\r\nHost: x\r\n";
    CHECK_FALSE(http_parse_request(unterminated, strlen(unterminated), &req));

    /* bare LF is not a terminator */
    const char *bare_lf = "GET / HTTP/1.1\nHost: x\n\n";
    CHECK_FALSE(http_parse_request(bare_lf, strlen(bare_lf), &req));

    /* --- malformed request lines --- */
    const char *two_tokens = "GET /\r\n\r\n";
    CHECK_FALSE(http_parse_request(two_tokens, strlen(two_tokens), &req));
    const char *four_tokens = "GET / HTTP/1.1 extra\r\n\r\n";
    CHECK_FALSE(http_parse_request(four_tokens, strlen(four_tokens), &req));
    const char *double_space = "GET  / HTTP/1.1\r\n\r\n"; /* empty token */
    CHECK_FALSE(http_parse_request(double_space, strlen(double_space), &req));
    const char *empty_line = "\r\n\r\n";
    CHECK_FALSE(http_parse_request(empty_line, strlen(empty_line), &req));

    /* --- malformed headers --- */
    const char *no_colon = "GET / HTTP/1.1\r\nBadHeader\r\n\r\n";
    CHECK_FALSE(http_parse_request(no_colon, strlen(no_colon), &req));
    const char *empty_name = "GET / HTTP/1.1\r\n: value\r\n\r\n";
    CHECK_FALSE(http_parse_request(empty_name, strlen(empty_name), &req));

    /* --- embedded NUL inside the header section --- */
    char nul_buf[] = "GET / HTTP/1.1\r\nHost: exam_le.com\r\n\r\n";
    size_t nul_len = sizeof nul_buf - 1;
    nul_buf[26] = '\0'; /* clobber a byte of the host value */
    CHECK_FALSE(http_parse_request(nul_buf, nul_len, &req));

    /* --- torture: every truncation of a valid request must be rejected,
     *     because the CRLFCRLF terminator is no longer inside len --- */
    const char *full =
        "GET /a HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "Accept: */*\r\n"
        "\r\n";
    size_t full_len = strlen(full);
    for (size_t i = 0; i < full_len; i++) {
        CHECK_FALSE(http_parse_request(full, i, &req));
    }
    CHECK_TRUE(http_parse_request(full, full_len, &req));

    /* --- torture: flip each byte of the request line to '\r' and make sure
     *     we never crash or read out of bounds (result may be true or false,
     *     the sanitizers are the real judge here) --- */
    char fuzz[128];
    for (size_t i = 0; i < 17; i++) { /* request line + its CRLF */
        memcpy(fuzz, full, full_len);
        fuzz[i] = '\r';
        (void)http_parse_request(fuzz, full_len, &req);
    }
    CHECK_TRUE(1); /* survived the fuzz loop */

    /* --- too many headers --- */
    char many[4096];
    int off = snprintf(many, sizeof many, "GET / HTTP/1.1\r\n");
    for (int i = 0; i < HTTP_MAX_HEADERS + 1; i++) {
        off += snprintf(many + off, sizeof many - (size_t)off,
                        "H%d: v\r\n", i);
    }
    off += snprintf(many + off, sizeof many - (size_t)off, "\r\n");
    CHECK_FALSE(http_parse_request(many, (size_t)off, &req));

    /* exactly HTTP_MAX_HEADERS is fine */
    off = snprintf(many, sizeof many, "GET / HTTP/1.1\r\n");
    for (int i = 0; i < HTTP_MAX_HEADERS; i++) {
        off += snprintf(many + off, sizeof many - (size_t)off,
                        "H%d: v\r\n", i);
    }
    off += snprintf(many + off, sizeof many - (size_t)off, "\r\n");
    CHECK_TRUE(http_parse_request(many, (size_t)off, &req));
    CHECK_INT_EQ(req.nheaders, HTTP_MAX_HEADERS);
    CHECK_STR_EQ(http_get_header(&req, "h31"), "v");

    CTEST_END();
}
