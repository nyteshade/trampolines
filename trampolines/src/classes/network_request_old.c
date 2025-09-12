/**
 * @file network_request.c
 * @brief NetworkRequest implementation using new trampoline macros
 */

#include <trampolines/network.h>
#include <trampolines/string.h>
#include <trampoline.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/select.h>
#include <ctype.h>

/* ======================================================================== */
/* Private Structures                                                       */
/* ======================================================================== */

typedef struct HeaderNode {
    char* key;
    char* value;
    struct HeaderNode* next;
} HeaderNode;

typedef struct NetworkRequestPrivate {
    NetworkRequest public;  /* Public interface MUST be first */
    
    char* url;
    HttpMethod method;
    HeaderNode* headers;
    char* body;
    size_t body_length;
    int port;
    int timeout_seconds;
} NetworkRequestPrivate;

/* ======================================================================== */
/* URL Parsing (global utility function)                                    */
/* ======================================================================== */

int parse_url_t(char* url_string, url_t* parsed_url) {
    if (!url_string || !parsed_url) return -1;
    
    memset(parsed_url, 0, sizeof(url_t));
    
    /* Parse scheme */
    char* scheme_end = strstr(url_string, "://");
    if (!scheme_end) return -1;
    
    *scheme_end = '\0';
    parsed_url->scheme = url_string;
    
    /* Parse host and port */
    char* host_start = scheme_end + 3;
    char* port_start = strchr(host_start, ':');
    char* path_start = strchr(host_start, '/');
    
    if (port_start && (!path_start || port_start < path_start)) {
        *port_start = '\0';
        parsed_url->host = host_start;
        
        char* port_end = path_start ? path_start : port_start + strlen(port_start + 1) + 1;
        if (path_start) *path_start = '\0';
        parsed_url->port = port_start + 1;
        
        if (path_start) {
            *path_start = '/';
            parsed_url->path = path_start;
        }
    } else {
        if (path_start) *path_start = '\0';
        parsed_url->host = host_start;
        if (path_start) {
            *path_start = '/';
            parsed_url->path = path_start;
        }
    }
    
    /* Parse query and fragment */
    if (parsed_url->path) {
        char* query_start = strchr(parsed_url->path, '?');
        if (query_start) {
            *query_start = '\0';
            parsed_url->query = query_start + 1;
            
            char* fragment_start = strchr(parsed_url->query, '#');
            if (fragment_start) {
                *fragment_start = '\0';
                parsed_url->fragment = fragment_start + 1;
            }
        } else {
            char* fragment_start = strchr(parsed_url->path, '#');
            if (fragment_start) {
                *fragment_start = '\0';
                parsed_url->fragment = fragment_start + 1;
            }
        }
    }
    
    return 0;
}

/* ======================================================================== */
/* Helper Functions                                                          */
/* ======================================================================== */

static const char* http_method_string(HttpMethod method) {
    switch (method) {
        case HTTP_GET: return "GET";
        case HTTP_POST: return "POST";
        case HTTP_PUT: return "PUT";
        case HTTP_DELETE: return "DELETE";
        case HTTP_PATCH: return "PATCH";
        case HTTP_HEAD: return "HEAD";
        case HTTP_OPTIONS: return "OPTIONS";
        default: return "GET";
    }
}

static void free_headers(HeaderNode* headers) {
    while (headers) {
        HeaderNode* next = headers->next;
        if (headers->key) free(headers->key);
        if (headers->value) free(headers->value);
        free(headers);
        headers = next;
    }
}

/* ======================================================================== */
/* Trampoline Functions using TF_ macros                                    */
/* ======================================================================== */

static TF_Getter(networkrequest_url, NetworkRequest, NetworkRequestPrivate, const char*)
    return private->url;
}

static TF_Setter(networkrequest_setUrl, NetworkRequest, NetworkRequestPrivate, const char*)
    if (private->url) free(private->url);
    if (newValue) {
        private->url = malloc(strlen(newValue) + 1);
        if (private->url) strcpy(private->url, newValue);
    } else {
        private->url = NULL;
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
    if (private->body) free(private->body);
    if (newValue) {
        size_t len = strlen(newValue);
        private->body = malloc(len + 1);
        if (private->body) {
            strcpy(private->body, newValue);
            private->body_length = len;
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
    if (str) {
        networkrequest_setBody(self, str->cStr());
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
    HeaderNode* current = private->headers;
    
    while (current) {
        if (strcasecmp(current->key, key) == 0) {
            return current->value;
        }
        current = current->next;
    }
    
    return NULL;
}

static TF_Dyadic(void, networkrequest_setHeader, NetworkRequest, NetworkRequestPrivate, 
                 const char*, key, const char*, value)
    HeaderNode* current = private->headers;
    HeaderNode* prev = NULL;
    
    /* Update existing header */
    while (current) {
        if (strcasecmp(current->key, key) == 0) {
            free(current->value);
            current->value = malloc(strlen(value) + 1);
            if (current->value) {
                strcpy(current->value, value);
            }
            return;
        }
        prev = current;
        current = current->next;
    }
    
    /* Add new header */
    HeaderNode* new_header = calloc(1, sizeof(HeaderNode));
    if (new_header) {
        new_header->key = malloc(strlen(key) + 1);
        new_header->value = malloc(strlen(value) + 1);
        if (new_header->key && new_header->value) {
            strcpy(new_header->key, key);
            strcpy(new_header->value, value);
            
            if (prev) {
                prev->next = new_header;
            } else {
                private->headers = new_header;
            }
        } else {
            if (new_header->key) free(new_header->key);
            if (new_header->value) free(new_header->value);
            free(new_header);
        }
    }
}

static TF_Unary(void, networkrequest_removeHeader, NetworkRequest, NetworkRequestPrivate, const char*, key)
    HeaderNode* current = private->headers;
    HeaderNode* prev = NULL;
    
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

/* Forward declaration for NetworkResponseMake */
NetworkResponse* NetworkResponseMake(int status_code, const char* status_text, const char* body);

static TF_Getter(networkrequest_send, NetworkRequest, NetworkRequestPrivate, NetworkResponse*)
    if (!private->url) return NULL;
    
    /* Parse URL */
    char* url_copy = malloc(strlen(private->url) + 1);
    if (!url_copy) return NULL;
    strcpy(url_copy, private->url);
    
    url_t parsed_url;
    if (parse_url_t(url_copy, &parsed_url) != 0) {
        free(url_copy);
        return NULL;
    }
    
    /* Only support HTTP for now */
    if (strcmp(parsed_url.scheme, "http") != 0) {
        free(url_copy);
        return NetworkResponseMake(501, "Not Implemented", "Only HTTP is supported");
    }
    
    /* Get host info */
    struct hostent* host_info = gethostbyname(parsed_url.host);
    if (!host_info) {
        free(url_copy);
        return NetworkResponseMake(502, "Bad Gateway", "Failed to resolve host");
    }
    
    /* Create socket */
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        free(url_copy);
        return NetworkResponseMake(500, "Internal Server Error", "Failed to create socket");
    }
    
    /* Set timeout */
    struct timeval tv;
    tv.tv_sec = private->timeout_seconds;
    tv.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    
    /* Connect */
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(parsed_url.port ? atoi(parsed_url.port) : private->port);
    memcpy(&server_addr.sin_addr, host_info->h_addr_list[0], host_info->h_length);
    
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        close(sockfd);
        free(url_copy);
        return NetworkResponseMake(502, "Bad Gateway", "Failed to connect to server");
    }
    
    /* Build request using String class */
    String* request = StringMake("");
    if (!request) {
        close(sockfd);
        free(url_copy);
        return NULL;
    }
    
    /* Request line */
    request->append(http_method_string(private->method));
    request->append(" ");
    request->append(parsed_url.path ? parsed_url.path : "/");
    if (parsed_url.query) {
        request->append("?");
        request->append(parsed_url.query);
    }
    request->append(" HTTP/1.1\r\n");
    
    /* Host header */
    request->append("Host: ");
    request->append(parsed_url.host);
    request->append("\r\n");
    
    /* User headers */
    HeaderNode* header = private->headers;
    while (header) {
        request->append(header->key);
        request->append(": ");
        request->append(header->value);
        request->append("\r\n");
        header = header->next;
    }
    
    /* Content-Length for body */
    if (private->body && private->body_length > 0) {
        char content_length[32];
        snprintf(content_length, sizeof(content_length), "Content-Length: %zu\r\n", private->body_length);
        request->append(content_length);
    }
    
    /* End headers */
    request->append("\r\n");
    
    /* Body */
    if (private->body) {
        request->append(private->body);
    }
    
    /* Send request */
    ssize_t sent = send(sockfd, request->cStr(), request->length(), 0);
    request->free();
    
    if (sent < 0) {
        close(sockfd);
        free(url_copy);
        return NetworkResponseMake(500, "Internal Server Error", "Failed to send request");
    }
    
    /* Read response */
    char buffer[65536];  /* 64KB buffer */
    ssize_t total_read = 0;
    ssize_t bytes_read;
    
    while (total_read < sizeof(buffer) - 1) {
        bytes_read = recv(sockfd, buffer + total_read, sizeof(buffer) - total_read - 1, 0);
        if (bytes_read <= 0) break;
        total_read += bytes_read;
    }
    buffer[total_read] = '\0';
    
    close(sockfd);
    free(url_copy);
    
    /* Parse response - simple version for now */
    /* This would normally parse headers, status, etc. */
    /* For now, just return the raw response */
    return NetworkResponseMake(200, "OK", buffer);
}

static TF_Nullary(networkrequest_free, NetworkRequest, NetworkRequestPrivate)
    if (private) {
        if (private->url) free(private->url);
        if (private->body) free(private->body);
        free_headers(private->headers);
        trampoline_tracker_free_by_context(self);
        free(private);
    }
}

/* ======================================================================== */
/* Creation Functions                                                        */
/* ======================================================================== */

NetworkRequest* NetworkRequestMake(const char* url, HttpMethod method) {
    /* Use new TA_Allocate macro */
    TA_Allocate(NetworkRequest, NetworkRequestPrivate);
    
    if (!private) return NULL;
    
    /* Initialize fields */
    private->url = url ? strdup(url) : NULL;
    private->method = method;
    private->headers = NULL;
    private->body = NULL;
    private->body_length = 0;
    private->port = 80;
    private->timeout_seconds = 30;
    
    /* Create trampoline functions using trampoline_monitor */
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
    
    public->port = trampoline_monitor(networkrequest_port, public, 0, &tracker);
    public->setPort = trampoline_monitor(networkrequest_setPort, public, 1, &tracker);
    public->timeout = trampoline_monitor(networkrequest_timeout, public, 0, &tracker);
    public->setTimeout = trampoline_monitor(networkrequest_setTimeout, public, 1, &tracker);
    
    public->send = trampoline_monitor(networkrequest_send, public, 0, &tracker);
    public->free = trampoline_monitor(networkrequest_free, public, 0, &tracker);
    
    /* Validate all trampolines were created successfully */
    if (!trampoline_validate(tracker)) {
        if (private->url) free(private->url);
        free_headers(private->headers);
        free(private);
        return NULL;
    }
    
    return public;
}

NetworkRequest* NetworkRequestMakeWithString(String* url, HttpMethod method) {
    return NetworkRequestMake(url ? url->cStr() : NULL, method);
}