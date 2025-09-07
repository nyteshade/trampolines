/**
 * @file network_response.h
 * @brief NetworkResponse structure for HTTP responses using trampolines
 *
 * This file provides a NetworkResponse object that encapsulates HTTP response
 * data received from a NetworkRequest. It uses the trampoline pattern to
 * provide an object-oriented interface for accessing response data.
 *
 * @author Trampoline Network Example
 * @date 2025
 */

#ifndef NETWORK_RESPONSE_H
#define NETWORK_RESPONSE_H

#include <trampoline.h>
#include <stddef.h>

/**
 * @struct NetworkResponse
 * @brief HTTP response object with trampoline function pointers
 *
 * The NetworkResponse struct represents an HTTP response received from a
 * server. It provides methods to access the status code, headers, body,
 * and error information. All methods are implemented as trampoline functions.
 *
 * NetworkResponse objects are typically created by calling the send() method
 * on a NetworkRequest. They can also be created directly for error cases
 * using NetworkResponseMakeError().
 *
 * @example Checking response status
 * @code
 * NetworkResponse* resp = request->send();
 * if (resp) {
 *     if (resp->isSuccess()) {
 *         printf("Success: %d\n", resp->statusCode());
 *         printf("Body: %s\n", resp->body());
 *     } else {
 *         printf("Error: %s\n", resp->error());
 *     }
 *     resp->free();
 * }
 * @endcode
 *
 * @example Accessing response headers
 * @code
 * NetworkResponse* resp = request->send();
 * if (resp && resp->isSuccess()) {
 *     // Get specific header
 *     const char* contentType = resp->header("Content-Type");
 *     
 *     // Iterate all headers
 *     const char** keys = resp->allHeaderKeys();
 *     for (size_t i = 0; i < resp->headerCount(); i++) {
 *         printf("%s: %s\n", keys[i], resp->header(keys[i]));
 *     }
 *     resp->free();
 * }
 * @endcode
 */
typedef struct NetworkResponse {
  /**
   * @brief Get the HTTP status code
   * @return The status code (e.g., 200, 404, 500)
   * @note Returns 0 if the response is an error response
   */
  int (*statusCode)();
  
  /**
   * @brief Get the HTTP status message
   * @return The status message (e.g., "OK", "Not Found")
   * @note May return NULL if no message was provided
   */
  const char* (*statusMessage)();
  
  /**
   * @brief Get a response header value by key
   * @param key The header name (case-insensitive)
   * @return The header value or NULL if not found
   *
   * @example
   * @code
   * const char* contentType = resp->header("Content-Type");
   * const char* length = resp->header("Content-Length");
   * @endcode
   */
  const char* (*header)(const char* key);
  
  /**
   * @brief Get all header keys
   * @return Array of header keys (NULL-terminated)
   * @note The returned array is cached and owned by the response
   * @warning Do not free the returned array or its contents
   */
  const char** (*allHeaderKeys)();
  
  /**
   * @brief Get the number of headers
   * @return The count of response headers
   */
  size_t (*headerCount)();
  
  /**
   * @brief Get the response body
   * @return The body content or NULL if none
   * @note The body is null-terminated for convenience
   */
  const char* (*body)();
  
  /**
   * @brief Get the length of the response body
   * @return The body length in bytes (excluding null terminator)
   */
  size_t (*bodyLength)();
  
  /**
   * @brief Check if the response indicates success
   * @return 1 if status is 2xx and no error, 0 otherwise
   *
   * @note Success is defined as:
   *       - Status code between 200 and 299 (inclusive)
   *       - No error message set
   */
  int (*isSuccess)();
  
  /**
   * @brief Get the error message if any
   * @return Error description or NULL if no error
   * @note This is set for network/parsing errors, not HTTP errors
   *
   * @example
   * @code
   * if (!resp->isSuccess()) {
   *     const char* err = resp->error();
   *     if (err) {
   *         printf("Network error: %s\n", err);
   *     } else {
   *         printf("HTTP error: %d\n", resp->statusCode());
   *     }
   * }
   * @endcode
   */
  const char* (*error)();
  
  /**
   * @brief Free the NetworkResponse and all resources
   * @warning Do not use the object after calling this
   */
  void (*free)();
} NetworkResponse;

/**
 * @brief Create an empty NetworkResponse instance
 *
 * This function creates a NetworkResponse object.
 *
 * @return A new NetworkResponse object
 * @note The returned object must be freed with resp->free()
 *
 * @example
 * @code
 * NetworkResponse* resp = NetworkResponseMake();
 * resp->free();
 * @endcode
 */
NetworkResponse* NetworkResponseMake();

/**
 * @brief Create a NetworkResponse for an error condition
 *
 * This function creates a NetworkResponse object that represents an error
 * that occurred before receiving an HTTP response (e.g., connection failure,
 * DNS resolution failure, timeout).
 *
 * @param error The error message to store
 * @return A new NetworkResponse object with the error set
 * @note The returned object must be freed with resp->free()
 *
 * @example
 * @code
 * NetworkResponse* resp = NetworkResponseMakeError("Connection refused");
 * printf("Error: %s\n", resp->error());
 * resp->free();
 * @endcode
 */
NetworkResponse* NetworkResponseMakeError(const char* error);

#endif