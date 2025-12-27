/**
 * @file network.h
 * @brief Network classes for HTTP requests and responses using trampolines
 */

#ifndef TRAMPOLINES_NETWORK_H
#define TRAMPOLINES_NETWORK_H

#include <trampoline/trampoline.h>
#include <trampoline/macros.h>
#include <trampoline/classes/string.h>
#include <trampoline/classes/json.h>
#include <stddef.h>

/* ======================================================================== */
/* HTTP Types                                                               */
/* ======================================================================== */

typedef enum HttpMethod {
  HTTP_GET,
  HTTP_POST,
  HTTP_PUT,
  HTTP_DELETE,
  HTTP_PATCH,
  HTTP_HEAD,
  HTTP_OPTIONS
} HttpMethod;

typedef enum HttpStatus {
  HTTP_CONTINUE = 100,
  HTTP_OK = 200,
  HTTP_CREATED = 201,
  HTTP_ACCEPTED = 202,
  HTTP_NO_CONTENT = 204,
  HTTP_MOVED_PERMANENTLY = 301,
  HTTP_FOUND = 302,
  HTTP_NOT_MODIFIED = 304,
  HTTP_BAD_REQUEST = 400,
  HTTP_UNAUTHORIZED = 401,
  HTTP_FORBIDDEN = 403,
  HTTP_NOT_FOUND = 404,
  HTTP_METHOD_NOT_ALLOWED = 405,
  HTTP_INTERNAL_SERVER_ERROR = 500,
  HTTP_NOT_IMPLEMENTED = 501,
  HTTP_BAD_GATEWAY = 502,
  HTTP_SERVICE_UNAVAILABLE = 503
} HttpStatus;

/* ======================================================================== */
/* NetworkResponse Class                                                    */
/* ======================================================================== */

typedef struct NetworkResponse {
  /* Status and metadata */
  TDGetter(statusCode, int);
  TDGetter(statusText, const char*);
  TDGetter(isSuccess, int);
  TDGetter(isRedirect, int);
  TDGetter(isError, int);

  /* Headers */
  TDUnary(const char*, header, const char*);
  TDGetter(headerCount, size_t);

  /* Body */
  TDGetter(body, const char*);
  TDGetter(bodyLength, size_t);
  TDGetter(bodyAsString, String*);
  TDGetter(bodyAsJson, Json*);

  /* Utilities */
  TDGetter(contentType, const char*);
  TDGetter(contentLength, size_t);
  TDUnary(int, hasHeader, const char*);
  TDGetter(isJson, int);

  /* Memory management */
  TDNullary(free);
} NetworkResponse;

/* ======================================================================== */
/* NetworkRequest Class                                                     */
/* ======================================================================== */

typedef struct NetworkRequest {
  /* URL and method */
  TDGetter(url, const char*);
  TDSetter(setUrl, const char*);
  TDGetter(method, HttpMethod);
  TDSetter(setMethod, HttpMethod);

  /* Headers */
  TDUnary(const char*, header, const char*);
  TDDyadic(void, setHeader, const char*, const char*);
  TDUnary(void, removeHeader, const char*);

  /* Body */
  TDGetter(body, const char*);
  TDSetter(setBody, const char*);
  TDGetter(bodyLength, size_t);
  TDUnary(void, setBodyString, String*);
  TDUnary(void, setBodyJson, Json*);

  /* Connection settings */
  TDGetter(port, int);
  TDSetter(setPort, int);
  TDGetter(timeout, int);
  TDSetter(setTimeout, int);

  /* Send the request */
  TDGetter(send, NetworkResponse*);

  /* Memory management */
  TDNullary(free);
} NetworkRequest;

/* ======================================================================== */
/* Creation Functions                                                       */
/* ======================================================================== */

NetworkRequest* NetworkRequestMake(const char* url, HttpMethod method);
NetworkRequest* NetworkRequestMakeWithString(String* url, HttpMethod method);
NetworkResponse* NetworkResponseMake(int status_code, const char* status_text, const char* body);

#endif /* TRAMPOLINES_NETWORK_H */
