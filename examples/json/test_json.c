#include <trampolines/json.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    Json* obj;
    Json* item;
    char* json_str;
    
    printf("Testing JSON class...\n");
    
    /* Create a simple object */
    obj = JsonMakeObject();
    if (!obj) {
        printf("Failed to create object\n");
        return 1;
    }
    
    /* Add a string */
    item = JsonMakeString("Hello, JSON!");
    obj->objectSet("message", item);
    item->free();
    
    /* Add a number */
    item = JsonMakeNumber(42);
    obj->objectSet("answer", item);
    item->free();
    
    /* Add a boolean */
    item = JsonMakeBool(true);
    obj->objectSet("success", item);
    item->free();
    
    /* Stringify */
    json_str = obj->stringify();
    printf("JSON: %s\n", json_str);
    free(json_str);
    
    /* Pretty print */
    json_str = obj->prettyPrint(2);
    printf("\nPretty JSON:\n%s\n", json_str);
    free(json_str);
    
    /* Test parsing */
    printf("\nTesting parse...\n");
    Json* parsed = JsonParse("{\"test\":123,\"hello\":\"world\"}");
    if (parsed) {
        json_str = parsed->stringify();
        printf("Parsed: %s\n", json_str);
        free(json_str);
        parsed->free();
    } else {
        printf("Parse failed\n");
    }
    
    obj->free();
    
    printf("\nTest complete!\n");
    return 0;
}