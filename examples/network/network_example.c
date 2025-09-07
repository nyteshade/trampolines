#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <trampoline.h>

#include "network_request.h"
#include "network_response.h"

int main(int argc, char **argv) {
  (void)argc;
  (void)argv;

  printf("Network Request/Response Example with Trampolines\n");
  printf("==================================================\n\n");

  NetworkRequest* request = NetworkRequestMake("http://httpbin.org/get", HTTP_GET);

  if (!request) {
    printf("Failed to create network request\n");
    return 1;
  }

  printf("Making GET request to: %s\n", request->url());

  request->setHeader("User-Agent", "TrampolineNetworkClient/1.0");
  request->setHeader("Accept", "application/json");
  request->setTimeout(10);

  printf("Sending request...\n");
  NetworkResponse* response = NULL; //request->send();

  if (response) {
    if (response->isSuccess()) {
      printf("Success! Status: %d %s\n",
        response->statusCode(),
        response->statusMessage() ? response->statusMessage() : "");

      printf("\nResponse Headers:\n");
      {
        const char** header_keys = response->allHeaderKeys();
        if (header_keys) {
          size_t i;
          for (i = 0; i < response->headerCount(); i++) {
            if (header_keys[i]) {
              printf("  %s: %s\n", header_keys[i], response->header(header_keys[i]));
            }
          }
        }
      }

      printf("\nResponse Body (%zu bytes):\n", response->bodyLength());
      if (response->body()) {
        printf("%s\n", response->body());
      }
    } else {
      printf("Request failed: %s\n",
        response->error() ? response->error() : "Unknown error");
    }

    response->free();
  } else {
    printf("Failed to get response\n");
  }

  printf("\n--- POST Request Example ---\n");

  if (0)
  {
    NetworkRequest* post_request = NetworkRequestMake("https://httpbin.org/post", HTTP_POST);
    if (post_request) {
      NetworkResponse* post_response;
      
      post_request->setHeader("Content-Type", "application/json");
      post_request->setBody("{\"message\":\"Hello from trampoline network client!\"}");

      printf("Sending POST request with body...\n");
      post_response = post_request->send();

      if (post_response) {
        if (post_response->isSuccess()) {
          printf("POST Success! Status: %d\n", post_response->statusCode());
          printf("Response excerpt: %.200s...\n", post_response->body());
        } else {
          printf("POST failed: %s\n", post_response->error());
        }
        post_response->free();
      }

      post_request->free();
    }
  }
  
  printf("\n--- SSL POST Request Example ---\n");

  {
    const char* apiKey = "--enter your api key here--";    
    NetworkRequest* post_request = NetworkRequestMake("https://api.anthropic.com/v1/messages", HTTP_POST);    
    
    if (post_request) {
      NetworkResponse* post_response;
      
      post_request->setHeader("Content-Type", "application/json");
      post_request->setHeader("x-api-key", apiKey);
      post_request->setHeader("anthropic-version", "2023-06-01");      
      
      post_request->setBody(
        "{"
          "\"model\": \"claude-opus-4-1-20250805\","
          "\"max_tokens\": 8192,"
          "\"messages\": ["
            " { \"role\": \"user\", \"content\": \"hello\" }"
          "]"
        "}"
      );

      printf("Sending POST request with body...\n");
      post_response = post_request->send();

      if (post_response) {
        if (post_response->isSuccess()) {
          printf("POST Success! Status: %d\n", post_response->statusCode());
          printf("Response excerpt: %.200s...\n", post_response->body());
        } else {
          printf("POST failed: %s\n", post_response->error());
        }
        post_response->free();
      }

      post_request->free();
    }
  }  

  request->free();

  return 0;
}
