/**
 * @file network_response.c
 * @brief NetworkResponse implementation using new trampoline macros
 */

#include <trampoline/trampoline.h>
#include <trampoline/macros.h>
#include <trampoline/classes/json.h>
#include <trampoline/classes/string.h>
#include <trampoline/classes/network.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

/* ======================================================================== */
/* Private Structures                                                       */
/* ======================================================================== */

typedef struct ResponseHeader {
    char* key;
    char* value;
    struct ResponseHeader* next;
} ResponseHeader;

typedef struct NetworkResponsePrivate {
    NetworkResponse public;  /* Public interface MUST be first */

    int status_code;
    char* status_text;
    ResponseHeader* headers;
    char* body;
    size_t body_length;
    size_t header_count;
} NetworkResponsePrivate;

/* ======================================================================== */
/* Helper Functions                                                          */
/* ======================================================================== */

static void free_response_headers(ResponseHeader* headers) {
    while (headers) {
        ResponseHeader* next = headers->next;
        if (headers->key) free(headers->key);
        if (headers->value) free(headers->value);
        free(headers);
        headers = next;
    }
}

static void add_response_header(NetworkResponsePrivate* private, const char* key, const char* value) {
    ResponseHeader* new_header = calloc(1, sizeof(ResponseHeader));
    if (!new_header) return;

    new_header->key = strdup(key);
    new_header->value = strdup(value);

    if (!new_header->key || !new_header->value) {
        if (new_header->key) free(new_header->key);
        if (new_header->value) free(new_header->value);
        free(new_header);
        return;
    }

    /* Add to end of list */
    if (!private->headers) {
        private->headers = new_header;
    } else {
        ResponseHeader* current = private->headers;
        while (current->next) {
            current = current->next;
        }
        current->next = new_header;
    }
    private->header_count++;
}

/* ======================================================================== */
/* Trampoline Functions using TF_ macros                                    */
/* ======================================================================== */

static TF_Getter(networkresponse_statusCode, NetworkResponse, NetworkResponsePrivate, int)
    return private->status_code;
}

static TF_Getter(networkresponse_statusText, NetworkResponse, NetworkResponsePrivate, const char*)
    return private->status_text;
}

static TF_Getter(networkresponse_isSuccess, NetworkResponse, NetworkResponsePrivate, int)
    return private->status_code >= 200 && private->status_code < 300;
}

static TF_Getter(networkresponse_isRedirect, NetworkResponse, NetworkResponsePrivate, int)
    return private->status_code >= 300 && private->status_code < 400;
}

static TF_Getter(networkresponse_isError, NetworkResponse, NetworkResponsePrivate, int)
    return private->status_code >= 400;
}

static TF_Unary(const char*, networkresponse_header, NetworkResponse, NetworkResponsePrivate, const char*, key)
    ResponseHeader* current = private->headers;

    while (current) {
        if (strcasecmp(current->key, key) == 0) {
            return current->value;
        }
        current = current->next;
    }

    return NULL;
}

static TF_Getter(networkresponse_headerCount, NetworkResponse, NetworkResponsePrivate, size_t)
    return private->header_count;
}

/* Removed headerKey - not in simplified API */
#if 0
static TF_Dyadic(const char*, networkresponse_headerKey, NetworkResponse, NetworkResponsePrivate,
                 size_t, index, size_t*, value_index)
    if (index >= private->header_count) return NULL;

    ResponseHeader* current = private->headers;
    size_t i = 0;

    while (current && i < index) {
        current = current->next;
        i++;
    }

    if (current) {
        if (value_index) {
            /* This would normally return the index of the value */
            /* For simplicity, we're not implementing this fully */
            *value_index = index;
        }
        return current->key;
    }

    return NULL;
}
#endif

static TF_Getter(networkresponse_body, NetworkResponse, NetworkResponsePrivate, const char*)
    return private->body;
}

static TF_Getter(networkresponse_bodyLength, NetworkResponse, NetworkResponsePrivate, size_t)
    return private->body_length;
}

static TF_Getter(networkresponse_bodyAsString, NetworkResponse, NetworkResponsePrivate, String*)
    if (private->body) {
        return StringMake(private->body);
    }
    return StringMake("");
}

static TF_Getter(networkresponse_bodyAsJson, NetworkResponse, NetworkResponsePrivate, Json*)
    if (!private->body || private->body_length == 0) {
        return NULL;
    }
    return JsonParse(private->body);
}

static TF_Getter(networkresponse_isJson, NetworkResponse, NetworkResponsePrivate, int)
    const char* content_type = networkresponse_header(self, "Content-Type");
    if (!content_type) {
        return 0;
    }
    /* Check if content type contains "json" */
    return strstr(content_type, "json") != NULL;
}

static TF_Getter(networkresponse_contentType, NetworkResponse, NetworkResponsePrivate, const char*)
    (void)private; /* Suppress unused warning */
    return networkresponse_header(self, "Content-Type");
}

static TF_Getter(networkresponse_contentLength, NetworkResponse, NetworkResponsePrivate, size_t)
    const char* length_str = networkresponse_header(self, "Content-Length");
    if (length_str) {
        return (size_t)atol(length_str);
    }
    return private->body_length;
}

static TF_Unary(int, networkresponse_hasHeader, NetworkResponse, NetworkResponsePrivate, const char*, key)
    (void)private; /* Suppress unused warning */
    return networkresponse_header(self, key) != NULL;
}

static TF_Nullary(networkresponse_free, NetworkResponse, NetworkResponsePrivate)
    if (private) {
        if (private->status_text) free(private->status_text);
        if (private->body) free(private->body);
        free_response_headers(private->headers);
        trampoline_tracker_free_by_context(self);
        free(private);
    }
}

/* ======================================================================== */
/* Response Parsing                                                         */
/* ======================================================================== */

static void parse_response(NetworkResponsePrivate* private, const char* raw_response) {
    if (!raw_response) return;

    /* Make a copy to work with */
    char* response = strdup(raw_response);
    if (!response) return;

    char* current = response;

    /* Parse status line: HTTP/1.1 200 OK */
    char* line_end = strstr(current, "\r\n");
    if (line_end) {
        *line_end = '\0';

        /* Skip HTTP version */
        char* space = strchr(current, ' ');
        if (space) {
            space++;
            /* Parse status code */
            private->status_code = atoi(space);

            /* Parse status text */
            space = strchr(space, ' ');
            if (space) {
                space++;
                if (private->status_text) free(private->status_text);
                private->status_text = strdup(space);
            }
        }

        current = line_end + 2;  /* Skip \r\n */
    }

    /* Parse headers */
    while (*current && *current != '\r') {
        line_end = strstr(current, "\r\n");
        if (!line_end) break;

        *line_end = '\0';

        /* Parse header: Key: Value */
        char* colon = strchr(current, ':');
        if (colon) {
            *colon = '\0';
            char* value = colon + 1;

            /* Skip leading whitespace in value */
            while (*value && isspace(*value)) value++;

            add_response_header(private, current, value);
        }

        current = line_end + 2;
    }

    /* Skip empty line between headers and body */
    if (strncmp(current, "\r\n", 2) == 0) {
        current += 2;
    }

    /* Rest is body */
    if (*current) {
        if (private->body) free(private->body);
        private->body = strdup(current);
        private->body_length = strlen(current);
    }

    free(response);
}

/* ======================================================================== */
/* Creation Functions                                                        */
/* ======================================================================== */

NetworkResponse* NetworkResponseMake(int status_code, const char* status_text, const char* body) {
    /* Use new TA_Allocate macro */
    TA_Allocate(NetworkResponse, NetworkResponsePrivate);

    if (!private) return NULL;

    /* Initialize fields */
    private->status_code = status_code;
    private->status_text = status_text ? strdup(status_text) : NULL;
    private->headers = NULL;
    private->header_count = 0;

    /* If body looks like a full HTTP response, parse it */
    if (body && strncmp(body, "HTTP/", 5) == 0) {
        parse_response(private, body);
    } else {
        /* Otherwise just set the body */
        private->body = body ? strdup(body) : NULL;
        private->body_length = body ? strlen(body) : 0;
    }

    /* Create trampoline functions using trampoline_monitor */
    public->statusCode = trampoline_monitor(networkresponse_statusCode, public, 0, &tracker);
    public->statusText = trampoline_monitor(networkresponse_statusText, public, 0, &tracker);
    public->isSuccess = trampoline_monitor(networkresponse_isSuccess, public, 0, &tracker);
    public->isRedirect = trampoline_monitor(networkresponse_isRedirect, public, 0, &tracker);
    public->isError = trampoline_monitor(networkresponse_isError, public, 0, &tracker);

    public->header = trampoline_monitor(networkresponse_header, public, 1, &tracker);
    public->headerCount = trampoline_monitor(networkresponse_headerCount, public, 0, &tracker);
    /* headerKey removed - not in simplified API */

    public->body = trampoline_monitor(networkresponse_body, public, 0, &tracker);
    public->bodyLength = trampoline_monitor(networkresponse_bodyLength, public, 0, &tracker);
    public->bodyAsString = trampoline_monitor(networkresponse_bodyAsString, public, 0, &tracker);
    public->bodyAsJson = trampoline_monitor(networkresponse_bodyAsJson, public, 0, &tracker);

    public->contentType = trampoline_monitor(networkresponse_contentType, public, 0, &tracker);
    public->contentLength = trampoline_monitor(networkresponse_contentLength, public, 0, &tracker);
    public->hasHeader = trampoline_monitor(networkresponse_hasHeader, public, 1, &tracker);
    public->isJson = trampoline_monitor(networkresponse_isJson, public, 0, &tracker);

    public->free = trampoline_monitor(networkresponse_free, public, 0, &tracker);

    /* Validate all trampolines were created successfully */
    if (!trampoline_validate(tracker)) {
        if (private->status_text) free(private->status_text);
        if (private->body) free(private->body);
        free_response_headers(private->headers);
        free(private);
        return NULL;
    }

    return public;
}
