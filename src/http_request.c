#include "http_request.h"

#include <stddef.h>
#include <string.h>

static char ascii_lower(char value) {
    return value >= 'A' && value <= 'Z' ? (char)(value + ('a' - 'A')) : value;
}

static bool starts_with_ignore_case(const char *text, const char *prefix) {
    while (*prefix != '\0') {
        if (ascii_lower(*text++) != ascii_lower(*prefix++)) return false;
    }
    return true;
}

static bool equals_ignore_case(const char *left, const char *right) {
    while (*left != '\0' && *right != '\0') {
        if (ascii_lower(*left++) != ascii_lower(*right++)) return false;
    }
    return *left == '\0' && *right == '\0';
}

static bool copy_token(char *destination, size_t capacity,
                       const char *begin, const char *end, bool lower_case) {
    const size_t length = (size_t)(end - begin);
    if (length == 0u || length >= capacity) return false;
    for (size_t i = 0; i < length; ++i) {
        destination[i] = lower_case ? ascii_lower(begin[i]) : begin[i];
    }
    destination[length] = '\0';
    return true;
}

static const char *line_end(const char *line) {
    const char *end = strchr(line, '\n');
    return end != NULL && end > line && end[-1] == '\r' ? end - 1 : end;
}

static bool parse_absolute_target(const char *begin, const char *end,
                                  http_request_t *request) {
    const char *authority;
    if ((size_t)(end - begin) >= 7u && starts_with_ignore_case(begin, "http://")) {
        authority = begin + 7u;
    } else if ((size_t)(end - begin) >= 8u && starts_with_ignore_case(begin, "https://")) {
        authority = begin + 8u;
    } else {
        return false;
    }

    const char *path = authority;
    while (path < end && *path != '/' && *path != '?' && *path != '#') ++path;
    const char *host_end = authority;
    while (host_end < path && *host_end != ':') ++host_end;
    if (!copy_token(request->host, sizeof(request->host), authority, host_end, true)) return false;

    if (path == end) {
        strcpy(request->path, "/");
        return true;
    }
    if (*path == '/') return copy_token(request->path, sizeof(request->path), path, end, false);

    request->path[0] = '/';
    return copy_token(request->path + 1u, sizeof(request->path) - 1u, path, end, false);
}

static void parse_host_header(const char *headers, http_request_t *request) {
    while (headers != NULL && *headers != '\0') {
        const char *end = line_end(headers);
        if (end == NULL) return;
        const char *colon = headers;
        while (colon < end && *colon != ':') ++colon;
        if (colon - headers == 4 && starts_with_ignore_case(headers, "host")) {
            const char *value = colon + 1;
            while (value < end && (*value == ' ' || *value == '\t')) ++value;
            const char *value_end = value;
            while (value_end < end && *value_end != ':' && *value_end != ' ' && *value_end != '\t') {
                ++value_end;
            }
            (void)copy_token(request->host, sizeof(request->host), value, value_end, true);
            return;
        }
        headers = *end == '\r' ? end + 2 : end + 1;
    }
}

bool http_request_parse(const char *text, http_request_t *request) {
    if (text == NULL || request == NULL) return false;
    memset(request, 0, sizeof(*request));

    const char *method_end = strchr(text, ' ');
    if (method_end == NULL) return false;
    if (method_end - text == 3 && memcmp(text, "GET", 3u) == 0) request->method = HTTP_METHOD_GET;
    else if (method_end - text == 4 && memcmp(text, "HEAD", 4u) == 0) request->method = HTTP_METHOD_HEAD;
    else if (method_end - text == 4 && memcmp(text, "POST", 4u) == 0) request->method = HTTP_METHOD_POST;
    else return false;

    const char *target = method_end + 1;
    const char *target_end = strchr(target, ' ');
    if (target_end == NULL || target_end == target) return false;
    const bool absolute = parse_absolute_target(target, target_end, request);
    if (!absolute && !copy_token(request->path, sizeof(request->path),
                                 target, target_end, false)) return false;

    const char *headers = strchr(target_end, '\n');
    if (!absolute && headers != NULL) parse_host_header(headers + 1, request);
    return request->path[0] == '/';
}

bool http_request_path_equals(const http_request_t *request, const char *path) {
    if (request == NULL || path == NULL) return false;
    const size_t length = strlen(path);
    return strncmp(request->path, path, length) == 0 &&
           (request->path[length] == '\0' || request->path[length] == '?' ||
            request->path[length] == '#');
}

bool http_request_host_equals(const http_request_t *request, const char *host) {
    return request != NULL && host != NULL && equals_ignore_case(request->host, host);
}

bool http_request_is_android_probe(const http_request_t *request) {
    if (request == NULL || (request->method != HTTP_METHOD_GET &&
                            request->method != HTTP_METHOD_HEAD)) return false;
    const bool probe_path = http_request_path_equals(request, "/generate_204") ||
                            http_request_path_equals(request, "/gen_204") ||
                            http_request_path_equals(request, "/connectivitycheck.gstatic.com") ||
                            http_request_path_equals(request, "/clients3.google.com");
    const bool probe_host = http_request_host_equals(request, "connectivitycheck.gstatic.com") ||
                            http_request_host_equals(request, "connectivitycheck.gstatic.cn") ||
                            http_request_host_equals(request, "clients3.google.com") ||
                            http_request_host_equals(request, "www.google.com") ||
                            http_request_host_equals(request, "play.googleapis.com") ||
                            http_request_host_equals(request, "connectivitycheck.android.com") ||
                            http_request_host_equals(request, "connect.rom.miui.com") ||
                            http_request_host_equals(request, "connectivitycheck.platform.hicloud.com") ||
                            http_request_host_equals(request, "wifi.vivo.com.cn") ||
                            http_request_host_equals(request, "conn1.oppomobile.com");
    return probe_path || probe_host;
}
