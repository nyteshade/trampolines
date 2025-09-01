/**
 * @file network_request.h
 * @brief NetworkRequest structure for making HTTP requests using trampolines
 *
 * This file provides a NetworkRequest object that encapsulates HTTP request
 * functionality using the trampoline pattern. It supports common HTTP methods,
 * headers, request bodies, and can send requests over sockets.
 *
 * @author Trampoline Network Example
 * @date 2025
 */

#ifndef NETWORK_REQUEST_H
#define NETWORK_REQUEST_H

#include <trampoline.h>
#include <stddef.h>

/**
 * @enum HttpMethod
 * @brief HTTP methods supported by NetworkRequest
 */
typedef enum HttpMethod {
  HTTP_GET,     /**< GET method for retrieving resources */
  HTTP_POST,    /**< POST method for submitting data */
  HTTP_PUT,     /**< PUT method for updating resources */
  HTTP_DELETE,  /**< DELETE method for removing resources */
  HTTP_PATCH,   /**< PATCH method for partial updates */
  HTTP_HEAD,    /**< HEAD method for retrieving headers only */
  HTTP_OPTIONS  /**< OPTIONS method for checking allowed methods */
} HttpMethod;

/**
 * @struct NetworkRequest
 * @brief HTTP request object with trampoline function pointers
 *
 * The NetworkRequest struct provides an object-oriented interface for
 * building and sending HTTP requests. All methods are implemented as
 * trampoline functions that maintain context through the self pointer.
 *
 * @note Only HTTP is currently supported. HTTPS requires SSL/TLS libraries.
 * @note The maximum response size is limited to 64KB.
 *
 * @example Basic GET request
 * @code
 * NetworkRequest* req = NetworkRequestMake("http://example.com", HTTP_GET);
 * req->setHeader("User-Agent", "MyApp/1.0");
 * NetworkResponse* resp = req->send();
 * if (resp->isSuccess()) {
 *     printf("Response: %s\n", resp->body());
 * }
 * resp->free();
 * req->free();
 * @endcode
 *
 * @example POST request with JSON body
 * @code
 * NetworkRequest* req = NetworkRequestMake("http://api.example.com/data",
 *                                          HTTP_POST);
 * req->setHeader("Content-Type", "application/json");
 * req->setBody("{\"key\":\"value\"}");
 * NetworkResponse* resp = req->send();
 * // Process response...
 * resp->free();
 * req->free();
 * @endcode
 */
typedef struct NetworkRequest {
  /**
   * @brief Get the request URL
   * @return The current URL string (read-only)
   */
  const char* (*url)();
  
  /**
   * @brief Set the request URL
   * @param url The new URL (will be copied)
   */
  void (*setUrl)(const char* url);
  
  /**
   * @brief Get the HTTP method
   * @return The current HttpMethod enum value
   */
  HttpMethod (*method)();
  
  /**
   * @brief Set the HTTP method
   * @param method The HttpMethod to use
   */
  void (*setMethod)(HttpMethod method);
  
  /**
   * @brief Get a header value by key
   * @param key The header name (case-insensitive)
   * @return The header value or NULL if not found
   */
  const char* (*header)(const char* key);
  
  /**
   * @brief Set or update a header
   * @param key The header name
   * @param value The header value
   * @note If the header exists, it will be updated
   */
  void (*setHeader)(const char* key, const char* value);
  
  /**
   * @brief Remove a header by key
   * @param key The header name to remove
   */
  void (*removeHeader)(const char* key);
  
  /**
   * @brief Get the request body
   * @return The body content or NULL if none
   */
  const char* (*body)();
  
  /**
   * @brief Set the request body
   * @param body The body content (will be copied)
   * @note Content-Length header is automatically set when sending
   */
  void (*setBody)(const char* body);
  
  /**
   * @brief Get the length of the request body
   * @return The body length in bytes
   */
  size_t (*bodyLength)();
  
  /**
   * @brief Get the port number
   * @return The port (default: 80)
   */
  int (*port)();
  
  /**
   * @brief Set the port number
   * @param port The port to connect to
   */
  void (*setPort)(int port);
  
  /**
   * @brief Get the timeout in seconds
   * @return The timeout value (default: 30)
   */
  int (*timeout)();
  
  /**
   * @brief Set the timeout in seconds
   * @param seconds The timeout for socket operations
   */
  void (*setTimeout)(int seconds);
  
  /**
   * @brief Send the HTTP request and get a response
   * @return NetworkResponse object or NULL on failure
   * @note The caller owns the returned response and must free it
   * @warning This blocks until the response is received or timeout
   */
  struct NetworkResponse* (*send)();
  
  /**
   * @brief Free the NetworkRequest and all resources
   * @warning Do not use the object after calling this
   */
  void (*free)();
} NetworkRequest;

/**
 * @brief Create a new NetworkRequest object
 *
 * @param url The request URL (e.g., "http://example.com/path")
 * @param method The HTTP method to use
 * @return A new NetworkRequest object or NULL on failure
 *
 * @note Default values: port=80, timeout=30 seconds
 * @note The returned object must be freed with req->free()
 *
 * @example
 * @code
 * NetworkRequest* req = NetworkRequestMake("http://example.com", HTTP_GET);
 * if (req) {
 *     // Use the request...
 *     req->free();
 * }
 * @endcode
 */
NetworkRequest* NetworkRequestMake(const char* url, HttpMethod method);

#endif