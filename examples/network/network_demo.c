/**
 * @file network_demo.c
 * @brief Demo of NetworkRequest and NetworkResponse using libtrampolines
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <trampolines/network.h>
#include <trampolines/string.h>

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    
    printf("Network Request/Response Demo\n");
    printf("=============================\n\n");
    
    /* Example 1: Simple GET request */
    printf("Example 1: Simple GET Request\n");
    printf("------------------------------\n");
    
    NetworkRequest* request = NetworkRequestMake("http://httpbin.org/get", HTTP_GET);
    if (!request) {
        printf("Failed to create network request\n");
        return 1;
    }
    
    printf("URL: %s\n", request->url());
    printf("Method: %d (GET)\n", request->method());
    
    /* Set headers */
    request->setHeader("User-Agent", "TrampolineNetworkClient/1.0");
    request->setHeader("Accept", "application/json");
    request->setTimeout(10);
    
    printf("Headers set:\n");
    printf("  User-Agent: %s\n", request->header("User-Agent"));
    printf("  Accept: %s\n", request->header("Accept"));
    printf("  Timeout: %d seconds\n", request->timeout());
    
    /* Note: actual send() would require network connectivity */
    printf("\nNote: Actual network request disabled for demo\n");
    
    /* Example 2: POST request with body */
    printf("\nExample 2: POST Request with Body\n");
    printf("----------------------------------\n");
    
    NetworkRequest* post_request = NetworkRequestMake("http://httpbin.org/post", HTTP_POST);
    if (post_request) {
        post_request->setHeader("Content-Type", "application/json");
        
        /* Using String class to build JSON */
        String* json_body = StringMake("{");
        json_body->append("\"message\": \"Hello from trampolines\",");
        json_body->append("\"version\": 1.0");
        json_body->append("}");
        
        /* Set body using String */
        post_request->setBodyString(json_body);
        
        printf("POST URL: %s\n", post_request->url());
        printf("Content-Type: %s\n", post_request->header("Content-Type"));
        printf("Body: %s\n", post_request->body());
        printf("Body Length: %zu\n", post_request->bodyLength());
        
        json_body->free();
        post_request->free();
    }
    
    /* Example 3: Mock Response handling */
    printf("\nExample 3: Response Handling (Mock)\n");
    printf("------------------------------------\n");
    
    /* Create a mock response */
    const char* mock_response = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: 27\r\n"
        "Server: httpbin\r\n"
        "\r\n"
        "{\"status\": \"success\"}";
    
    NetworkResponse* response = NetworkResponseMake(200, "OK", mock_response);
    if (response) {
        printf("Status Code: %d\n", response->statusCode());
        printf("Status Text: %s\n", response->statusText());
        printf("Is Success: %s\n", response->isSuccess() ? "true" : "false");
        printf("Is Error: %s\n", response->isError() ? "true" : "false");
        
        printf("\nHeaders:\n");
        printf("  Content-Type: %s\n", response->contentType());
        printf("  Content-Length: %zu\n", response->contentLength());
        printf("  Server: %s\n", response->header("Server"));
        
        printf("\nBody: %s\n", response->body());
        
        /* Get body as String for manipulation */
        String* body_str = response->bodyAsString();
        if (body_str) {
            printf("Body as String (length=%zu): %s\n", 
                   body_str->length(), body_str->cStr());
            body_str->free();
        }
        
        response->free();
    }
    
    /* Example 4: Error Response */
    printf("\nExample 4: Error Response (Mock)\n");
    printf("---------------------------------\n");
    
    NetworkResponse* error_response = NetworkResponseMake(404, "Not Found", 
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: text/plain\r\n"
        "\r\n"
        "Resource not found");
    
    if (error_response) {
        printf("Status: %d %s\n", 
               error_response->statusCode(), 
               error_response->statusText());
        printf("Is Error: %s\n", error_response->isError() ? "true" : "false");
        printf("Body: %s\n", error_response->body());
        
        error_response->free();
    }
    
    request->free();
    
    printf("\nDemo completed successfully!\n");
    return 0;
}