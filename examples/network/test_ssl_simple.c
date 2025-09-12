/**
 * @file test_ssl_simple.c
 * @brief Simple test of SSL functionality
 */

#include <stdio.h>
#include <trampolines/network.h>

int main(void) {
    printf("Testing SSL support...\n");
    
    /* Try to create an HTTPS request */
    NetworkRequest* req = NetworkRequestMake("https://www.example.com", HTTP_GET);
    if (!req) {
        printf("Failed to create HTTPS request\n");
        return 1;
    }
    
    printf("Created HTTPS request:\n");
    printf("  URL: %s\n", req->url());
    printf("  Port: %d\n", req->port());
    
    printf("\nAttempting connection...\n");
    NetworkResponse* resp = req->send();
    
    if (resp) {
        printf("Got response!\n");
        printf("  Status: %d %s\n", resp->statusCode(), resp->statusText());
        printf("  Success: %s\n", resp->isSuccess() ? "Yes" : "No");
        
        if (resp->body()) {
            printf("  Body length: %zu\n", resp->bodyLength());
            printf("  First 100 chars: %.100s\n", resp->body());
        }
        
        resp->free();
    } else {
        printf("No response received\n");
    }
    
    req->free();
    
    printf("\nTest complete!\n");
    return 0;
}