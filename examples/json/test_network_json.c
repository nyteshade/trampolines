/**
 * Test program demonstrating Network classes with JSON integration
 */

#include <trampolines/network.h>
#include <trampolines/json.h>
#include <trampolines/string.h>
#include <stdio.h>
#include <stdlib.h>

void test_json_request(void) {
    NetworkRequest* request;
    Json* body;
    Json* item;
    
    printf("\n=== Testing JSON Request Body ===\n");
    
    /* Create a request */
    request = NetworkRequestMake("https://api.example.com/users", HTTP_POST);
    
    /* Create JSON body */
    body = JsonMakeObject();
    
    item = JsonMakeString("John Doe");
    body->objectSet("name", item);
    item->free();
    
    item = JsonMakeString("john@example.com");
    body->objectSet("email", item);
    item->free();
    
    item = JsonMakeNumber(25);
    body->objectSet("age", item);
    item->free();
    
    /* Set JSON as request body - this should also set Content-Type */
    request->setBodyJson(body);
    
    /* Check the body and headers */
    printf("Request URL: %s\n", request->url());
    printf("Request Body: %s\n", request->body());
    printf("Content-Type: %s\n", request->header("Content-Type"));
    printf("Body Length: %zu\n", request->bodyLength());
    
    /* Clean up */
    body->free();
    request->free();
}

void test_json_response(void) {
    NetworkResponse* response;
    Json* json;
    Json* item;
    char* json_str;
    
    printf("\n=== Testing JSON Response Parsing ===\n");
    
    /* Simulate a JSON response */
    const char* json_body = "{\"status\":\"success\",\"data\":{\"id\":123,\"message\":\"User created\"},\"timestamp\":1234567890}";
    
    response = NetworkResponseMake(200, "OK", json_body);
    
    /* Check if response is JSON */
    /* Note: This would work better with actual Content-Type header */
    printf("Response Status: %d %s\n", response->statusCode(), response->statusText());
    printf("Response Body: %s\n", response->body());
    
    /* Parse response as JSON */
    json = response->bodyAsJson();
    if (json) {
        printf("Successfully parsed response as JSON\n");
        
        /* Pretty print the JSON */
        json_str = json->prettyPrint(2);
        printf("Pretty Response:\n%s\n", json_str);
        free(json_str);
        
        /* Access specific fields */
        item = json->objectGet("status");
        if (item && item->isString()) {
            printf("Status field: %s\n", item->getString());
        }
        if (item) item->free();
        
        item = json->objectGet("data");
        if (item && item->isObject()) {
            Json* id = item->objectGet("id");
            if (id && id->isNumber()) {
                printf("User ID: %.0f\n", id->getNumber());
            }
            if (id) id->free();
            
            Json* msg = item->objectGet("message");
            if (msg && msg->isString()) {
                printf("Message: %s\n", msg->getString());
            }
            if (msg) msg->free();
        }
        if (item) item->free();
        
        json->free();
    } else {
        printf("Failed to parse response as JSON\n");
    }
    
    response->free();
}

void test_api_simulation(void) {
    NetworkRequest* request;
    Json* req_body;
    Json* user;
    Json* preferences;
    Json* item;
    char* body_str;
    
    printf("\n=== Simulating REST API with JSON ===\n");
    
    /* Create a complex request */
    request = NetworkRequestMake("https://api.example.com/v1/users/update", HTTP_PUT);
    
    /* Build complex JSON request body */
    req_body = JsonMakeObject();
    
    /* User object */
    user = JsonMakeObject();
    item = JsonMakeNumber(12345);
    user->objectSet("id", item);
    item->free();
    
    item = JsonMakeString("alice@example.com");
    user->objectSet("email", item);
    item->free();
    
    /* Preferences object */
    preferences = JsonMakeObject();
    item = JsonMakeString("dark");
    preferences->objectSet("theme", item);
    item->free();
    
    item = JsonMakeBool(true);
    preferences->objectSet("notifications", item);
    item->free();
    
    user->objectSet("preferences", preferences);
    preferences->free();
    
    req_body->objectSet("user", user);
    user->free();
    
    /* Timestamp */
    item = JsonMakeNumber(1234567890);
    req_body->objectSet("timestamp", item);
    item->free();
    
    /* Set the JSON body */
    request->setBodyJson(req_body);
    
    /* Also set additional headers */
    request->setHeader("Authorization", "Bearer token123");
    request->setHeader("X-API-Version", "1.0");
    
    /* Display the request */
    printf("Request Method: PUT\n");
    printf("Request URL: %s\n", request->url());
    printf("Request Headers:\n");
    printf("  Content-Type: %s\n", request->header("Content-Type"));
    printf("  Authorization: %s\n", request->header("Authorization"));
    printf("  X-API-Version: %s\n", request->header("X-API-Version"));
    
    body_str = req_body->prettyPrint(2);
    printf("Request Body (formatted):\n%s\n", body_str);
    free(body_str);
    
    /* In a real scenario, request->send() would send this and return a response */
    printf("\n[In real usage, request->send() would send this to the server]\n");
    
    /* Clean up */
    req_body->free();
    request->free();
}

int main(void) {
    printf("=== Network JSON Integration Test ===\n");
    
    test_json_request();
    test_json_response();
    test_api_simulation();
    
    printf("\n=== Test Complete ===\n");
    return 0;
}