/**
 * @file network_request_clean.c
 * @brief Clean NetworkRequest implementation with SSL support
 */

#include <trampolines/network.h>
#include <trampolines/string.h>
#include <trampolines/json.h>
#include <trampoline.h>
#include "network_common.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ======================================================================== */
/* Private Structures                                                       */
/* ======================================================================== */

typedef struct RequestHeader {
    char* key;
    char* value;
    struct RequestHeader* next;
} RequestHeader;

typedef struct NetworkRequestPrivate {
    NetworkRequest public;  /* Public interface MUST be first */
    
    /* Request properties */
    char* url;
    HttpMethod method;
    RequestHeader* headers;
    char* body;
    size_t body_length;
    
    /* Connection settings */
    int timeout_seconds;
    bool follow_redirects;
    int max_redirects;
    
    /* Parsed URL components */
    char* scheme;
    char* host;
    int port;
    char* path;
    char* query;
} NetworkRequestPrivate;

/* ======================================================================== */
/* Helper Functions                                                          */
/* ======================================================================== */

static const char* method_to_string(HttpMethod method) {
    static const char* methods[] = {
        "GET", "POST", "PUT", "DELETE", "PATCH", "HEAD", "OPTIONS"
    };
    
    if (method >= 0 && method < sizeof(methods)/sizeof(methods[0])) {
        return methods[method];
    }
    return "GET";
}

static void free_headers(RequestHeader* headers) {
    while (headers) {
        RequestHeader* next = headers->next;
        free(headers->key);
        free(headers->value);
        free(headers);
        headers = next;
    }
}

static bool parse_url_clean(const char* url, NetworkRequestPrivate* private) {
    if (!url || !private) return false;
    
    /* Clean up any existing URL components */
    free(private->scheme);
    free(private->host);
    free(private->path);
    free(private->query);
    
    private->scheme = NULL;
    private->host = NULL;
    private->path = NULL;
    private->query = NULL;
    
    /* Make a working copy */
    char* work = strdup(url);
    if (!work) return false;
    
    char* ptr = work;
    
    /* Parse scheme */
    char* scheme_end = strstr(ptr, "://");
    if (!scheme_end) {
        free(work);
        return false;
    }
    
    *scheme_end = '\0';
    private->scheme = strdup(ptr);
    ptr = scheme_end + 3;
    
    /* Default port based on scheme */
    if (strcmp(private->scheme, "https") == 0) {
        private->port = 443;
    } else if (strcmp(private->scheme, "http") == 0) {
        private->port = 80;
    } else {
        private->port = 80;
    }
    
    /* Parse host and port */
    char* path_start = strchr(ptr, '/');
    char* port_start = strchr(ptr, ':');
    
    if (port_start && (!path_start || port_start < path_start)) {
        /* Host with port */
        *port_start = '\0';
        private->host = strdup(ptr);
        
        ptr = port_start + 1;
        if (path_start) {
            *path_start = '\0';
            private->port = atoi(ptr);
            ptr = path_start + 1;
            *path_start = '/';
        } else {
            private->port = atoi(ptr);
            ptr = NULL;
        }
    } else {
        /* Host without port */
        if (path_start) {
            *path_start = '\0';
            private->host = strdup(ptr);
            ptr = path_start;
            *path_start = '/';
        } else {
            private->host = strdup(ptr);
            ptr = NULL;
        }
    }
    
    /* Parse path and query */
    if (ptr) {
        char* query_start = strchr(ptr, '?');
        if (query_start) {
            *query_start = '\0';
            private->path = strdup(ptr);
            private->query = strdup(query_start + 1);
        } else {
            private->path = strdup(ptr);
        }
    } else {
        private->path = strdup("/");
    }
    
    free(work);
    return true;
}

static RequestHeader* find_header(RequestHeader* headers, const char* key) {
    while (headers) {
        if (strcasecmp(headers->key, key) == 0) {
            return headers;
        }
        headers = headers->next;
    }
    return NULL;
}

static void add_or_update_header(NetworkRequestPrivate* private, 
                                 const char* key, const char* value) {
    RequestHeader* existing = find_header(private->headers, key);
    
    if (existing) {
        /* Update existing header */
        free(existing->value);
        existing->value = strdup(value);
    } else {
        /* Add new header */
        RequestHeader* new_header = calloc(1, sizeof(RequestHeader));
        if (new_header) {
            new_header->key = strdup(key);
            new_header->value = strdup(value);
            
            /* Add to front of list */
            new_header->next = private->headers;
            private->headers = new_header;
        }
    }
}

static char* build_header_string(RequestHeader* headers) {
    /* Calculate total size needed */
    size_t total_size = 0;
    RequestHeader* h = headers;
    while (h) {
        total_size += strlen(h->key) + strlen(h->value) + 4; /* ": \r\n" */
        h = h->next;
    }
    
    if (total_size == 0) return NULL;
    
    char* result = malloc(total_size + 1);
    if (!result) return NULL;
    
    char* ptr = result;
    h = headers;
    while (h) {
        ptr += sprintf(ptr, "%s: %s\r\n", h->key, h->value);
        h = h->next;
    }
    
    return result;
}

/* ======================================================================== */
/* Trampoline Functions using TF_ macros                                    */
/* ======================================================================== */

static TF_Getter(networkrequest_url, NetworkRequest, NetworkRequestPrivate, const char*)
    return private->url;
}

static TF_Setter(networkrequest_setUrl, NetworkRequest, NetworkRequestPrivate, const char*)
    free(private->url);
    private->url = newValue ? strdup(newValue) : NULL;
    
    /* Re-parse URL */
    if (private->url) {
        parse_url_clean(private->url, private);
    }
}

static TF_Getter(networkrequest_method, NetworkRequest, NetworkRequestPrivate, HttpMethod)
    return private->method;
}

static TF_Setter(networkrequest_setMethod, NetworkRequest, NetworkRequestPrivate, HttpMethod)
    private->method = newValue;
}

static TF_Getter(networkrequest_body, NetworkRequest, NetworkRequestPrivate, const char*)
    return private->body;
}

static TF_Setter(networkrequest_setBody, NetworkRequest, NetworkRequestPrivate, const char*)
    free(private->body);
    if (newValue) {
        private->body_length = strlen(newValue);
        private->body = malloc(private->body_length + 1);
        if (private->body) {
            strcpy(private->body, newValue);
        }
    } else {
        private->body = NULL;
        private->body_length = 0;
    }
}

static TF_Getter(networkrequest_bodyLength, NetworkRequest, NetworkRequestPrivate, size_t)
    return private->body_length;
}

static TF_Unary(void, networkrequest_setBodyString, NetworkRequest, NetworkRequestPrivate, String*, str)
    (void)private; /* Suppress unused warning */
    if (str && str->cStr) {
        networkrequest_setBody(self, str->cStr());
    } else {
        networkrequest_setBody(self, NULL);
    }
}

static TF_Unary(void, networkrequest_setBodyJson, NetworkRequest, NetworkRequestPrivate, Json*, json)
    char* json_str;
    
    (void)private; /* Suppress unused warning */
    
    if (json) {
        json_str = json->stringify();
        if (json_str) {
            networkrequest_setBody(self, json_str);
            /* Also set Content-Type header for JSON */
            self->setHeader("Content-Type", "application/json");
            free(json_str);
        }
    } else {
        networkrequest_setBody(self, NULL);
    }
}

static TF_Getter(networkrequest_port, NetworkRequest, NetworkRequestPrivate, int)
    return private->port;
}

static TF_Setter(networkrequest_setPort, NetworkRequest, NetworkRequestPrivate, int)
    private->port = newValue;
}

static TF_Getter(networkrequest_timeout, NetworkRequest, NetworkRequestPrivate, int)
    return private->timeout_seconds;
}

static TF_Setter(networkrequest_setTimeout, NetworkRequest, NetworkRequestPrivate, int)
    private->timeout_seconds = newValue;
}

static TF_Unary(const char*, networkrequest_header, NetworkRequest, NetworkRequestPrivate, const char*, key)
    RequestHeader* header = find_header(private->headers, key);
    return header ? header->value : NULL;
}

static TF_Dyadic(void, networkrequest_setHeader, NetworkRequest, NetworkRequestPrivate,
                const char*, key, const char*, value)
    if (key && value) {
        add_or_update_header(private, key, value);
    }
}

static TF_Unary(void, networkrequest_removeHeader, NetworkRequest, NetworkRequestPrivate, const char*, key)
    RequestHeader* prev = NULL;
    RequestHeader* current = private->headers;
    
    while (current) {
        if (strcasecmp(current->key, key) == 0) {
            if (prev) {
                prev->next = current->next;
            } else {
                private->headers = current->next;
            }
            free(current->key);
            free(current->value);
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}

/* Forward declaration */
NetworkResponse* NetworkResponseMake(int status_code, const char* status_text, const char* body);

static TF_Getter(networkrequest_send, NetworkRequest, NetworkRequestPrivate, NetworkResponse*)
    bool use_ssl;
    Connection* conn;
    NetworkResponse* error_resp;
    String* full_path;
    char* header_string;
    char* request;
    ssize_t sent;
    char buffer[65536];
    size_t total_read = 0;
    ssize_t bytes_read;
    
    if (!private->url || !private->host) {
        return NetworkResponseMake(400, "Bad Request", "Invalid URL");
    }
    
    /* Determine if we need SSL */
    use_ssl = (strcmp(private->scheme, "https") == 0);
    
    /* Create connection */
    conn = connection_create(private->host, private->port, use_ssl);
    if (!conn) {
        return NetworkResponseMake(500, "Internal Server Error", 
                                  "Failed to create connection");
    }
    
    /* Set timeout */
    conn->timeout_seconds = private->timeout_seconds;
    
    /* Connect to server */
    if (!connection_connect(conn)) {
        error_resp = NetworkResponseMake(502, "Bad Gateway",
                                         connection_error(conn));
        connection_free(conn);
        return error_resp;
    }
    
    /* Build path with query */
    full_path = StringMake(private->path ? private->path : "/");
    if (private->query) {
        full_path->append("?");
        full_path->append(private->query);
    }
    
    /* Build headers string */
    header_string = build_header_string(private->headers);
    
    /* Build HTTP request */
    request = http_build_request(
        method_to_string(private->method),
        full_path->cStr(),
        private->host,
        header_string,
        private->body,
        private->body_length
    );
    
    full_path->free();
    free(header_string);
    
    if (!request) {
        connection_free(conn);
        return NetworkResponseMake(500, "Internal Server Error",
                                  "Failed to build request");
    }
    
    /* Send request */
    sent = connection_send(conn, request, strlen(request));
    free(request);
    
    if (sent < 0) {
        error_resp = NetworkResponseMake(500, "Internal Server Error",
                                         connection_error(conn));
        connection_free(conn);
        return error_resp;
    }
    
    /* Read response */
    
    while (total_read < sizeof(buffer) - 1) {
        bytes_read = connection_recv(conn, buffer + total_read, 
                                     sizeof(buffer) - total_read - 1);
        if (bytes_read <= 0) break;
        total_read += bytes_read;
    }
    buffer[total_read] = '\0';
    
    connection_free(conn);
    
    /* Create response from raw data */
    return NetworkResponseMake(200, "OK", buffer);
}

static TF_Nullary(networkrequest_free, NetworkRequest, NetworkRequestPrivate)
    if (private) {
        free(private->url);
        free(private->body);
        free(private->scheme);
        free(private->host);
        free(private->path);
        free(private->query);
        free_headers(private->headers);
        trampoline_tracker_free_by_context(self);
        free(private);
    }
}

/* ======================================================================== */
/* Creation Functions                                                        */
/* ======================================================================== */

NetworkRequest* NetworkRequestMake(const char* url, HttpMethod method) {
    TA_Allocate(NetworkRequest, NetworkRequestPrivate);
    
    if (!private) return NULL;
    
    /* Initialize fields */
    private->method = method;
    private->timeout_seconds = 30;
    private->follow_redirects = true;
    private->max_redirects = 5;
    
    /* Parse and set URL */
    if (url) {
        private->url = strdup(url);
        if (!parse_url_clean(url, private)) {
            free(private->url);
            free(private);
            return NULL;
        }
    }
    
    /* Add default headers */
    add_or_update_header(private, "User-Agent", "TrampolineHTTP/2.0");
    add_or_update_header(private, "Accept", "*/*");
    
    /* Create trampoline functions */
    public->url = trampoline_monitor(networkrequest_url, public, 0, &tracker);
    public->setUrl = trampoline_monitor(networkrequest_setUrl, public, 1, &tracker);
    public->method = trampoline_monitor(networkrequest_method, public, 0, &tracker);
    public->setMethod = trampoline_monitor(networkrequest_setMethod, public, 1, &tracker);
    
    public->header = trampoline_monitor(networkrequest_header, public, 1, &tracker);
    public->setHeader = trampoline_monitor(networkrequest_setHeader, public, 2, &tracker);
    public->removeHeader = trampoline_monitor(networkrequest_removeHeader, public, 1, &tracker);
    
    public->body = trampoline_monitor(networkrequest_body, public, 0, &tracker);
    public->setBody = trampoline_monitor(networkrequest_setBody, public, 1, &tracker);
    public->bodyLength = trampoline_monitor(networkrequest_bodyLength, public, 0, &tracker);
    public->setBodyString = trampoline_monitor(networkrequest_setBodyString, public, 1, &tracker);
    public->setBodyJson = trampoline_monitor(networkrequest_setBodyJson, public, 1, &tracker);
    
    public->port = trampoline_monitor(networkrequest_port, public, 0, &tracker);
    public->setPort = trampoline_monitor(networkrequest_setPort, public, 1, &tracker);
    public->timeout = trampoline_monitor(networkrequest_timeout, public, 0, &tracker);
    public->setTimeout = trampoline_monitor(networkrequest_setTimeout, public, 1, &tracker);
    
    public->send = trampoline_monitor(networkrequest_send, public, 0, &tracker);
    public->free = trampoline_monitor(networkrequest_free, public, 0, &tracker);
    
    /* Validate all trampolines */
    if (!trampoline_validate(tracker)) {
        free(private->url);
        free(private->scheme);
        free(private->host);
        free(private->path);
        free(private->query);
        free_headers(private->headers);
        free(private);
        return NULL;
    }
    
    return public;
}

NetworkRequest* NetworkRequestMakeWithString(String* url, HttpMethod method) {
    return NetworkRequestMake(url ? url->cStr() : NULL, method);
}