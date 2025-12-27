/**
 * @file network_ssl_demo.c
 * @brief Demo of NetworkRequest with SSL/HTTPS support
 */

#include <stdio.h>
#include <string.h>
#include <trampolines/network.h>
#include <trampolines/string.h>

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    printf("Network SSL/HTTPS Demo\n");
    printf("======================\n\n");

    /* Example 1: HTTPS Request */
    printf("Example 1: HTTPS GET Request\n");
    printf("-----------------------------\n");

    NetworkRequest* https_req = NetworkRequestMake("https://httpbin.org/get", HTTP_GET);
    if (!https_req) {
        printf("Failed to create HTTPS request\n");
        return 1;
    }

    printf("URL: %s\n", https_req->url());
    printf("Method: GET\n");
    printf("Default port: %d\n", https_req->port());

    /* Set custom headers */
    https_req->setHeader("User-Agent", "TrampolineSSL/1.0");
    https_req->setHeader("Accept", "application/json");
    https_req->setTimeout(10);

    printf("\nSending HTTPS request...\n");
    NetworkResponse* response = https_req->send();

    if (response) {
        printf("Response received!\n");
        printf("  Status: %d %s\n", response->statusCode(), response->statusText());
        printf("  Success: %s\n", response->isSuccess() ? "Yes" : "No");

        if (response->isSuccess()) {
            printf("  Content-Type: %s\n", response->contentType());
            printf("  Body length: %zu\n", response->bodyLength());

            /* Show first 200 chars of response */
            const char* body = response->body();
            if (body) {
                printf("  Body preview: %.200s%s\n",
                       body,
                       strlen(body) > 200 ? "..." : "");
            }
        } else {
            printf("  Error body: %s\n", response->body());
        }

        response->free();
    } else {
        printf("Failed to get response\n");
    }

    https_req->free();

    /* Example 2: HTTPS POST with JSON */
    printf("\nExample 2: HTTPS POST with JSON\n");
    printf("--------------------------------\n");

    NetworkRequest* post_req = NetworkRequestMake("https://httpbin.org/post", HTTP_POST);
    if (post_req) {
        /* Build JSON using String class */
        String* json = StringMake("{\n");
        json->append("  \"message\": \"Hello from SSL-enabled trampolines\",\n");
        json->append("  \"timestamp\": 1234567890,\n");
        json->append("  \"ssl\": true\n");
        json->append("}");

        post_req->setHeader("Content-Type", "application/json");
        post_req->setBodyString(json);

        printf("Sending POST to: %s\n", post_req->url());
        printf("Body:\n%s\n", json->cStr());

        NetworkResponse* post_resp = post_req->send();
        if (post_resp) {
            printf("\nResponse Status: %d %s\n",
                   post_resp->statusCode(),
                   post_resp->statusText());

            if (post_resp->isSuccess()) {
                /* httpbin.org echoes back our POST data */
                String* resp_body = post_resp->bodyAsString();
                if (resp_body) {
                    printf("Response preview: %.300s%s\n",
                           resp_body->cStr(),
                           resp_body->length() > 300 ? "..." : "");
                    resp_body->free();
                }
            }

            post_resp->free();
        } else {
            printf("POST request failed\n");
        }

        json->free();
        post_req->free();
    }

    /* Example 3: Test different ports */
    printf("\nExample 3: Custom Port Testing\n");
    printf("-------------------------------\n");

    NetworkRequest* custom_req = NetworkRequestMake("https://httpbin.org:443/status/200", HTTP_GET);
    if (custom_req) {
        printf("URL with explicit port: %s\n", custom_req->url());
        printf("Parsed port: %d\n", custom_req->port());

        /* Test changing port */
        custom_req->setPort(8443);
        printf("After setPort(8443): %d\n", custom_req->port());

        custom_req->free();
    }

    /* Example 4: HTTP (non-SSL) request for comparison */
    printf("\nExample 4: Plain HTTP Request\n");
    printf("------------------------------\n");

    NetworkRequest* http_req = NetworkRequestMake("http://httpbin.org/get", HTTP_GET);
    if (http_req) {
        printf("URL: %s\n", http_req->url());
        printf("Port: %d (should be 80)\n", http_req->port());

        NetworkResponse* http_resp = http_req->send();
        if (http_resp) {
            printf("HTTP Response: %d %s\n",
                   http_resp->statusCode(),
                   http_resp->statusText());
            http_resp->free();
        }

        http_req->free();
    }

    printf("\nSSL Demo completed!\n");

    return 0;
}
