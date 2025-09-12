/**
 * @file network_enhanced.h
 * @brief Enhanced Network classes leveraging String class throughout
 */

#ifndef TRAMPOLINES_NETWORK_ENHANCED_H
#define TRAMPOLINES_NETWORK_ENHANCED_H

#include <trampoline.h>
#include <trampolines/string.h>
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
/* NetworkResponse Class - Enhanced with String                             */
/* ======================================================================== */

typedef struct NetworkResponse {
  /* Status and metadata */
  TDGetter(statusCode, int);
  TDGetter(statusText, String*);           /* Returns as String for manipulation */
  TDGetter(statusTextRaw, const char*);    /* Raw C string version */
  TDGetter(isSuccess, int);
  TDGetter(isRedirect, int);
  TDGetter(isError, int);
  
  /* Headers - enhanced with String */
  TDUnary(String*, header, const char*);              /* Get header as String */
  TDUnary(String*, headerString, String*);            /* Get header using String key */
  TDUnary(const char*, headerRaw, const char*);       /* Raw C string version */
  TDGetter(headerCount, size_t);
  TDUnary(String*, headerAt, size_t);                 /* Get header at index as "Key: Value" */
  TDGetter(allHeaders, String*);                      /* All headers as single String */
  TDGetter(allHeadersArray, String**);                /* Array of String headers */
  
  /* Body - enhanced with String */
  TDGetter(body, String*);                  /* Body as String (primary) */
  TDGetter(bodyRaw, const char*);          /* Raw C string version */
  TDGetter(bodyLength, size_t);
  
  /* JSON helpers using String */
  TDUnary(String*, jsonValue, const char*);           /* Extract JSON value by key */
  TDUnary(String*, jsonValueString, String*);         /* Extract using String key */
  TDGetter(isJson, int);                              /* Check if Content-Type is JSON */
  
  /* Utilities */
  TDGetter(contentType, String*);
  TDGetter(contentLength, size_t);
  TDUnary(int, hasHeader, const char*);
  TDUnary(int, hasHeaderString, String*);
  
  /* Conversion */
  TDGetter(toString, String*);              /* Full response as String */
  TDGetter(toDebugString, String*);        /* Debug representation */
  
  /* Memory management */
  TDNullary(free);
} NetworkResponse;

/* ======================================================================== */
/* NetworkRequest Class - Enhanced with String                              */
/* ======================================================================== */

typedef struct NetworkRequest {
  /* URL and method - enhanced with String */
  TDGetter(url, String*);                   /* URL as String (primary) */
  TDGetter(urlRaw, const char*);           /* Raw C string version */
  TDSetter(setUrl, const char*);
  TDUnary(void, setUrlString, String*);    /* Set URL from String */
  TDGetter(method, HttpMethod);
  TDSetter(setMethod, HttpMethod);
  TDGetter(methodString, String*);         /* Method as String ("GET", "POST", etc) */
  
  /* URL components as String */
  TDGetter(scheme, String*);               /* "http" or "https" */
  TDGetter(host, String*);                 /* "example.com" */
  TDGetter(path, String*);                 /* "/api/v1/users" */
  TDGetter(query, String*);                /* "id=123&name=test" */
  TDGetter(fullPath, String*);             /* Path + query combined */
  
  /* Headers - enhanced with String */
  TDUnary(String*, header, const char*);              /* Get header as String */
  TDUnary(String*, headerString, String*);            /* Get using String key */
  TDDyadic(void, setHeader, const char*, const char*);
  TDDyadic(void, setHeaderString, String*, String*);  /* Set using String objects */
  TDUnary(void, removeHeader, const char*);
  TDUnary(void, removeHeaderString, String*);
  TDGetter(allHeaders, String*);                      /* All headers formatted */
  
  /* Header builders using String */
  TDUnary(void, setAuthorization, String*);           /* Set Authorization header */
  TDUnary(void, setBearerToken, String*);             /* Set Bearer token */
  TDDyadic(void, setBasicAuth, String*, String*);     /* Username, password */
  TDUnary(void, setUserAgent, String*);
  TDUnary(void, setContentType, String*);
  TDUnary(void, addCookie, String*);                  /* Add to Cookie header */
  
  /* Body - enhanced with String */
  TDGetter(body, String*);                  /* Body as String (primary) */
  TDGetter(bodyRaw, const char*);          /* Raw C string version */
  TDSetter(setBody, const char*);
  TDUnary(void, setBodyString, String*);
  TDGetter(bodyLength, size_t);
  
  /* JSON body builders using String */
  TDUnary(void, setJsonBody, String*);                /* Set body and Content-Type */
  TDDyadic(void, addJsonField, String*, String*);     /* Add field to JSON body */
  TDTriadic(void, addJsonValue, String*, const char*, size_t); /* Add any type */
  
  /* Form data using String */
  TDDyadic(void, addFormField, String*, String*);     /* Add form field */
  TDUnary(void, setFormBody, String*);                /* Set URL-encoded form */
  
  /* Query parameters using String */
  TDDyadic(void, addQueryParam, String*, String*);    /* Add to query string */
  TDUnary(void, setQueryString, String*);             /* Set entire query */
  
  /* Connection settings */
  TDGetter(port, int);
  TDSetter(setPort, int);
  TDGetter(timeout, int);
  TDSetter(setTimeout, int);
  TDGetter(followRedirects, int);
  TDSetter(setFollowRedirects, int);
  
  /* Request building */
  TDGetter(buildRequest, String*);          /* Build full HTTP request as String */
  TDGetter(toDebugString, String*);        /* Debug representation */
  
  /* Send the request */
  TDGetter(send, NetworkResponse*);
  
  /* Memory management */
  TDNullary(free);
} NetworkRequest;

/* ======================================================================== */
/* URL Builder Class using String                                           */
/* ======================================================================== */

typedef struct UrlBuilder {
  TDSetter(setScheme, String*);
  TDSetter(setHost, String*);
  TDSetter(setPort, int);
  TDSetter(setPath, String*);
  TDDyadic(void, addPathSegment, String*, int);       /* Segment, encode? */
  TDDyadic(void, addQueryParam, String*, String*);
  TDSetter(setFragment, String*);
  TDGetter(build, String*);                           /* Build complete URL */
  TDGetter(toString, String*);                        /* Alias for build */
  TDNullary(free);
} UrlBuilder;

/* ======================================================================== */
/* Creation Functions - Enhanced                                            */
/* ======================================================================== */

/* NetworkRequest creators */
NetworkRequest* NetworkRequestMake(const char* url, HttpMethod method);
NetworkRequest* NetworkRequestMakeString(String* url, HttpMethod method);
NetworkRequest* NetworkRequestMakeEmpty(void);  /* Create without URL */

/* NetworkResponse creators (for testing/mocking) */
NetworkResponse* NetworkResponseMake(int status_code, const char* status_text, const char* body);
NetworkResponse* NetworkResponseMakeString(int status_code, String* status_text, String* body);
NetworkResponse* NetworkResponseFromRaw(String* raw_http_response);

/* URL Builder */
UrlBuilder* UrlBuilderMake(void);
UrlBuilder* UrlBuilderMakeFrom(String* base_url);

/* ======================================================================== */
/* Convenience Functions using String                                       */
/* ======================================================================== */

/* Quick request builders */
NetworkRequest* GetRequest(String* url);
NetworkRequest* PostRequest(String* url, String* body);
NetworkRequest* PutRequest(String* url, String* body);
NetworkRequest* DeleteRequest(String* url);

/* Quick JSON requests */
NetworkRequest* JsonGetRequest(String* url);
NetworkRequest* JsonPostRequest(String* url, String* json_body);
NetworkRequest* JsonPutRequest(String* url, String* json_body);

/* URL utilities using String */
String* UrlEncode(String* input);
String* UrlDecode(String* input);
String* BuildQueryString(String** keys, String** values, size_t count);
int ParseQueryString(String* query, String*** keys, String*** values);

/* Base64 for authentication */
String* Base64Encode(String* input);
String* Base64Decode(String* input);

#endif /* TRAMPOLINES_NETWORK_ENHANCED_H */