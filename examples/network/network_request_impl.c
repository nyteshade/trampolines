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

#include "network_request.h"
#include "network_response.h"
#include "network_response_impl.c"

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

TRAMP_GETTER_(networkrequest_url, NetworkRequest, NetworkRequest_,
              const char*, url);
TRAMP_STRING_SETTER_(networkrequest_setUrl, NetworkRequest,
                     NetworkRequest_, url);

TRAMP_GETTER_(networkrequest_method, NetworkRequest, NetworkRequest_,
              HttpMethod, method);
TRAMP_SETTER_(networkrequest_setMethod, NetworkRequest, NetworkRequest_,
              HttpMethod, method);

TRAMP_GETTER_(networkrequest_body, NetworkRequest, NetworkRequest_,
              const char*, body);
TRAMP_GETTER_(networkrequest_bodyLength, NetworkRequest, NetworkRequest_,
              size_t, body_length);

TRAMP_GETTER_(networkrequest_port, NetworkRequest, NetworkRequest_,
              int, port);
TRAMP_SETTER_(networkrequest_setPort, NetworkRequest, NetworkRequest_,
              int, port);

TRAMP_GETTER_(networkrequest_timeout, NetworkRequest, NetworkRequest_,
              int, timeout_seconds);
TRAMP_SETTER_(networkrequest_setTimeout, NetworkRequest, NetworkRequest_,
              int, timeout_seconds);

const char* networkrequest_header(NetworkRequest* self,
                                   const char* key) {
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

void networkrequest_setHeader(NetworkRequest* self, const char* key,
                               const char* value) {
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

static int parse_url(const char* url, char** hostname, char** path, int* port) {
  if (!url) return -1;

  const char* url_start = url;
  if (strncmp(url, "http://", 7) == 0) {
    url_start += 7;
  } else if (strncmp(url, "https://", 8) == 0) {
    *port = 443;
    return -1;
  }

  const char* path_start = strchr(url_start, '/');
  size_t host_len;

  if (path_start) {
    host_len = path_start - url_start;
    *path = calloc(1, strlen(path_start) + 1);
    strcpy(*path, path_start);
  } else {
    host_len = strlen(url_start);
    *path = calloc(1, 2);
    strcpy(*path, "/");
  }

  *hostname = calloc(1, host_len + 1);
  strncpy(*hostname, url_start, host_len);

  char* port_start = strchr(*hostname, ':');
  if (port_start) {
    *port_start = '\0';
    *port = atoi(port_start + 1);
  }

  return 0;
}

static NetworkResponse* parse_http_response(const char* raw_response,
                                             size_t response_len) {
  NetworkResponse* response = NetworkResponseMake();
  if (!response) return NULL;

  NetworkResponse_* private = (NetworkResponse_*)response;

  const char* line_end = strstr(raw_response, "\r\n");
  if (!line_end) {
    response->free();
    return NetworkResponseMakeError("Invalid HTTP response");
  }

  char status_line[256];
  size_t status_len = line_end - raw_response;
  if (status_len >= sizeof(status_line)) status_len = sizeof(status_line) - 1;
  strncpy(status_line, raw_response, status_len);
  status_line[status_len] = '\0';

  char version[16];
  char status_msg[128] = "";
  if (sscanf(status_line, "%15s %d %127[^\r\n]", version,
             &private->status_code, status_msg) < 2) {
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

  return response;
}

NetworkResponse* networkrequest_send(NetworkRequest* self) {
  NetworkRequest_* private = (NetworkRequest_*)self;

  if (!private->url) {
    return NetworkResponseMakeError("No URL specified");
  }

  char* hostname = NULL;
  char* path = NULL;
  int port = private->port;

  if (parse_url(private->url, &hostname, &path, &port) != 0) {
    if (hostname) free(hostname);
    if (path) free(path);
    return NetworkResponseMakeError(
        "Failed to parse URL or HTTPS not supported");
  }

  struct hostent* host = gethostbyname(hostname);
  if (!host) {
    free(hostname);
    free(path);
    return NetworkResponseMakeError("Failed to resolve hostname");
  }

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    free(hostname);
    free(path);
    return NetworkResponseMakeError("Failed to create socket");
  }

  struct timeval tv;
  tv.tv_sec = private->timeout_seconds;
  tv.tv_usec = 0;
  setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
  setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  memcpy(&server_addr.sin_addr.s_addr, host->h_addr, host->h_length);

  if (connect(sockfd, (struct sockaddr*)&server_addr,
              sizeof(server_addr)) < 0) {
    close(sockfd);
    free(hostname);
    free(path);
    return NetworkResponseMakeError("Failed to connect to server");
  }

  char request[4096];
  int request_len = snprintf(request, sizeof(request),
    "%s %s HTTP/1.1\r\n"
    "Host: %s\r\n"
    "Connection: close\r\n",
    http_method_strings[private->method],
    path,
    hostname
  );

  HeaderNode* header = private->headers;
  while (header && (size_t)request_len < sizeof(request) - 100) {
    request_len += snprintf(request + request_len,
                            sizeof(request) - request_len,
                            "%s: %s\r\n", header->key, header->value);
    header = header->next;
  }

  if (private->body && private->body_length > 0) {
    request_len += snprintf(request + request_len,
                            sizeof(request) - request_len,
                            "Content-Length: %zu\r\n", private->body_length);
  }

  request_len += snprintf(request + request_len,
                          sizeof(request) - request_len, "\r\n");

  if (send(sockfd, request, request_len, 0) < 0) {
    close(sockfd);
    free(hostname);
    free(path);
    return NetworkResponseMakeError("Failed to send request");
  }

  if (private->body && private->body_length > 0) {
    if (send(sockfd, private->body, private->body_length, 0) < 0) {
      close(sockfd);
      free(hostname);
      free(path);
      return NetworkResponseMakeError("Failed to send request body");
    }
  }

  char response_buffer[65536];
  size_t total_received = 0;
  ssize_t bytes_received;

  while ((bytes_received = recv(sockfd, response_buffer + total_received,
          sizeof(response_buffer) - total_received - 1, 0)) > 0) {
    total_received += bytes_received;
    if (total_received >= sizeof(response_buffer) - 1) break;
  }

  close(sockfd);
  free(hostname);
  free(path);

  if (total_received == 0) {
    return NetworkResponseMakeError("No response received");
  }

  response_buffer[total_received] = '\0';

  return parse_http_response(response_buffer, total_received);
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
