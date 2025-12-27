/* C89-compliant JSON test */
#include <trampoline/classes/json.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    Json* obj;
    Json* item;
    char* json_str;

    printf("Testing JSON class with C89...\n");

    /* Create object */
    obj = JsonMakeObject();
    if (!obj) {
        printf("Failed to create object\n");
        return 1;
    }

    /* Add string */
    item = JsonMakeString("C89 Compatible!");
    obj->objectSet("message", item);
    item->free();

    /* Add number */
    item = JsonMakeNumber(89);
    obj->objectSet("standard", item);
    item->free();

    /* Add boolean (using int in C89) */
    item = JsonMakeBool(1);  /* true */
    obj->objectSet("compatible", item);
    item->free();

    /* Stringify */
    json_str = obj->stringify();
    printf("JSON: %s\n", json_str);
    free(json_str);

    /* Clean up */
    obj->free();

    printf("C89 test successful!\n");
    return 0;
}
