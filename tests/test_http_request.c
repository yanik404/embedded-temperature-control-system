#include <assert.h>
#include <stdio.h>

#include "http_request.h"

static void test_standard_android_probe(void) {
    const char text[] = "GET /generate_204 HTTP/1.0\r\n"
                        "hOsT: ConnectivityCheck.GStatic.com\r\n\r\n";
    http_request_t request;
    assert(http_request_parse(text, &request));
    assert(request.method == HTTP_METHOD_GET);
    assert(http_request_path_equals(&request, "/generate_204"));
    assert(http_request_host_equals(&request, "connectivitycheck.gstatic.com"));
    assert(http_request_is_android_probe(&request));
}

static void test_absolute_android_url(void) {
    const char text[] = "GET http://clients3.google.com/gen_204?device=android HTTP/1.1\r\n"
                        "host: ignored.example\r\n\r\n";
    http_request_t request;
    assert(http_request_parse(text, &request));
    assert(http_request_path_equals(&request, "/gen_204"));
    assert(http_request_host_equals(&request, "clients3.google.com"));
    assert(http_request_is_android_probe(&request));
}

static void test_oem_probe_paths(void) {
    http_request_t request;
    assert(http_request_parse("HEAD /connectivitycheck.gstatic.com HTTP/1.1\r\n\r\n", &request));
    assert(http_request_is_android_probe(&request));
    assert(http_request_parse("GET /clients3.google.com HTTP/1.1\r\n\r\n", &request));
    assert(http_request_is_android_probe(&request));
}

static void test_dashboard_and_api_are_not_probes(void) {
    http_request_t request;
    assert(http_request_parse("GET / HTTP/1.1\r\nHost: 192.168.4.1:80\r\n\r\n", &request));
    assert(http_request_path_equals(&request, "/"));
    assert(http_request_host_equals(&request, "192.168.4.1"));
    assert(!http_request_is_android_probe(&request));
    assert(http_request_parse("POST /api/start HTTP/1.1\r\nHost: 192.168.4.1\r\n\r\n", &request));
    assert(request.method == HTTP_METHOD_POST);
    assert(http_request_path_equals(&request, "/api/start"));
    assert(!http_request_is_android_probe(&request));
}

int main(void) {
    test_standard_android_probe();
    test_absolute_android_url();
    test_oem_probe_paths();
    test_dashboard_and_api_are_not_probes();
    puts("HTTP request tests passed");
    return 0;
}
