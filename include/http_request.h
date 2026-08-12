#pragma once

#include <stdbool.h>

#define HTTP_REQUEST_PATH_MAX 256u
#define HTTP_REQUEST_HOST_MAX 128u

typedef enum {
    HTTP_METHOD_UNKNOWN = 0,
    HTTP_METHOD_GET,
    HTTP_METHOD_HEAD,
    HTTP_METHOD_POST
} http_method_t;

typedef struct {
    http_method_t method;
    char path[HTTP_REQUEST_PATH_MAX];
    char host[HTTP_REQUEST_HOST_MAX];
} http_request_t;

bool http_request_parse(const char *text, http_request_t *request);
bool http_request_path_equals(const http_request_t *request, const char *path);
bool http_request_host_equals(const http_request_t *request, const char *host);
bool http_request_is_android_probe(const http_request_t *request);
