/**
 * @file network_common.h
 * @brief Common network utilities and SSL abstraction layer
 */

#ifndef NETWORK_COMMON_H
#define NETWORK_COMMON_H

#include <stddef.h>
#include <stdbool.h>

/* Platform detection for socket headers */
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <sys/select.h>
#endif

/* SSL Support - can be disabled at compile time */
#ifndef NO_SSL_SUPPORT
    #include <openssl/ssl.h>
    #include <openssl/err.h>
    #include <openssl/bio.h>
    #define SSL_SUPPORT 1
#else
    #define SSL_SUPPORT 0
#endif

/* ======================================================================== */
/* Connection Abstraction                                                   */
/* ======================================================================== */

typedef enum {
    CONN_TYPE_PLAIN,
    CONN_TYPE_SSL
} ConnectionType;

typedef struct Connection {
    ConnectionType type;
    int socket_fd;
    
#if SSL_SUPPORT
    SSL_CTX* ssl_ctx;
    SSL* ssl;
#endif
    
    /* Connection info */
    char* hostname;
    int port;
    int timeout_seconds;
    
    /* Error handling */
    char error_buffer[256];
    int last_error;
} Connection;

/* ======================================================================== */
/* Connection Functions                                                      */
/* ======================================================================== */

/**
 * Initialize SSL library (call once at program start)
 */
void network_init_ssl(void);

/**
 * Cleanup SSL library (call at program end)
 */
void network_cleanup_ssl(void);

/**
 * Create a new connection
 */
Connection* connection_create(const char* hostname, int port, bool use_ssl);

/**
 * Connect to the server
 */
bool connection_connect(Connection* conn);

/**
 * Send data over the connection
 */
ssize_t connection_send(Connection* conn, const void* data, size_t length);

/**
 * Receive data from the connection
 */
ssize_t connection_recv(Connection* conn, void* buffer, size_t buffer_size);

/**
 * Close and free the connection
 */
void connection_free(Connection* conn);

/**
 * Get last error message
 */
const char* connection_error(Connection* conn);

/* ======================================================================== */
/* HTTP Utilities                                                           */
/* ======================================================================== */

/**
 * Build HTTP request headers
 */
char* http_build_request(const char* method, const char* path, 
                         const char* host, const char* headers,
                         const char* body, size_t body_length);

/**
 * Parse HTTP response status line
 */
bool http_parse_status_line(const char* line, int* status_code, char** status_text);

/**
 * Parse a single HTTP header
 */
bool http_parse_header(const char* line, char** key, char** value);

#endif /* NETWORK_COMMON_H */