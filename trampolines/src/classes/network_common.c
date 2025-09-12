/**
 * @file network_common.c
 * @brief Common network utilities implementation
 */

#include "network_common.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>

/* ======================================================================== */
/* SSL Initialization                                                       */
/* ======================================================================== */

#if SSL_SUPPORT
static bool ssl_initialized = false;

void network_init_ssl(void) {
    if (!ssl_initialized) {
        SSL_library_init();
        SSL_load_error_strings();
        OpenSSL_add_all_algorithms();
        ssl_initialized = true;
    }
}

void network_cleanup_ssl(void) {
    if (ssl_initialized) {
        EVP_cleanup();
        ERR_free_strings();
        ssl_initialized = false;
    }
}
#else
void network_init_ssl(void) {
    /* No-op when SSL is disabled */
}

void network_cleanup_ssl(void) {
    /* No-op when SSL is disabled */
}
#endif

/* ======================================================================== */
/* Connection Implementation                                                 */
/* ======================================================================== */

Connection* connection_create(const char* hostname, int port, bool use_ssl) {
    Connection* conn = calloc(1, sizeof(Connection));
    if (!conn) return NULL;
    
    conn->hostname = strdup(hostname);
    conn->port = port;
    conn->timeout_seconds = 30;
    conn->socket_fd = -1;
    
#if SSL_SUPPORT
    if (use_ssl) {
        conn->type = CONN_TYPE_SSL;
        network_init_ssl();
        
        /* Create SSL context */
        const SSL_METHOD* method = SSLv23_client_method();
        conn->ssl_ctx = SSL_CTX_new(method);
        
        if (!conn->ssl_ctx) {
            snprintf(conn->error_buffer, sizeof(conn->error_buffer),
                    "Failed to create SSL context");
            free(conn->hostname);
            free(conn);
            return NULL;
        }
        
        /* Set SSL options for better compatibility */
        SSL_CTX_set_options(conn->ssl_ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);
        SSL_CTX_set_verify(conn->ssl_ctx, SSL_VERIFY_NONE, NULL);
    } else {
        conn->type = CONN_TYPE_PLAIN;
    }
#else
    if (use_ssl) {
        snprintf(conn->error_buffer, sizeof(conn->error_buffer),
                "SSL support not compiled in");
        free(conn->hostname);
        free(conn);
        return NULL;
    }
    conn->type = CONN_TYPE_PLAIN;
#endif
    
    return conn;
}

bool connection_connect(Connection* conn) {
    if (!conn) return false;
    
    /* Resolve hostname */
    struct hostent* host_info = gethostbyname(conn->hostname);
    if (!host_info) {
        snprintf(conn->error_buffer, sizeof(conn->error_buffer),
                "Failed to resolve hostname: %s", conn->hostname);
        return false;
    }
    
    /* Create socket */
    conn->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (conn->socket_fd < 0) {
        snprintf(conn->error_buffer, sizeof(conn->error_buffer),
                "Failed to create socket: %s", strerror(errno));
        return false;
    }
    
    /* Set timeout */
    struct timeval tv;
    tv.tv_sec = conn->timeout_seconds;
    tv.tv_usec = 0;
    setsockopt(conn->socket_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(conn->socket_fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    
    /* Connect to server */
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(conn->port);
    memcpy(&server_addr.sin_addr, host_info->h_addr_list[0], host_info->h_length);
    
    if (connect(conn->socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        snprintf(conn->error_buffer, sizeof(conn->error_buffer),
                "Failed to connect: %s", strerror(errno));
        close(conn->socket_fd);
        conn->socket_fd = -1;
        return false;
    }
    
#if SSL_SUPPORT
    /* Setup SSL if needed */
    if (conn->type == CONN_TYPE_SSL) {
        conn->ssl = SSL_new(conn->ssl_ctx);
        if (!conn->ssl) {
            snprintf(conn->error_buffer, sizeof(conn->error_buffer),
                    "Failed to create SSL structure");
            close(conn->socket_fd);
            conn->socket_fd = -1;
            return false;
        }
        
        SSL_set_fd(conn->ssl, conn->socket_fd);
        
        /* Perform SSL handshake */
        if (SSL_connect(conn->ssl) <= 0) {
            unsigned long err = ERR_get_error();
            char err_buf[256];
            ERR_error_string_n(err, err_buf, sizeof(err_buf));
            snprintf(conn->error_buffer, sizeof(conn->error_buffer),
                    "SSL handshake failed: %s", err_buf);
            SSL_free(conn->ssl);
            conn->ssl = NULL;
            close(conn->socket_fd);
            conn->socket_fd = -1;
            return false;
        }
    }
#endif
    
    return true;
}

ssize_t connection_send(Connection* conn, const void* data, size_t length) {
    if (!conn || conn->socket_fd < 0) return -1;
    
#if SSL_SUPPORT
    if (conn->type == CONN_TYPE_SSL && conn->ssl) {
        int ret = SSL_write(conn->ssl, data, (int)length);
        if (ret <= 0) {
            int ssl_error = SSL_get_error(conn->ssl, ret);
            snprintf(conn->error_buffer, sizeof(conn->error_buffer),
                    "SSL write error: %d", ssl_error);
            return -1;
        }
        return ret;
    }
#endif
    
    return send(conn->socket_fd, data, length, 0);
}

ssize_t connection_recv(Connection* conn, void* buffer, size_t buffer_size) {
    if (!conn || conn->socket_fd < 0) return -1;
    
#if SSL_SUPPORT
    if (conn->type == CONN_TYPE_SSL && conn->ssl) {
        int ret = SSL_read(conn->ssl, buffer, (int)buffer_size);
        if (ret <= 0) {
            int ssl_error = SSL_get_error(conn->ssl, ret);
            if (ssl_error == SSL_ERROR_ZERO_RETURN) {
                /* Clean shutdown */
                return 0;
            }
            snprintf(conn->error_buffer, sizeof(conn->error_buffer),
                    "SSL read error: %d", ssl_error);
            return -1;
        }
        return ret;
    }
#endif
    
    return recv(conn->socket_fd, buffer, buffer_size, 0);
}

void connection_free(Connection* conn) {
    if (!conn) return;
    
#if SSL_SUPPORT
    if (conn->ssl) {
        SSL_shutdown(conn->ssl);
        SSL_free(conn->ssl);
    }
    if (conn->ssl_ctx) {
        SSL_CTX_free(conn->ssl_ctx);
    }
#endif
    
    if (conn->socket_fd >= 0) {
        close(conn->socket_fd);
    }
    
    if (conn->hostname) {
        free(conn->hostname);
    }
    
    free(conn);
}

const char* connection_error(Connection* conn) {
    return conn ? conn->error_buffer : "NULL connection";
}

/* ======================================================================== */
/* HTTP Utilities                                                           */
/* ======================================================================== */

char* http_build_request(const char* method, const char* path,
                         const char* host, const char* headers,
                         const char* body, size_t body_length) {
    /* Calculate size needed */
    size_t size = strlen(method) + strlen(path) + strlen(host) + 100;
    if (headers) size += strlen(headers);
    if (body) size += body_length + 50;
    
    char* request = malloc(size);
    if (!request) return NULL;
    
    /* Build request line */
    int offset = snprintf(request, size, "%s %s HTTP/1.1\r\n", method, path);
    
    /* Add Host header */
    offset += snprintf(request + offset, size - offset, "Host: %s\r\n", host);
    
    /* Add Connection header */
    offset += snprintf(request + offset, size - offset, "Connection: close\r\n");
    
    /* Add user headers */
    if (headers && strlen(headers) > 0) {
        offset += snprintf(request + offset, size - offset, "%s", headers);
    }
    
    /* Add Content-Length if body present */
    if (body && body_length > 0) {
        offset += snprintf(request + offset, size - offset,
                          "Content-Length: %zu\r\n", body_length);
    }
    
    /* End headers */
    offset += snprintf(request + offset, size - offset, "\r\n");
    
    /* Add body */
    if (body && body_length > 0) {
        memcpy(request + offset, body, body_length);
        offset += body_length;
    }
    
    request[offset] = '\0';
    return request;
}

bool http_parse_status_line(const char* line, int* status_code, char** status_text) {
    char version[16];
    int code;
    char* text_start;
    
    /* Parse: HTTP/1.1 200 OK */
    if (sscanf(line, "%15s %d", version, &code) != 2) {
        return false;
    }
    
    /* Find status text */
    text_start = strchr(line, ' ');
    if (text_start) {
        text_start++; /* Skip first space */
        text_start = strchr(text_start, ' ');
        if (text_start) {
            text_start++; /* Skip second space */
            
            /* Remove trailing whitespace */
            size_t len = strlen(text_start);
            while (len > 0 && isspace(text_start[len - 1])) {
                len--;
            }
            
            if (status_text) {
                *status_text = malloc(len + 1);
                if (*status_text) {
                    strncpy(*status_text, text_start, len);
                    (*status_text)[len] = '\0';
                }
            }
        }
    }
    
    if (status_code) {
        *status_code = code;
    }
    
    return true;
}

bool http_parse_header(const char* line, char** key, char** value) {
    const char* colon = strchr(line, ':');
    if (!colon) return false;
    
    /* Extract key */
    size_t key_len = colon - line;
    if (key) {
        *key = malloc(key_len + 1);
        if (*key) {
            strncpy(*key, line, key_len);
            (*key)[key_len] = '\0';
        }
    }
    
    /* Extract value (skip colon and whitespace) */
    const char* val_start = colon + 1;
    while (*val_start && isspace(*val_start)) {
        val_start++;
    }
    
    /* Remove trailing whitespace */
    size_t val_len = strlen(val_start);
    while (val_len > 0 && isspace(val_start[val_len - 1])) {
        val_len--;
    }
    
    if (value) {
        *value = malloc(val_len + 1);
        if (*value) {
            strncpy(*value, val_start, val_len);
            (*value)[val_len] = '\0';
        }
    }
    
    return true;
}