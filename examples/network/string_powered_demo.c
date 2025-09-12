/**
 * @file string_powered_demo.c
 * @brief Demo showing the power of String-enhanced network classes
 */

#include <stdio.h>
#include <trampolines/network.h>
#include <trampolines/string.h>

void demo_string_benefits() {
    printf("String-Powered Network API Demo\n");
    printf("================================\n\n");
    
    /* -------------------------------------------------------------------- */
    /* Example 1: Building complex URLs with String                         */
    /* -------------------------------------------------------------------- */
    printf("1. Building URLs with String:\n");
    printf("------------------------------\n");
    
    /* Old way (C strings) - error prone, manual memory management */
    char old_url[256];
    snprintf(old_url, sizeof(old_url), 
             "https://api.example.com/v1/users/%d?filter=%s&limit=%d",
             123, "active", 50);
    
    /* New way with String - clean, safe, chainable */
    String* base = StringMake("https://api.example.com");
    String* endpoint = StringMake("/v1/users/");
    endpoint->appendFormat("%d", 123);
    
    String* url = StringMake("");
    url->append(base->cStr())
       ->append(endpoint->cStr())
       ->append("?filter=active&limit=50");
    
    NetworkRequest* req = NetworkRequestMakeString(url, HTTP_GET);
    
    printf("Built URL: %s\n", req->url()->cStr());
    printf("  Scheme: %s\n", req->scheme()->cStr());
    printf("  Host: %s\n", req->host()->cStr());
    printf("  Path: %s\n", req->path()->cStr());
    printf("  Query: %s\n", req->query()->cStr());
    
    base->free();
    endpoint->free();
    url->free();
    req->free();
    
    /* -------------------------------------------------------------------- */
    /* Example 2: Dynamic header building with String                       */
    /* -------------------------------------------------------------------- */
    printf("\n2. Dynamic Headers with String:\n");
    printf("---------------------------------\n");
    
    NetworkRequest* req2 = NetworkRequestMake("https://api.example.com", HTTP_POST);
    
    /* Build complex authorization header */
    String* token = StringMake("eyJhbGciOiJIUzI1NiIs...");
    req2->setBearerToken(token);
    
    /* Build custom headers dynamically */
    String* client_info = StringMake("MyApp/");
    client_info->append("1.0.0");
    client_info->append(" (");
    client_info->append("Build 42");
    client_info->append(")");
    
    req2->setHeaderString(StringMake("X-Client-Info"), client_info);
    
    /* Add request ID */
    String* request_id = StringMake("req-");
    request_id->appendFormat("%d-%d", 12345, 67890);
    req2->setHeaderString(StringMake("X-Request-ID"), request_id);
    
    /* Print all headers as a single String */
    String* all_headers = req2->allHeaders();
    printf("Headers:\n%s", all_headers->cStr());
    
    token->free();
    client_info->free();
    request_id->free();
    all_headers->free();
    req2->free();
    
    /* -------------------------------------------------------------------- */
    /* Example 3: JSON body building with String                            */
    /* -------------------------------------------------------------------- */
    printf("\n3. JSON Body Building with String:\n");
    printf("------------------------------------\n");
    
    NetworkRequest* req3 = NetworkRequestMake("https://api.example.com/data", HTTP_POST);
    
    /* Build JSON progressively using String */
    String* json = StringMake("{\n");
    
    /* Add user object */
    json->append("  \"user\": {\n");
    json->appendFormat("    \"id\": %d,\n", 123);
    json->append("    \"name\": \"");
    
    /* Safely escape and add user input */
    String* username = StringMake("John \"The Dev\" O'Brien");
    username->replace("\"", "\\\"");  /* Escape quotes */
    json->append(username->cStr());
    json->append("\",\n");
    
    /* Add array of tags */
    json->append("    \"tags\": [");
    String* tags[] = { 
        StringMake("developer"), 
        StringMake("senior"), 
        StringMake("full-stack") 
    };
    for (int i = 0; i < 3; i++) {
        if (i > 0) json->append(", ");
        json->append("\"");
        json->append(tags[i]->cStr());
        json->append("\"");
        tags[i]->free();
    }
    json->append("]\n");
    
    json->append("  },\n");
    json->appendFormat("  \"timestamp\": %ld\n", (long)time(NULL));
    json->append("}");
    
    req3->setJsonBody(json);
    
    printf("JSON Body:\n%s\n", req3->body()->cStr());
    
    username->free();
    json->free();
    req3->free();
    
    /* -------------------------------------------------------------------- */
    /* Example 4: Form data with String                                     */
    /* -------------------------------------------------------------------- */
    printf("\n4. Form Data with String:\n");
    printf("--------------------------\n");
    
    NetworkRequest* req4 = NetworkRequestMake("https://example.com/form", HTTP_POST);
    
    /* Add form fields using String - automatic encoding */
    req4->addFormField(StringMake("name"), StringMake("John Doe"));
    req4->addFormField(StringMake("email"), StringMake("john@example.com"));
    
    /* Complex field with special characters */
    String* message = StringMake("Hello & welcome! Special chars: <>&\"'");
    req4->addFormField(StringMake("message"), message);
    
    printf("Form Body: %s\n", req4->body()->cStr());
    printf("Content-Type: %s\n", 
           req4->headerString(StringMake("Content-Type"))->cStr());
    
    message->free();
    req4->free();
    
    /* -------------------------------------------------------------------- */
    /* Example 5: Query parameter building                                  */
    /* -------------------------------------------------------------------- */
    printf("\n5. Query Parameters with String:\n");
    printf("----------------------------------\n");
    
    NetworkRequest* req5 = NetworkRequestMake("https://api.example.com/search", HTTP_GET);
    
    /* Add query parameters dynamically */
    req5->addQueryParam(StringMake("q"), StringMake("trampolines"));
    req5->addQueryParam(StringMake("category"), StringMake("programming"));
    req5->addQueryParam(StringMake("limit"), StringMake("10"));
    
    /* Complex query with spaces and special chars */
    String* complex_query = StringMake("C++ & trampolines");
    req5->addQueryParam(StringMake("tags"), complex_query);
    
    printf("Full URL: %s\n", req5->url()->cStr());
    printf("Query string: %s\n", req5->query()->cStr());
    
    complex_query->free();
    req5->free();
    
    /* -------------------------------------------------------------------- */
    /* Example 6: Response handling with String                             */
    /* -------------------------------------------------------------------- */
    printf("\n6. Response Handling with String:\n");
    printf("-----------------------------------\n");
    
    /* Mock response for demo */
    String* mock_response = StringMake(
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "X-Custom-Header: Some Value\r\n"
        "\r\n"
        "{\"status\":\"success\",\"data\":{\"id\":123}}"
    );
    
    NetworkResponse* resp = NetworkResponseFromRaw(mock_response);
    
    if (resp) {
        /* Status as String for easy manipulation */
        String* status = resp->statusText();
        printf("Status: %s (uppercase: %s)\n", 
               status->cStr(), 
               status->toUpperCase()->cStr());
        
        /* Headers as String */
        String* content_type = resp->headerString(StringMake("Content-Type"));
        if (content_type && content_type->contains("json")) {
            printf("Response is JSON\n");
        }
        
        /* Body as String for parsing */
        String* body = resp->body();
        if (body->contains("\"status\":\"success\"")) {
            printf("Request succeeded!\n");
            
            /* Extract JSON values using String methods */
            ssize_t id_pos = body->indexOf("\"id\":");
            if (id_pos >= 0) {
                String* id_part = body->substring(id_pos + 5, body->length());
                printf("Extracted ID portion: %s\n", id_part->cStr());
                id_part->free();
            }
        }
        
        /* Debug representation */
        String* debug = resp->toDebugString();
        printf("\nDebug output:\n%s\n", debug->cStr());
        
        status->free();
        if (content_type) content_type->free();
        body->free();
        debug->free();
        resp->free();
    }
    
    mock_response->free();
    
    /* -------------------------------------------------------------------- */
    /* Example 7: Request debugging with String                             */
    /* -------------------------------------------------------------------- */
    printf("\n7. Request Debugging with String:\n");
    printf("-----------------------------------\n");
    
    NetworkRequest* req7 = JsonPostRequest(
        StringMake("https://api.example.com/debug"),
        StringMake("{\"debug\":true}")
    );
    
    /* Get complete HTTP request as String for debugging */
    String* full_request = req7->buildRequest();
    printf("Full HTTP Request:\n");
    printf("==================\n");
    printf("%s\n", full_request->cStr());
    
    /* Get debug representation */
    String* debug_str = req7->toDebugString();
    printf("Debug Representation:\n");
    printf("=====================\n");
    printf("%s", debug_str->cStr());
    
    full_request->free();
    debug_str->free();
    req7->free();
    
    printf("\nDemo Complete!\n");
}

int main(void) {
    demo_string_benefits();
    return 0;
}