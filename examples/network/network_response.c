#ifndef NETWORK_RESPONSE_IMPL_C
#define NETWORK_RESPONSE_IMPL_C

#include <trampoline.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "network_response.h"
#include "network_response_private.h"

TRAMP_GETTER_(networkresponse_statusCode, NetworkResponse, 
              NetworkResponse_, int, status_code);
TRAMP_GETTER_(networkresponse_statusMessage, NetworkResponse, 
              NetworkResponse_, const char*, status_message);
TRAMP_GETTER_(networkresponse_body, NetworkResponse, NetworkResponse_, 
              const char*, body);
TRAMP_GETTER_(networkresponse_bodyLength, NetworkResponse, 
              NetworkResponse_, size_t, body_length);
TRAMP_GETTER_(networkresponse_headerCount, NetworkResponse, 
              NetworkResponse_, size_t, header_count);
TRAMP_GETTER_(networkresponse_error, NetworkResponse, NetworkResponse_, 
              const char*, error);

int networkresponse_isSuccess(NetworkResponse* self) {
  NetworkResponse_* private = (NetworkResponse_*)self;
  return (private->status_code >= 200 && 
          private->status_code < 300 && 
          !private->error);
}

const char* networkresponse_header(NetworkResponse* self, const char* key) {
  NetworkResponse_* private = (NetworkResponse_*)self;
  ResponseHeader* current = private->headers;
  
  while (current) {
    if (strcasecmp(current->key, key) == 0) {
      return current->value;
    }
    current = current->next;
  }
  
  return NULL;
}

const char** networkresponse_allHeaderKeys(NetworkResponse* self) {
  NetworkResponse_* private = (NetworkResponse_*)self;
  
  if (!private->header_keys_cache && private->header_count > 0) {
    private->header_keys_cache = (char**)calloc(
        private->header_count + 1, sizeof(char*));
    if (private->header_keys_cache) {
      ResponseHeader* current = private->headers;
      size_t i = 0;
      while (current && i < private->header_count) {
        private->header_keys_cache[i++] = current->key;
        current = current->next;
      }
    }
  }
  
  return (const char**)private->header_keys_cache;
}

void networkresponse_free(NetworkResponse* self) {
  NetworkResponse_* private = (NetworkResponse_*)self;
  
  if (private) {
    if (private->status_message) free(private->status_message);
    if (private->body) free(private->body);
    if (private->error) free(private->error);
    if (private->header_keys_cache) free(private->header_keys_cache);
    
    ResponseHeader* current = private->headers;
    while (current) {
      ResponseHeader* next = current->next;
      free(current->key);
      free(current->value);
      free(current);
      current = next;
    }
    
    trampoline_free(self->statusCode);
    trampoline_free(self->statusMessage);
    trampoline_free(self->header);
    trampoline_free(self->allHeaderKeys);
    trampoline_free(self->headerCount);
    trampoline_free(self->body);
    trampoline_free(self->bodyLength);
    trampoline_free(self->isSuccess);
    trampoline_free(self->error);
    trampoline_free(self->free);
    
    free(private);
  }
}

void networkresponse_addHeader(NetworkResponse_* private, 
                                const char* key, const char* value) {
  ResponseHeader* new_header = (ResponseHeader*)calloc(1, sizeof(ResponseHeader));
  if (new_header) {
    new_header->key = calloc(1, strlen(key) + 1);
    new_header->value = calloc(1, strlen(value) + 1);
    
    if (new_header->key && new_header->value) {
      strcpy(new_header->key, key);
      strcpy(new_header->value, value);
      
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
    } else {
      if (new_header->key) free(new_header->key);
      if (new_header->value) free(new_header->value);
      free(new_header);
    }
  }
}

NetworkResponse* NetworkResponseMake() {
  NetworkResponse_* private = (NetworkResponse_*)calloc(
      1, sizeof(NetworkResponse_));
  NetworkResponse* public = (NetworkResponse*)private;
  trampoline_allocations list = { 0 };
  
  if (private) {
    public->statusCode = trampoline_create_and_track(
        networkresponse_statusCode, public, 0, &list);
    public->statusMessage = trampoline_create_and_track(
        networkresponse_statusMessage, public, 0, &list);
    public->header = trampoline_create_and_track(
        networkresponse_header, public, 1, &list);
    public->allHeaderKeys = trampoline_create_and_track(
        networkresponse_allHeaderKeys, public, 0, &list);
    public->headerCount = trampoline_create_and_track(
        networkresponse_headerCount, public, 0, &list);
    public->body = trampoline_create_and_track(
        networkresponse_body, public, 0, &list);
    public->bodyLength = trampoline_create_and_track(
        networkresponse_bodyLength, public, 0, &list);
    public->isSuccess = trampoline_create_and_track(
        networkresponse_isSuccess, public, 0, &list);
    public->error = trampoline_create_and_track(
        networkresponse_error, public, 0, &list);
    public->free = trampoline_create_and_track(
        networkresponse_free, public, 0, &list);
    
    if (!trampolines_validate(&list)) {
      free(private);
      public = NULL;
    }
  }
  
  return public;
}

NetworkResponse* NetworkResponseMakeError(const char* error) {
  NetworkResponse* response = NetworkResponseMake();
  if (response) {
    NetworkResponse_* private = (NetworkResponse_*)response;
    private->error = calloc(1, strlen(error) + 1);
    if (private->error) {
      strcpy(private->error, error);
    }
  }
  return response;
}

#endif