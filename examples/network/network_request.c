#ifndef NETWORK_REQUEST_IMPL_C
#define NETWORK_REQUEST_IMPL_C

#include <trampoline.h>
#include <stdlib.h>
#include <stddef.h>
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

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/crypto.h>

#include "network_request.h"
#include "network_response.h"
#include "network_response_private.h"

typedef struct HeaderNode {
  char* key;
  char* value;
  struct HeaderNode* next;
} HeaderNode;

typedef struct NetworkRequest_ {
  NetworkRequest public;

  char* url;
  HttpMethod method;
  HeaderNode* headers;
  char* body;
  size_t body_length;
  int port;
  int timeout_seconds;
} NetworkRequest_;

TRAMP_GETTER_(networkrequest_url, NetworkRequest, NetworkRequest_, const char*, url);
TRAMP_STRING_SETTER_(networkrequest_setUrl, NetworkRequest, NetworkRequest_, url);

TRAMP_GETTER_(networkrequest_method, NetworkRequest, NetworkRequest_, HttpMethod, method);
TRAMP_SETTER_(networkrequest_setMethod, NetworkRequest, NetworkRequest_, HttpMethod, method);

TRAMP_GETTER_(networkrequest_body, NetworkRequest, NetworkRequest_, const char*, body);
TRAMP_GETTER_(networkrequest_bodyLength, NetworkRequest, NetworkRequest_, size_t, body_length);

TRAMP_GETTER_(networkrequest_port, NetworkRequest, NetworkRequest_, int, port);
TRAMP_SETTER_(networkrequest_setPort, NetworkRequest, NetworkRequest_, int, port);

TRAMP_GETTER_(networkrequest_timeout, NetworkRequest, NetworkRequest_, int, timeout_seconds);
TRAMP_SETTER_(networkrequest_setTimeout, NetworkRequest, NetworkRequest_, int, timeout_seconds);

const char* networkrequest_header(
  NetworkRequest* self,
  const char* key
) {
  NetworkRequest_* private = (NetworkRequest_*)self;
  HeaderNode* current = private->headers;

  while (current) {
    if (strcmp(current->key, key) == 0) {
      return current->value;
    }
    current = current->next;
  }

  return NULL;
}

void networkrequest_setHeader(
  NetworkRequest* self, 
  const char* key,
  const char* value
) {
  NetworkRequest_* private = (NetworkRequest_*)self;
  HeaderNode* current = private->headers;
  HeaderNode* prev = NULL;

  while (current) {
    if (strcmp(current->key, key) == 0) {
      free(current->value);
      current->value = calloc(1, strlen(value) + 1);
      if (current->value) {
        strcpy(current->value, value);
      }
      return;
    }
    prev = current;
    current = current->next;
  }

  HeaderNode* new_header = (HeaderNode*)calloc(1, sizeof(HeaderNode));
  if (new_header) {
    new_header->key = calloc(1, strlen(key) + 1);
    new_header->value = calloc(1, strlen(value) + 1);

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

void networkrequest_removeHeader(NetworkRequest* self, const char* key) {
  NetworkRequest_* private = (NetworkRequest_*)self;
  HeaderNode* current = private->headers;
  HeaderNode* prev = NULL;

  while (current) {
    if (strcmp(current->key, key) == 0) {
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

void networkrequest_setBody(NetworkRequest* self, const char* body) {
  NetworkRequest_* private = (NetworkRequest_*)self;

  if (private->body) {
    free(private->body);
  }

  if (body) {
    private->body_length = strlen(body);
    private->body = calloc(1, private->body_length + 1);
    if (private->body) {
      strcpy(private->body, body);
    }
  } else {
    private->body = NULL;
    private->body_length = 0;
  }
}

static const char* http_method_strings[] = {
  "GET", "POST", "PUT", "DELETE", "PATCH", "HEAD", "OPTIONS"
};

/**
 * @brief Parses a URL string into its component parts
 * 
 * This function modifies the original URL string by null-terminating
 * components. The original string must remain valid while using the
 * parsed_url structure.
 * 
 * @param url_string The URL string to parse (will be modified)
 * @param parsed_url Pointer to url_t structure to fill with components
 * @return 0 on success, -1 on error
 * 
 * @note The input url_string will be modified (null terminators added)
 * @note All pointers in parsed_url reference the original url_string
 * @note Path will NOT include leading '/' - add it when building requests
 */
int parse_url_t(char *url_string, url_t *parsed_url) {
  if (!url_string || !parsed_url) {
    return -1;
  }
  
  // Initialize all fields to NULL
  memset(parsed_url, 0, sizeof(url_t));
  
  char *current = url_string;
  char *temp;
  
  // Parse scheme (everything before "://")
  temp = strstr(current, "://");
  if (!temp) {
    return -1;  // Invalid URL - no scheme
  }
  
  *temp = '\0';  // Null terminate scheme
  parsed_url->scheme = current;
  current = temp + 3;  // Skip past "://"
  
  // Parse host (everything up to ':', '/', '?', or '#')
  parsed_url->host = current;
  temp = strpbrk(current, ":/?#");
  
  if (temp) {
    char found_char = *temp;  // Remember what delimiter we found
    *temp = '\0';            // Null terminate host
    current = temp + 1;      // Move past the delimiter
    
    if (found_char == ':') {
      // Port is present
      parsed_url->port = current;
      
      // Find end of port
      temp = strpbrk(current, "/?#");
      if (temp) {
        found_char = *temp;
        *temp = '\0';
        current = temp + 1;
      }
      else {
        return 0;  // URL ends with port
      }
    }
    
    // Parse path (everything up to '?' or '#')
    if (found_char == '/') {
      // Path starts after the '/' delimiter we found
      // Note: path will NOT include the leading '/'
      parsed_url->path = current;
      
      temp = strpbrk(current, "?#");
      
      printf("Host: '%s', Path: '%s' (add leading / when using)\n", 
             parsed_url->host, parsed_url->path);
      
      if (temp) {
        found_char = *temp;
        *temp = '\0';
        current = temp + 1;
      }
      else {
        return 0;  // URL ends with path
      }
    }
    
    // Parse query (everything between '?' and '#')
    if (found_char == '?') {
      parsed_url->query = current;
      temp = strchr(current, '#');
      
      if (temp) {
        *temp = '\0';
        current = temp + 1;
        found_char = '#';
      }
      else {
        return 0;  // URL ends with query
      }
    }
    
    // Parse fragment (everything after '#')
    if (found_char == '#') {
      parsed_url->fragment = current;
    }
  }
  else {
    return 0;  // URL ends with host
  }
  
  return 0;
}
static NetworkResponse* parse_http_response(
  const char* raw_response,
  size_t response_len
) {
  NetworkResponse* response = NetworkResponseMake();
  
  if (!response) 
    return NULL;

  {
    NetworkResponse_* private = (NetworkResponse_*)response;
    const char* line_end = strstr(raw_response, "\r\n");
    char status_line[256];
    size_t status_len;
    char version[16];
    char status_msg[128] = "";

    if (!line_end) {
      response->free();
      return NetworkResponseMakeError("Invalid HTTP response");
    }

    status_len = line_end - raw_response;
    
    if (status_len >= sizeof(status_line)) 
      status_len = sizeof(status_line) - 1;
      
    strncpy(status_line, raw_response, status_len);
    status_line[status_len] = '\0';
    
    if (sscanf(
      status_line, 
      "%15s %d %127[^\r\n]", 
      version,
      &private->status_code, 
      status_msg
    ) < 2) {
      response->free();
      return NetworkResponseMakeError("Failed to parse status line");
    }

    if (strlen(status_msg) > 0) {
      private->status_message = calloc(1, strlen(status_msg) + 1);
      strcpy(private->status_message, status_msg);
    }

    const char* headers_start = line_end + 2;
    const char* headers_end = strstr(headers_start, "\r\n\r\n");

    if (!headers_end) {
      response->free();
      return NetworkResponseMakeError("Invalid HTTP headers");
    }

    const char* header_line = headers_start;
    while (header_line < headers_end) {
      const char* next_line = strstr(header_line, "\r\n");
      if (!next_line) break;

      const char* colon = strchr(header_line, ':');
      if (colon && colon < next_line) {
        size_t key_len = colon - header_line;
        char* key = calloc(1, key_len + 1);
        strncpy(key, header_line, key_len);

        const char* value_start = colon + 1;
        while (*value_start == ' ' || *value_start == '\t') value_start++;

        size_t value_len = next_line - value_start;
        char* value = calloc(1, value_len + 1);
        strncpy(value, value_start, value_len);

        networkresponse_addHeader(private, key, value);

        free(key);
        free(value);
      }

      header_line = next_line + 2;
    }

    const char* body_start = headers_end + 4;
    size_t body_len = response_len - (body_start - raw_response);

    if (body_len > 0) {
      private->body = calloc(1, body_len + 1);
      memcpy(private->body, body_start, body_len);
      private->body_length = body_len;
    }
  }

  return response;
}

char* string_to_lower(const char* source, char* dest, size_t dest_size) {
  size_t i;
  size_t source_len;
  
  if (!source || !dest || dest_size == 0) {
    return NULL;
  }
  
  source_len = strlen(source);
  
  /* Check if destination buffer is large enough */
  if (dest_size <= source_len) {
    return NULL;
  }
  
  /* Convert each character to lowercase */
  for (i = 0; i < source_len; i++) {
    dest[i] = (char)tolower((unsigned char)source[i]);
  }
  
  /* Null terminate the result */
  dest[source_len] = '\0';
  
  return dest;
}

/**
 * @brief Sends a network request with support for both HTTP and HTTPS protocols
 * 
 * This function handles HTTP and HTTPS requests by establishing appropriate connections.
 * For HTTPS requests, it uses OpenSSL to create a secure SSL/TLS connection.
 * The function parses the URL, establishes a connection, sends the request with headers
 * and body (if present), and returns the server response.
 * 
 * @param self Pointer to the NetworkRequest structure containing request details
 * @return NetworkResponse* Pointer to response structure, or error response on failure
 * 
 * @note Requires OpenSSL library to be linked and initialized
 * @note The caller is responsible for freeing the returned NetworkResponse
 */
NetworkResponse* networkrequest_send(NetworkRequest* self) {
  NetworkRequest_* private = (NetworkRequest_*)self;
  SSL_CTX* ctx = NULL;
  SSL* ssl = NULL;
  url_t parsed = { 0 };
  char scheme[32] = { 0 };
  int port;
  int is_https = 0;
  int sockfd = -1;
  NetworkResponse* error_response = NULL;
  
  if (!private->url) {
    return NetworkResponseMakeError("No URL specified");
  }
  
  parse_url_t(private->url, &parsed);
  
  // Determine port and protocol
  if (parsed.port) {
    port = atoi(parsed.port);
  }
  else {
    string_to_lower(parsed.scheme, scheme, strlen(parsed.scheme) + 1);
    
    if (strcmp(scheme, "https") == 0) {
      port = 443;
      is_https = 1;
    }
    else {
      port = 80;
      is_https = 0;
    }
  }
  
  char _path[2048] = { 0 };
  
  strcpy(_path, "/");
  strcat(_path, parsed.path ? parsed.path : "");
  
  parsed.path = _path;
  
  printf("scheme: %s\n", parsed.scheme);
  printf("host: %s\n", parsed.host);
  printf("path: %s\n", parsed.path ? parsed.path : "/");
  printf("port: %d\n", port);
  
  // Initialize OpenSSL for HTTPS connections
  if (is_https) {
    SSL_library_init();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
    
    // Create SSL context
    ctx = SSL_CTX_new(TLS_client_method());
    if (!ctx) {
      return NetworkResponseMakeError("Failed to create SSL context");
    }
    else {
      printf("SSL context: %p\n", ctx);
    }
    
    // Configure SSL context for security
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
    SSL_CTX_set_default_verify_paths(ctx);
  }
  
  // Resolve hostname
  struct hostent* host = gethostbyname(parsed.host);
  if (!host) {
    error_response = NetworkResponseMakeError("Failed to resolve hostname");
    printf("Failed to resolve hostname\n");
    goto cleanup;
  }
  
  // Create socket
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    error_response = NetworkResponseMakeError("Failed to create socket");
    printf("Failed to create socket\n");
    goto cleanup;
  }
  
  // Set socket timeouts
  struct timeval tv;
  tv.tv_sec = private->timeout_seconds;
  tv.tv_usec = 0;
  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
  setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
  
  // Connect to server
  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  memcpy(&server_addr.sin_addr.s_addr, host->h_addr, host->h_length);
  
  if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
    printf("Failed to connect to server\n");
    error_response = NetworkResponseMakeError("Failed to connect to server");
    goto cleanup;
  }
  
  // Establish SSL connection for HTTPS
  if (is_https) {
    ssl = SSL_new(ctx);
    if (!ssl) {
      printf("Failed to create SSL structure\n");
      error_response = NetworkResponseMakeError("Failed to create SSL structure");
      goto cleanup;
    }
    
    if (SSL_set_fd(ssl, sockfd) != 1) {
      printf("Failed to bind SSL to socket\n");
      error_response = NetworkResponseMakeError("Failed to bind SSL to socket");
      goto cleanup;
    }
    
    // Set hostname for SNI (Server Name Indication)
    SSL_set_tlsext_host_name(ssl, parsed.host);
    
    if (SSL_connect(ssl) != 1) {
      printf("Failed to establish SSL connection\n");
      error_response = NetworkResponseMakeError("Failed to establish SSL connection");
      goto cleanup;
    }
  }
  
  // Build HTTP request
  char request[4096];
  int request_len = snprintf(request, sizeof(request),
    "%s %s HTTP/1.1\r\n"
    "Host: %s\r\n"
    "Connection: close\r\n",
    http_method_strings[private->method],
    parsed.path ? parsed.path : "/",
    parsed.host
  );
  
  // Add custom headers
  HeaderNode* header = private->headers;
  while (header && (size_t)request_len < sizeof(request) - 100) {
    request_len += snprintf(request + request_len,
                            sizeof(request) - request_len,
                            "%s: %s\r\n", header->key, header->value);
    header = header->next;
  }
  
  // Add Content-Length header if body is present
  if (private->body && private->body_length > 0) {
    request_len += snprintf(request + request_len,
                            sizeof(request) - request_len,
                            "Content-Length: %zu\r\n", private->body_length);
  }
  
  // End headers
  request_len += snprintf(
    request + request_len,
    sizeof(request) - request_len, 
    "\r\n"
  );
                          
  printf("Headers completed, request so far is\n\n%s\n", request);  
  
  // Send request headers
  int send_result;
  if (is_https) {
    send_result = SSL_write(ssl, request, request_len);
    printf("SSL headers written, wrote %d byte(s)\n", send_result);
  }
  else {
    send_result = send(sockfd, request, request_len, 0);
    printf("HTTP headers written, result is %d\n", send_result);
  }
  
  if (send_result < 0) {
    error_response = NetworkResponseMakeError("Failed to send request");
    goto cleanup;
  }
  
  // Send request body if present
  if (private->body && private->body_length > 0) {
    if (is_https) {
      send_result = SSL_write(ssl, private->body, private->body_length);
      printf("SSL body written, wrote %d byte(s)\n", send_result);
    }
    else {
      send_result = send(sockfd, private->body, private->body_length, 0);
      printf("HTTP body written, wrote %d byte(s)\n", send_result);
    }
    
    if (send_result < 0) {
      error_response = NetworkResponseMakeError("Failed to send request body");
      goto cleanup;
    }
  }
  
  // Receive response
  {
    char response_buffer[65536] = { 0 };
    size_t total_received = 0;
    ssize_t bytes_received;
    
    while (total_received < sizeof(response_buffer) - 1) {
      if (is_https) {
        bytes_received = SSL_read(
          ssl, 
          response_buffer + total_received,
          sizeof(response_buffer) - total_received - 1
        );
        printf("SSL response is %ld bytes long\n", bytes_received);
      }
      else {
        bytes_received = recv(
          sockfd, 
          response_buffer + total_received,
          sizeof(response_buffer) - total_received - 1, 
          0
        );
        
        printf("HTTP response is %ld bytes long\n", bytes_received);
      }
      
      if (bytes_received <= 0) {
        break;
      }
      
      total_received += bytes_received;
    }
    
    printf("Response:\n\n%s\n", response_buffer);
    
    if (total_received == 0) {
      error_response = NetworkResponseMakeError("No response received");
      goto cleanup;
    }
    
    response_buffer[total_received] = '\0';
    NetworkResponse* response = parse_http_response(response_buffer, total_received);
    
    // Cleanup and return successful response
    goto cleanup_success;
    
cleanup_success:
    // Cleanup resources before returning successful response
    if (ssl) {
      SSL_shutdown(ssl);
      SSL_free(ssl);
    }
    if (ctx) {
      SSL_CTX_free(ctx);
    }
    if (sockfd >= 0) {
      close(sockfd);
    }
    return response;
  }
  
cleanup:
  // Cleanup resources on error
  if (ssl) {
    SSL_shutdown(ssl);
    SSL_free(ssl);
  }
  if (ctx) {
    SSL_CTX_free(ctx);
  }
  if (sockfd >= 0) {
    close(sockfd);
  }
  
  return error_response;
}

void networkrequest_free(NetworkRequest* self) {
  NetworkRequest_* private = (NetworkRequest_*)self;

  if (private) {
    if (private->url) free(private->url);
    if (private->body) free(private->body);

    HeaderNode* current = private->headers;
    while (current) {
      HeaderNode* next = current->next;
      free(current->key);
      free(current->value);
      free(current);
      current = next;
    }

    trampoline_free(self->url);
    trampoline_free(self->setUrl);
    trampoline_free(self->method);
    trampoline_free(self->setMethod);
    trampoline_free(self->header);
    trampoline_free(self->setHeader);
    trampoline_free(self->removeHeader);
    trampoline_free(self->body);
    trampoline_free(self->setBody);
    trampoline_free(self->bodyLength);
    trampoline_free(self->port);
    trampoline_free(self->setPort);
    trampoline_free(self->timeout);
    trampoline_free(self->setTimeout);
    trampoline_free(self->send);
    trampoline_free(self->free);

    free(private);
  }
}

NetworkRequest* NetworkRequestMake(const char* url, HttpMethod method) {
  NetworkRequest_* private = (NetworkRequest_*)calloc(1, sizeof(NetworkRequest_));
  NetworkRequest* public = (NetworkRequest*)private;
  trampoline_allocations list = { 0 };

  if (private) {
    if (url) {
      private->url = calloc(1, strlen(url) + 1);
      if (private->url) {
        strcpy(private->url, url);
      }
    }

    private->method = method;
    private->port = 80;
    private->timeout_seconds = 30;

    public->url = trampoline_create_and_track(networkrequest_url, public,
                                               0, &list);
    public->setUrl = trampoline_create_and_track(networkrequest_setUrl,
                                                  public, 1, &list);

    public->method = trampoline_create_and_track(networkrequest_method,
                                                  public, 0, &list);
    public->setMethod = trampoline_create_and_track(
        networkrequest_setMethod, public, 1, &list);

    public->header = trampoline_create_and_track(networkrequest_header,
                                                  public, 1, &list);
    public->setHeader = trampoline_create_and_track(
        networkrequest_setHeader, public, 2, &list);
    public->removeHeader = trampoline_create_and_track(
        networkrequest_removeHeader, public, 1, &list);

    public->body = trampoline_create_and_track(networkrequest_body,
                                                public, 0, &list);
    public->setBody = trampoline_create_and_track(networkrequest_setBody,
                                                   public, 1, &list);
    public->bodyLength = trampoline_create_and_track(
        networkrequest_bodyLength, public, 0, &list);

    public->port = trampoline_create_and_track(networkrequest_port,
                                                public, 0, &list);
    public->setPort = trampoline_create_and_track(networkrequest_setPort,
                                                   public, 1, &list);

    public->timeout = trampoline_create_and_track(networkrequest_timeout,
                                                   public, 0, &list);
    public->setTimeout = trampoline_create_and_track(
        networkrequest_setTimeout, public, 1, &list);

    public->send = trampoline_create_and_track(networkrequest_send,
                                                public, 0, &list);

    public->free = trampoline_create_and_track(networkrequest_free,
                                                public, 0, &list);

    if (!trampolines_validate(&list)) {
      if (private->url) free(private->url);
      free(private);
      public = NULL;
    }
  }

  return public;
}

#endif
