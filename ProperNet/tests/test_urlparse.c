#include "ctest.h"
#include "urlparse.h"

int main(void) {
    UrlParts u;

    /* full URL */
    SECTION("url_parse full URL");
    CHECK_TRUE(url_parse("http://example.com:8080/a/b.html", &u));
    CHECK_STR_EQ(u.scheme, "http");
    CHECK_STR_EQ(u.host, "example.com");
    CHECK_INT_EQ(u.port, 8080);
    CHECK_STR_EQ(u.path, "/a/b.html");

    /* default port 80 for http */
    SECTION("url_parse default http port");
    CHECK_TRUE(url_parse("http://example.com/index.html", &u));
    CHECK_STR_EQ(u.scheme, "http");
    CHECK_STR_EQ(u.host, "example.com");
    CHECK_INT_EQ(u.port, 80);
    CHECK_STR_EQ(u.path, "/index.html");

    /* default port 443 for https, default path "/" */
    SECTION("url_parse default https port");
    CHECK_TRUE(url_parse("https://example.com", &u));
    CHECK_STR_EQ(u.scheme, "https");
    CHECK_STR_EQ(u.host, "example.com");
    CHECK_INT_EQ(u.port, 443);
    CHECK_STR_EQ(u.path, "/");

    /* explicit port, default path */
    SECTION("url_parse explicit port");
    CHECK_TRUE(url_parse("https://example.com:8443", &u));
    CHECK_INT_EQ(u.port, 8443);
    CHECK_STR_EQ(u.path, "/");

    /* trailing slash is a real path */
    SECTION("url_parse trailing slash");
    CHECK_TRUE(url_parse("http://example.com/", &u));
    CHECK_STR_EQ(u.path, "/");

    /* IPv4 host */
    SECTION("url_parse IPv4 host");
    CHECK_TRUE(url_parse("http://127.0.0.1:3000/status", &u));
    CHECK_STR_EQ(u.host, "127.0.0.1");
    CHECK_INT_EQ(u.port, 3000);
    CHECK_STR_EQ(u.path, "/status");

    /* query string stays part of the path */
    SECTION("url_parse query string");
    CHECK_TRUE(url_parse("http://example.com/search?q=hi&n=2", &u));
    CHECK_STR_EQ(u.path, "/search?q=hi&n=2");

    /* max valid port */
    SECTION("url_parse max port");
    CHECK_TRUE(url_parse("http://h:65535/", &u));
    CHECK_INT_EQ(u.port, 65535);

    /* --- malformed inputs --- */

    /* missing scheme */
    SECTION("url_parse missing scheme");
    CHECK_FALSE(url_parse("example.com/path", &u));
    CHECK_FALSE(url_parse("//example.com/path", &u));
    CHECK_FALSE(url_parse("", &u));

    /* unsupported scheme */
    SECTION("url_parse unsupported scheme");
    CHECK_FALSE(url_parse("ftp://example.com/", &u));
    CHECK_FALSE(url_parse("httpx://example.com/", &u));

    /* empty host */
    SECTION("url_parse empty host");
    CHECK_FALSE(url_parse("http://", &u));
    CHECK_FALSE(url_parse("http:///path", &u));
    CHECK_FALSE(url_parse("http://:80/path", &u));

    /* bad ports */
    SECTION("url_parse bad ports");
    CHECK_FALSE(url_parse("http://example.com:", &u));
    CHECK_FALSE(url_parse("http://example.com:/x", &u));
    CHECK_FALSE(url_parse("http://example.com:abc/", &u));
    CHECK_FALSE(url_parse("http://example.com:80a/", &u));
    CHECK_FALSE(url_parse("http://example.com:65536/", &u));
    CHECK_FALSE(url_parse("http://example.com:99999", &u));

    CTEST_END();
}
