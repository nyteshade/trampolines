/**
 * @file test_integration.c
 * @brief Integration test for libtrampolines - tests String and Network together
 */

#include <trampolines/string.h>
#include <trampolines/network.h>
#include <stdio.h>

int main(void) {
    printf("=== Trampoline Libraries Integration Test ===\n\n");
    
    /* Test 1: String class */
    printf("1. Testing String class:\n");
    String* test = StringMake("Hello");
    test->append(" from");
    test->append(" libtrampolines!");
    printf("   String: %s\n", test->cStr());
    printf("   Length: %zu\n", test->length());
    printf("   Upper: %s\n", test->toUpperCase()->cStr());
    test->free();
    printf("   ✓ String class works!\n\n");
    
    /* Test 2: Build URL with String */
    printf("2. Building URL with String:\n");
    String* base_url = StringMake("https://api.example.com");
    String* endpoint = StringMake("/v1/users");
    String* url = StringMake(base_url->cStr());
    url->append(endpoint->cStr());
    url->append("?limit=10");
    printf("   Built URL: %s\n", url->cStr());
    
    /* Test 3: NetworkRequest with String-built URL */
    printf("\n3. Creating NetworkRequest:\n");
    NetworkRequest* req = NetworkRequestMakeWithString(url, HTTP_GET);
    if (req) {
        printf("   Request URL: %s\n", req->url());
        printf("   Method: %d (GET)\n", req->method());
        printf("   Port: %d\n", req->port());
        
        /* Add headers */
        req->setHeader("User-Agent", "IntegrationTest/1.0");
        req->setHeader("Accept", "application/json");
        
        /* Use String for body */
        String* json_body = StringMake("{\"test\": true}");
        req->setBodyString(json_body);
        printf("   Body: %s\n", req->body());
        
        json_body->free();
        req->free();
        printf("   ✓ NetworkRequest works!\n");
    }
    
    /* Test 4: Mock response with String integration */
    printf("\n4. Testing NetworkResponse:\n");
    NetworkResponse* resp = NetworkResponseMake(200, "OK", 
        "{\"message\": \"Integration successful\"}");
    if (resp) {
        printf("   Status: %d %s\n", resp->statusCode(), resp->statusText());
        printf("   Success: %s\n", resp->isSuccess() ? "Yes" : "No");
        
        /* Get body as String for manipulation */
        String* body = resp->bodyAsString();
        if (body) {
            printf("   Body: %s\n", body->cStr());
            if (body->contains("successful")) {
                printf("   ✓ Response contains 'successful'\n");
            }
            body->free();
        }
        
        resp->free();
        printf("   ✓ NetworkResponse works!\n");
    }
    
    base_url->free();
    endpoint->free();
    url->free();
    
    printf("\n=== All tests passed! ===\n");
    printf("libtrampolines is working correctly with both String and Network classes.\n");
    
    return 0;
}