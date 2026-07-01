#include "httparse.h"
#include "todo.h"

bool http_parse_request(const char *buf, size_t len, HttpRequest *out) {
    (void)buf;
    (void)len;
    (void)out;
    TODO("implement http_parse_request");
}

const char *http_get_header(const HttpRequest *req, const char *name) {
    (void)req;
    (void)name;
    TODO("implement http_get_header");
}
