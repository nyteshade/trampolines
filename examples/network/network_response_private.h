#ifndef NETWORK_RESPONSE_PRIVATE_H
#define NETWORK_RESPONSE_PRIVATE_H

typedef struct ResponseHeader {
  char* key;
  char* value;
  struct ResponseHeader* next;
} ResponseHeader;

typedef struct NetworkResponse_ {
  NetworkResponse public;
  
  int status_code;
  char* status_message;
  ResponseHeader* headers;
  size_t header_count;
  char** header_keys_cache;
  char* body;
  size_t body_length;
  char* error;
} NetworkResponse_;

void networkresponse_addHeader(
  NetworkResponse_* private, 
  const char* key, 
  const char* value
);

void networkresponse_free(NetworkResponse* self);


#endif