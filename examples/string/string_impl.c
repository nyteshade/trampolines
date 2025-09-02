/**
 * @file string_impl.c
 * @brief Implementation of String with comprehensive string manipulation using trampolines
 */

#include "string.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <limits.h>

/* ======================================================================== */
/* Private String Structure                                                 */
/* ======================================================================== */

typedef struct StringPrivate {
    String public;          /* Public interface MUST be first */
    char* data;            /* String data buffer */
    size_t length;         /* Current string length (excluding null) */
    size_t capacity;       /* Allocated buffer size */
} StringPrivate;

/* ======================================================================== */
/* Utility Functions                                                        */
/* ======================================================================== */

static bool string_ensure_capacity(StringPrivate* priv, size_t required) {
    size_t new_capacity;
    char* new_data;
    
    if (!priv) return false;
    
    if (required <= priv->capacity) return true;
    
    /* Double capacity until sufficient */
    new_capacity = priv->capacity * 2;
    while (new_capacity < required) {
        new_capacity *= 2;
    }
    
    new_data = realloc(priv->data, new_capacity);
    if (!new_data) return false;
    
    priv->data = new_data;
    priv->capacity = new_capacity;
    return true;
}

static char* string_strdup(const char* str) {
    size_t len;
    char* copy;
    
    if (!str) return NULL;
    len = strlen(str);
    copy = malloc(len + 1);
    if (copy) {
        memcpy(copy, str, len + 1);
    }
    return copy;
}

static bool char_is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
}

/* ======================================================================== */
/* Core String Access Functions                                             */
/* ======================================================================== */

const char* string_c_str(String* self) {
    StringPrivate* priv = (StringPrivate*)self;
    return priv->data ? priv->data : "";
}

size_t string_length(String* self) {
    StringPrivate* priv = (StringPrivate*)self;
    return priv->length;
}

size_t string_capacity(String* self) {
    StringPrivate* priv = (StringPrivate*)self;
    return priv->capacity;
}

bool string_is_empty(String* self) {
    StringPrivate* priv = (StringPrivate*)self;
    return priv->length == 0;
}

char string_char_at(String* self, size_t index) {
    StringPrivate* priv = (StringPrivate*)self;
    if (index >= priv->length) return '\0';
    return priv->data[index];
}

/* ======================================================================== */
/* String Modification Functions                                            */
/* ======================================================================== */

bool string_append(String* self, const char* str) {
    StringPrivate* priv = (StringPrivate*)self;
    size_t add_len;
    size_t new_len;
    
    if (!str || !*str) return true;
    
    add_len = strlen(str);
    new_len = priv->length + add_len;
    
    if (!string_ensure_capacity(priv, new_len + 1)) return false;
    
    memcpy(priv->data + priv->length, str, add_len + 1);
    priv->length = new_len;
    return true;
}

bool string_append_char(String* self, char ch) {
    StringPrivate* priv = (StringPrivate*)self;
    
    if (!string_ensure_capacity(priv, priv->length + 2)) return false;
    
    priv->data[priv->length++] = ch;
    priv->data[priv->length] = '\0';
    return true;
}

bool string_append_format(String* self, const char* format, ...) {
    StringPrivate* priv = (StringPrivate*)self;
    va_list args;
    int required;
    size_t new_len;
    
    if (!format) return false;
    
    /* First pass to determine size */
    va_start(args, format);
    required = vsnprintf(NULL, 0, format, args);
    va_end(args);
    
    if (required < 0) {
        return false;
    }
    
    new_len = priv->length + (size_t)required;
    if (!string_ensure_capacity(priv, new_len + 1)) {
        return false;
    }
    
    /* Second pass to actually format */
    va_start(args, format);
    vsnprintf(priv->data + priv->length, required + 1, format, args);
    va_end(args);
    
    priv->length = new_len;
    return true;
}

bool string_prepend(String* self, const char* str) {
    StringPrivate* priv = (StringPrivate*)self;
    size_t add_len;
    size_t new_len;
    
    if (!str || !*str) return true;
    
    add_len = strlen(str);
    new_len = priv->length + add_len;
    
    if (!string_ensure_capacity(priv, new_len + 1)) return false;
    
    /* Move existing content forward */
    memmove(priv->data + add_len, priv->data, priv->length + 1);
    /* Copy new content to beginning */
    memcpy(priv->data, str, add_len);
    
    priv->length = new_len;
    return true;
}

bool string_insert(String* self, size_t index, const char* str) {
    StringPrivate* priv = (StringPrivate*)self;
    size_t add_len;
    size_t new_len;
    
    if (!str || !*str) return true;
    if (index > priv->length) return false;
    
    if (index == 0) return string_prepend(self, str);
    if (index == priv->length) return string_append(self, str);
    
    add_len = strlen(str);
    new_len = priv->length + add_len;
    
    if (!string_ensure_capacity(priv, new_len + 1)) return false;
    
    /* Move tail forward */
    memmove(priv->data + index + add_len, 
            priv->data + index, 
            priv->length - index + 1);
    /* Insert new content */
    memcpy(priv->data + index, str, add_len);
    
    priv->length = new_len;
    return true;
}

size_t string_replace(String* self, const char* find, const char* replace) {
    StringPrivate* priv = (StringPrivate*)self;
    size_t count = 0;
    size_t find_len;
    size_t replace_len;
    size_t new_len;
    char* pos;
    
    if (!find || !*find) return 0;
    if (!replace) replace = "";
    
    find_len = strlen(find);
    replace_len = strlen(replace);
    pos = priv->data;
    
    /* Count occurrences first */
    while ((pos = strstr(pos, find)) != NULL) {
        count++;
        pos += find_len;
    }
    
    if (count == 0) return 0;
    
    /* Calculate new length */
    new_len = priv->length + count * (replace_len - find_len);
    
    if (replace_len > find_len) {
        /* Need more space */
        if (!string_ensure_capacity(priv, new_len + 1)) return 0;
    }
    
    /* Perform replacements */
    if (replace_len == find_len) {
        /* Same size - replace in place */
        pos = priv->data;
        while ((pos = strstr(pos, find)) != NULL) {
            memcpy(pos, replace, replace_len);
            pos += replace_len;
        }
    } else {
        /* Different size - need to rebuild string */
        char* new_data;
        char* src;
        char* dst;
        char* found;
        
        new_data = malloc(new_len + 1);
        if (!new_data) return 0;
        
        src = priv->data;
        dst = new_data;
        
        while ((found = strstr(src, find)) != NULL) {
            size_t prefix_len = found - src;
            memcpy(dst, src, prefix_len);
            dst += prefix_len;
            memcpy(dst, replace, replace_len);
            dst += replace_len;
            src = found + find_len;
        }
        strcpy(dst, src);
        
        free(priv->data);
        priv->data = new_data;
        priv->capacity = new_len + 1;
    }
    
    priv->length = new_len;
    return count;
}

bool string_replace_first(String* self, const char* find, const char* replace) {
    StringPrivate* priv = (StringPrivate*)self;
    char* pos;
    size_t find_len;
    size_t replace_len;
    size_t prefix_len;
    size_t suffix_start;
    size_t new_len;
    
    if (!find || !*find) return false;
    if (!replace) replace = "";
    
    pos = strstr(priv->data, find);
    if (!pos) return false;
    
    find_len = strlen(find);
    replace_len = strlen(replace);
    prefix_len = pos - priv->data;
    suffix_start = prefix_len + find_len;
    new_len = priv->length - find_len + replace_len;
    
    if (replace_len > find_len) {
        if (!string_ensure_capacity(priv, new_len + 1)) return false;
    }
    
    if (replace_len != find_len) {
        /* Move tail to new position */
        memmove(priv->data + prefix_len + replace_len,
                priv->data + suffix_start,
                priv->length - suffix_start + 1);
    }
    
    /* Insert replacement */
    memcpy(priv->data + prefix_len, replace, replace_len);
    priv->length = new_len;
    
    return true;
}

void string_clear(String* self) {
    StringPrivate* priv = (StringPrivate*)self;
    if (priv->data) {
        priv->data[0] = '\0';
        priv->length = 0;
    }
}

bool string_set(String* self, const char* str) {
    StringPrivate* priv = (StringPrivate*)self;
    string_clear(self);
    return string_append(self, str);
}

void string_reverse(String* self) {
    StringPrivate* priv = (StringPrivate*)self;
    char* start;
    char* end;
    char temp;
    
    if (priv->length <= 1) return;
    
    start = priv->data;
    end = priv->data + priv->length - 1;
    
    while (start < end) {
        temp = *start;
        *start = *end;
        *end = temp;
        start++;
        end--;
    }
}

void string_to_upper_case_in_place(String* self) {
    StringPrivate* priv = (StringPrivate*)self;
    size_t i;
    for (i = 0; i < priv->length; i++) {
        priv->data[i] = toupper((unsigned char)priv->data[i]);
    }
}

void string_to_lower_case_in_place(String* self) {
    StringPrivate* priv = (StringPrivate*)self;
    size_t i;
    for (i = 0; i < priv->length; i++) {
        priv->data[i] = tolower((unsigned char)priv->data[i]);
    }
}

/* ======================================================================== */
/* Forward Declarations                                                     */
/* ======================================================================== */

String* string_clone(String* self);

/* ======================================================================== */
/* String Creation Functions                                                */
/* ======================================================================== */

String* string_substring(String* self, size_t start, size_t length) {
    StringPrivate* priv = (StringPrivate*)self;
    String* result;
    StringPrivate* res_priv;
    
    if (start >= priv->length) return StringMake("");
    
    if (length == 0 || start + length > priv->length) {
        length = priv->length - start;
    }
    
    result = StringMakeWithCapacity(NULL, length + 1);
    if (!result) return NULL;
    
    res_priv = (StringPrivate*)result;
    memcpy(res_priv->data, priv->data + start, length);
    res_priv->data[length] = '\0';
    res_priv->length = length;
    
    return result;
}

String* string_trim(String* self) {
    StringPrivate* priv = (StringPrivate*)self;
    size_t start = 0;
    size_t end = priv->length;
    
    /* Find first non-whitespace */
    while (start < priv->length && char_is_whitespace(priv->data[start])) {
        start++;
    }
    
    /* Find last non-whitespace */
    while (end > start && char_is_whitespace(priv->data[end - 1])) {
        end--;
    }
    
    return string_substring(self, start, end - start);
}

String* string_trim_left(String* self) {
    StringPrivate* priv = (StringPrivate*)self;
    
    size_t start = 0;
    while (start < priv->length && char_is_whitespace(priv->data[start])) {
        start++;
    }
    
    return string_substring(self, start, 0);
}

String* string_trim_right(String* self) {
    StringPrivate* priv = (StringPrivate*)self;
    
    size_t end = priv->length;
    while (end > 0 && char_is_whitespace(priv->data[end - 1])) {
        end--;
    }
    
    return string_substring(self, 0, end);
}

String* string_to_upper_case(String* self) {
    String* result = string_clone(self);
    if (result) {
        string_to_upper_case_in_place(result);
    }
    return result;
}

String* string_to_lower_case(String* self) {
    String* result = string_clone(self);
    if (result) {
        string_to_lower_case_in_place(result);
    }
    return result;
}

String* string_clone(String* self) {
    StringPrivate* priv = (StringPrivate*)self;
    return StringMake(priv->data);
}

String* string_repeat(String* self, size_t count) {
    StringPrivate* priv = (StringPrivate*)self;
    size_t new_len;
    String* result;
    StringPrivate* res_priv;
    size_t i;
    
    if (count == 0) return StringMake("");
    if (count == 1) return string_clone(self);
    
    new_len = priv->length * count;
    result = StringMakeWithCapacity(NULL, new_len + 1);
    if (!result) return NULL;
    
    res_priv = (StringPrivate*)result;
    for (i = 0; i < count; i++) {
        memcpy(res_priv->data + (i * priv->length), priv->data, priv->length);
    }
    res_priv->data[new_len] = '\0';
    res_priv->length = new_len;
    
    return result;
}

/* ======================================================================== */
/* String Searching Functions                                               */
/* ======================================================================== */

bool string_contains(String* self, const char* needle) {
    StringPrivate* priv = (StringPrivate*)self;
    if (!needle) return false;
    return strstr(priv->data, needle) != NULL;
}

bool string_starts_with(String* self, const char* prefix) {
    StringPrivate* priv = (StringPrivate*)self;
    size_t prefix_len;
    
    if (!prefix) return false;
    
    prefix_len = strlen(prefix);
    if (prefix_len > priv->length) return false;
    
    return memcmp(priv->data, prefix, prefix_len) == 0;
}

bool string_ends_with(String* self, const char* suffix) {
    StringPrivate* priv = (StringPrivate*)self;
    size_t suffix_len;
    
    if (!suffix) return false;
    
    suffix_len = strlen(suffix);
    if (suffix_len > priv->length) return false;
    
    return memcmp(priv->data + priv->length - suffix_len, suffix, suffix_len) == 0;
}

size_t string_index_of(String* self, const char* needle) {
    StringPrivate* priv = (StringPrivate*)self;
    char* found;
    
    if (!needle) return (size_t)-1;
    
    found = strstr(priv->data, needle);
    if (!found) return (size_t)-1;
    
    return found - priv->data;
}

size_t string_last_index_of(String* self, const char* needle) {
    StringPrivate* priv = (StringPrivate*)self;
    size_t needle_len;
    size_t i;
    
    if (!needle) return (size_t)-1;
    
    needle_len = strlen(needle);
    if (needle_len > priv->length) return (size_t)-1;
    for (i = priv->length - needle_len + 1; i > 0; i--) {
        if (memcmp(priv->data + i - 1, needle, needle_len) == 0) {
            return i - 1;
        }
    }
    
    return (size_t)-1;
}

size_t string_index_of_any(String* self, const char* chars) {
    StringPrivate* priv = (StringPrivate*)self;
    size_t i;
    
    if (!chars) return (size_t)-1;
    for (i = 0; i < priv->length; i++) {
        if (strchr(chars, priv->data[i])) {
            return i;
        }
    }
    
    return (size_t)-1;
}

size_t string_count(String* self, const char* needle) {
    StringPrivate* priv = (StringPrivate*)self;
    size_t count = 0;
    size_t needle_len;
    char* pos;
    
    if (!needle || !*needle) return 0;
    
    needle_len = strlen(needle);
    pos = priv->data;
    
    while ((pos = strstr(pos, needle)) != NULL) {
        count++;
        pos += needle_len;
    }
    
    return count;
}

/* ======================================================================== */
/* String Splitting Functions                                               */
/* ======================================================================== */

String** string_split(String* self, const char* delimiter, size_t* out_count) {
    StringPrivate* priv = (StringPrivate*)self;
    size_t count = 1;
    size_t delim_len;
    char* pos;
    String** result;
    
    if (!delimiter || !out_count) return NULL;
    
    *out_count = 0;
    
    /* Count splits */
    delim_len = strlen(delimiter);
    pos = priv->data;
    
    if (delim_len == 0) {
        /* Split each character */
        count = priv->length;
    } else {
        while ((pos = strstr(pos, delimiter)) != NULL) {
            count++;
            pos += delim_len;
        }
    }
    
    /* Allocate array */
    result = calloc(count, sizeof(String*));
    if (!result) return NULL;
    
    /* Perform split */
    if (delim_len == 0) {
        /* Split each character */
        size_t i;
        for (i = 0; i < priv->length; i++) {
            result[i] = StringMakeWithCapacity(NULL, 2);
            if (!result[i]) {
                StringArray_Free(result, i);
                return NULL;
            }
            result[i]->appendChar(priv->data[i]);
        }
        *out_count = priv->length;
    } else {
        /* Split by delimiter */
        char* start;
        char* end;
        size_t idx = 0;
        
        start = priv->data;
        
        while ((end = strstr(start, delimiter)) != NULL) {
            size_t part_len;
            StringPrivate* part_priv;
            
            part_len = end - start;
            result[idx] = StringMakeWithCapacity(NULL, part_len + 1);
            if (!result[idx]) {
                StringArray_Free(result, idx);
                return NULL;
            }
            
            part_priv = (StringPrivate*)result[idx];
            memcpy(part_priv->data, start, part_len);
            part_priv->data[part_len] = '\0';
            part_priv->length = part_len;
            
            idx++;
            start = end + delim_len;
        }
        
        /* Add final part */
        result[idx] = StringMake(start);
        if (!result[idx]) {
            StringArray_Free(result, idx);
            return NULL;
        }
        
        *out_count = count;
    }
    
    return result;
}

String** string_split_any(String* self, const char* chars, size_t* out_count) {
    StringPrivate* priv = (StringPrivate*)self;
    size_t count = 1;
    size_t i;
    String** result;
    char* start;
    size_t idx = 0;
    
    if (!chars || !out_count) return NULL;
    
    *out_count = 0;
    
    /* Count parts */
    for (i = 0; i < priv->length; i++) {
        if (strchr(chars, priv->data[i])) {
            count++;
        }
    }
    
    /* Allocate array */
    result = calloc(count, sizeof(String*));
    if (!result) return NULL;
    
    /* Perform split */
    start = priv->data;
    
    for (i = 0; i < priv->length; i++) {
        if (strchr(chars, priv->data[i])) {
            size_t part_len;
            StringPrivate* part_priv;
            
            part_len = (priv->data + i) - start;
            result[idx] = StringMakeWithCapacity(NULL, part_len + 1);
            if (!result[idx]) {
                StringArray_Free(result, idx);
                return NULL;
            }
            
            part_priv = (StringPrivate*)result[idx];
            memcpy(part_priv->data, start, part_len);
            part_priv->data[part_len] = '\0';
            part_priv->length = part_len;
            
            idx++;
            start = priv->data + i + 1;
        }
    }
    
    /* Add final part */
    result[idx] = StringMake(start);
    if (!result[idx]) {
        StringArray_Free(result, idx);
        return NULL;
    }
    
    *out_count = count;
    return result;
}

String** string_split_lines(String* self, size_t* out_count) {
    return string_split_any(self, "\r\n", out_count);
}

String* string_join(String* self, String** strings, size_t count) {
    StringPrivate* priv = (StringPrivate*)self;
    size_t total_len = 0;
    size_t i;
    String* result;
    
    if (!strings || count == 0) return StringMake("");
    
    /* Calculate total length */
    for (i = 0; i < count; i++) {
        if (strings[i]) {
            total_len += strings[i]->length();
            if (i < count - 1) {
                total_len += priv->length;
            }
        }
    }
    
    /* Create result */
    result = StringMakeWithCapacity(NULL, total_len + 1);
    if (!result) return NULL;
    
    /* Join strings */
    for (i = 0; i < count; i++) {
        if (strings[i]) {
            result->append(strings[i]->cStr());
            if (i < count - 1) {
                result->append(priv->data);
            }
        }
    }
    
    return result;
}

/* ======================================================================== */
/* String Comparison Functions                                              */
/* ======================================================================== */

int string_compare(String* self, const char* other) {
    StringPrivate* priv = (StringPrivate*)self;
    if (!other) return 1;
    return strcmp(priv->data, other);
}

int string_compare_ignore_case(String* self, const char* other) {
    StringPrivate* priv = (StringPrivate*)self;
    const char* s1;
    const char* s2;
    
    if (!other) return 1;
    
    s1 = priv->data;
    s2 = other;
    
    while (*s1 && *s2) {
        int c1 = tolower((unsigned char)*s1);
        int c2 = tolower((unsigned char)*s2);
        if (c1 != c2) return c1 - c2;
        s1++;
        s2++;
    }
    
    return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
}

bool string_equals(String* self, const char* other) {
    return string_compare(self, other) == 0;
}

bool string_equals_ignore_case(String* self, const char* other) {
    return string_compare_ignore_case(self, other) == 0;
}

/* ======================================================================== */
/* String Utility Functions                                                 */
/* ======================================================================== */

bool string_is_integer(String* self) {
    StringPrivate* priv = (StringPrivate*)self;
    size_t i = 0;
    
    if (priv->length == 0) return false;
    if (priv->data[0] == '-' || priv->data[0] == '+') {
        if (priv->length == 1) return false;
        i = 1;
    }
    
    for (; i < priv->length; i++) {
        if (!isdigit((unsigned char)priv->data[i])) {
            return false;
        }
    }
    
    return true;
}

bool string_is_float(String* self) {
    StringPrivate* priv = (StringPrivate*)self;
    if (priv->length == 0) return false;
    
    bool has_dot = false;
    bool has_e = false;
    size_t i = 0;
    
    if (priv->data[0] == '-' || priv->data[0] == '+') {
        if (priv->length == 1) return false;
        i = 1;
    }
    
    for (; i < priv->length; i++) {
        char c = priv->data[i];
        
        if (c == '.') {
            if (has_dot || has_e) return false;
            has_dot = true;
        } else if (c == 'e' || c == 'E') {
            if (has_e || i == 0 || i == priv->length - 1) return false;
            has_e = true;
            if (i + 1 < priv->length && (priv->data[i+1] == '+' || priv->data[i+1] == '-')) {
                i++;
            }
        } else if (!isdigit((unsigned char)c)) {
            return false;
        }
    }
    
    return true;
}

bool string_is_alpha(String* self) {
    StringPrivate* priv = (StringPrivate*)self;
    if (priv->length == 0) return false;
    
    size_t i;
    for (i = 0; i < priv->length; i++) {
        if (!isalpha((unsigned char)priv->data[i])) {
            return false;
        }
    }
    
    return true;
}

bool string_is_digit(String* self) {
    StringPrivate* priv = (StringPrivate*)self;
    if (priv->length == 0) return false;
    
    size_t i;
    for (i = 0; i < priv->length; i++) {
        if (!isdigit((unsigned char)priv->data[i])) {
            return false;
        }
    }
    
    return true;
}

bool string_is_alpha_numeric(String* self) {
    StringPrivate* priv = (StringPrivate*)self;
    if (priv->length == 0) return false;
    
    size_t i;
    for (i = 0; i < priv->length; i++) {
        if (!isalnum((unsigned char)priv->data[i])) {
            return false;
        }
    }
    
    return true;
}

bool string_is_whitespace(String* self) {
    StringPrivate* priv = (StringPrivate*)self;
    if (priv->length == 0) return true;
    
    size_t i;
    for (i = 0; i < priv->length; i++) {
        if (!char_is_whitespace(priv->data[i])) {
            return false;
        }
    }
    
    return true;
}

int string_to_int(String* self, int default_value) {
    StringPrivate* priv = (StringPrivate*)self;
    if (!string_is_integer(self)) return default_value;
    
    char* endptr;
    long result = strtol(priv->data, &endptr, 10);
    
    if (*endptr != '\0') return default_value;
    if (result > INT_MAX || result < INT_MIN) return default_value;
    
    return (int)result;
}

float string_to_float(String* self, float default_value) {
    StringPrivate* priv = (StringPrivate*)self;
    if (!string_is_float(self) && !string_is_integer(self)) return default_value;
    
    char* endptr;
    float result = strtof(priv->data, &endptr);
    
    if (*endptr != '\0') return default_value;
    
    return result;
}

double string_to_double(String* self, double default_value) {
    StringPrivate* priv = (StringPrivate*)self;
    if (!string_is_float(self) && !string_is_integer(self)) return default_value;
    
    char* endptr;
    double result = strtod(priv->data, &endptr);
    
    if (*endptr != '\0') return default_value;
    
    return result;
}

size_t string_hash(String* self) {
    StringPrivate* priv = (StringPrivate*)self;
    
    /* DJB2 hash algorithm */
    size_t hash = 5381;
    size_t i;
    
    for (i = 0; i < priv->length; i++) {
        hash = ((hash << 5) + hash) + (unsigned char)priv->data[i];
    }
    
    return hash;
}

String* string_to_string(String* self) {
    return string_clone(self);
}

/* ======================================================================== */
/* Memory Management Functions                                              */
/* ======================================================================== */

bool string_reserve(String* self, size_t new_capacity) {
    StringPrivate* priv = (StringPrivate*)self;
    return string_ensure_capacity(priv, new_capacity);
}

bool string_shrink_to_fit(String* self) {
    StringPrivate* priv = (StringPrivate*)self;
    
    if (priv->capacity == priv->length + 1) return true;
    
    char* new_data = realloc(priv->data, priv->length + 1);
    if (!new_data) return false;
    
    priv->data = new_data;
    priv->capacity = priv->length + 1;
    return true;
}

void string_free(String* self) {
    StringPrivate* priv = (StringPrivate*)self;
    if (!priv) return;
    
    /* Free string data */
    free(priv->data);
    
    /* Free all trampoline functions */
    if (self->cStr) trampoline_free(self->cStr);
    if (self->length) trampoline_free(self->length);
    if (self->capacity) trampoline_free(self->capacity);
    if (self->isEmpty) trampoline_free(self->isEmpty);
    if (self->charAt) trampoline_free(self->charAt);
    if (self->append) trampoline_free(self->append);
    if (self->appendChar) trampoline_free(self->appendChar);
    if (self->appendFormat) trampoline_free(self->appendFormat);
    if (self->prepend) trampoline_free(self->prepend);
    if (self->insert) trampoline_free(self->insert);
    if (self->replace) trampoline_free(self->replace);
    if (self->replaceFirst) trampoline_free(self->replaceFirst);
    if (self->clear) trampoline_free(self->clear);
    if (self->set) trampoline_free(self->set);
    if (self->reverse) trampoline_free(self->reverse);
    if (self->toUpperCaseInPlace) trampoline_free(self->toUpperCaseInPlace);
    if (self->toLowerCaseInPlace) trampoline_free(self->toLowerCaseInPlace);
    if (self->substring) trampoline_free(self->substring);
    if (self->trim) trampoline_free(self->trim);
    if (self->trimLeft) trampoline_free(self->trimLeft);
    if (self->trimRight) trampoline_free(self->trimRight);
    if (self->toUpperCase) trampoline_free(self->toUpperCase);
    if (self->toLowerCase) trampoline_free(self->toLowerCase);
    if (self->clone) trampoline_free(self->clone);
    if (self->repeat) trampoline_free(self->repeat);
    if (self->contains) trampoline_free(self->contains);
    if (self->startsWith) trampoline_free(self->startsWith);
    if (self->endsWith) trampoline_free(self->endsWith);
    if (self->indexOf) trampoline_free(self->indexOf);
    if (self->lastIndexOf) trampoline_free(self->lastIndexOf);
    if (self->indexOfAny) trampoline_free(self->indexOfAny);
    if (self->count) trampoline_free(self->count);
    if (self->split) trampoline_free(self->split);
    if (self->splitAny) trampoline_free(self->splitAny);
    if (self->splitLines) trampoline_free(self->splitLines);
    if (self->join) trampoline_free(self->join);
    if (self->compare) trampoline_free(self->compare);
    if (self->compareIgnoreCase) trampoline_free(self->compareIgnoreCase);
    if (self->equals) trampoline_free(self->equals);
    if (self->equalsIgnoreCase) trampoline_free(self->equalsIgnoreCase);
    if (self->isInteger) trampoline_free(self->isInteger);
    if (self->isFloat) trampoline_free(self->isFloat);
    if (self->isAlpha) trampoline_free(self->isAlpha);
    if (self->isDigit) trampoline_free(self->isDigit);
    if (self->isAlphaNumeric) trampoline_free(self->isAlphaNumeric);
    if (self->isWhitespace) trampoline_free(self->isWhitespace);
    if (self->toInt) trampoline_free(self->toInt);
    if (self->toFloat) trampoline_free(self->toFloat);
    if (self->toDouble) trampoline_free(self->toDouble);
    if (self->hash) trampoline_free(self->hash);
    if (self->toString) trampoline_free(self->toString);
    if (self->reserve) trampoline_free(self->reserve);
    if (self->shrinkToFit) trampoline_free(self->shrinkToFit);
    if (self->free) trampoline_free(self->free);
    
    /* Free the structure itself */
    free(priv);
}

/* ======================================================================== */
/* String Creation Functions                                                */
/* ======================================================================== */

static String* string_make_internal(const char* str, size_t initial_capacity) {
    size_t str_len = str ? strlen(str) : 0;
    if (initial_capacity < str_len + 1) {
        initial_capacity = str_len + 1;
    }
    
    /* Allocate private structure */
    StringPrivate* priv = calloc(1, sizeof(StringPrivate));
    if (!priv) return NULL;
    
    /* Allocate string buffer */
    priv->data = calloc(initial_capacity, 1);
    if (!priv->data) {
        free(priv);
        return NULL;
    }
    
    /* Initialize fields */
    if (str) {
        memcpy(priv->data, str, str_len);
        priv->length = str_len;
    }
    priv->capacity = initial_capacity;
    
    /* Get reference to embedded public interface */
    String* string = &priv->public;
    
    /* Create trampoline functions */
    trampoline_allocations allocations = {0};
    
    /* Core access */
    string->cStr = trampoline_create_and_track(string_c_str, string, 0, &allocations);
    string->length = trampoline_create_and_track(string_length, string, 0, &allocations);
    string->capacity = trampoline_create_and_track(string_capacity, string, 0, &allocations);
    string->isEmpty = trampoline_create_and_track(string_is_empty, string, 0, &allocations);
    string->charAt = trampoline_create_and_track(string_char_at, string, 1, &allocations);
    
    /* Modification */
    string->append = trampoline_create_and_track(string_append, string, 1, &allocations);
    string->appendChar = trampoline_create_and_track(string_append_char, string, 1, &allocations);
    string->appendFormat = trampoline_create_and_track(string_append_format, string, 2, &allocations);
    string->prepend = trampoline_create_and_track(string_prepend, string, 1, &allocations);
    string->insert = trampoline_create_and_track(string_insert, string, 2, &allocations);
    string->replace = trampoline_create_and_track(string_replace, string, 2, &allocations);
    string->replaceFirst = trampoline_create_and_track(string_replace_first, string, 2, &allocations);
    string->clear = trampoline_create_and_track(string_clear, string, 0, &allocations);
    string->set = trampoline_create_and_track(string_set, string, 1, &allocations);
    string->reverse = trampoline_create_and_track(string_reverse, string, 0, &allocations);
    string->toUpperCaseInPlace = trampoline_create_and_track(string_to_upper_case_in_place, string, 0, &allocations);
    string->toLowerCaseInPlace = trampoline_create_and_track(string_to_lower_case_in_place, string, 0, &allocations);
    
    /* Creation */
    string->substring = trampoline_create_and_track(string_substring, string, 2, &allocations);
    string->trim = trampoline_create_and_track(string_trim, string, 0, &allocations);
    string->trimLeft = trampoline_create_and_track(string_trim_left, string, 0, &allocations);
    string->trimRight = trampoline_create_and_track(string_trim_right, string, 0, &allocations);
    string->toUpperCase = trampoline_create_and_track(string_to_upper_case, string, 0, &allocations);
    string->toLowerCase = trampoline_create_and_track(string_to_lower_case, string, 0, &allocations);
    string->clone = trampoline_create_and_track(string_clone, string, 0, &allocations);
    string->repeat = trampoline_create_and_track(string_repeat, string, 1, &allocations);
    
    /* Searching */
    string->contains = trampoline_create_and_track(string_contains, string, 1, &allocations);
    string->startsWith = trampoline_create_and_track(string_starts_with, string, 1, &allocations);
    string->endsWith = trampoline_create_and_track(string_ends_with, string, 1, &allocations);
    string->indexOf = trampoline_create_and_track(string_index_of, string, 1, &allocations);
    string->lastIndexOf = trampoline_create_and_track(string_last_index_of, string, 1, &allocations);
    string->indexOfAny = trampoline_create_and_track(string_index_of_any, string, 1, &allocations);
    string->count = trampoline_create_and_track(string_count, string, 1, &allocations);
    
    /* Splitting */
    string->split = trampoline_create_and_track(string_split, string, 2, &allocations);
    string->splitAny = trampoline_create_and_track(string_split_any, string, 2, &allocations);
    string->splitLines = trampoline_create_and_track(string_split_lines, string, 1, &allocations);
    string->join = trampoline_create_and_track(string_join, string, 2, &allocations);
    
    /* Comparison */
    string->compare = trampoline_create_and_track(string_compare, string, 1, &allocations);
    string->compareIgnoreCase = trampoline_create_and_track(string_compare_ignore_case, string, 1, &allocations);
    string->equals = trampoline_create_and_track(string_equals, string, 1, &allocations);
    string->equalsIgnoreCase = trampoline_create_and_track(string_equals_ignore_case, string, 1, &allocations);
    
    /* Utilities */
    string->isInteger = trampoline_create_and_track(string_is_integer, string, 0, &allocations);
    string->isFloat = trampoline_create_and_track(string_is_float, string, 0, &allocations);
    string->isAlpha = trampoline_create_and_track(string_is_alpha, string, 0, &allocations);
    string->isDigit = trampoline_create_and_track(string_is_digit, string, 0, &allocations);
    string->isAlphaNumeric = trampoline_create_and_track(string_is_alpha_numeric, string, 0, &allocations);
    string->isWhitespace = trampoline_create_and_track(string_is_whitespace, string, 0, &allocations);
    string->toInt = trampoline_create_and_track(string_to_int, string, 1, &allocations);
    string->toFloat = trampoline_create_and_track(string_to_float, string, 1, &allocations);
    string->toDouble = trampoline_create_and_track(string_to_double, string, 1, &allocations);
    string->hash = trampoline_create_and_track(string_hash, string, 0, &allocations);
    string->toString = trampoline_create_and_track(string_to_string, string, 0, &allocations);
    
    /* Memory management */
    string->reserve = trampoline_create_and_track(string_reserve, string, 1, &allocations);
    string->shrinkToFit = trampoline_create_and_track(string_shrink_to_fit, string, 0, &allocations);
    string->free = trampoline_create_and_track(string_free, string, 0, &allocations);
    
    /* Validate all trampolines were created */
    if (!trampolines_validate(&allocations)) {
        free(priv->data);
        free(priv);
        return NULL;
    }
    
    return string;
}

String* StringMake(const char* str) {
    return string_make_internal(str, 16);
}

String* StringMakeWithCapacity(const char* str, size_t capacity) {
    return string_make_internal(str, capacity);
}

String* StringMakeFormat(const char* format, ...) {
    va_list args;
    int required;
    String* result;
    StringPrivate* priv;
    
    if (!format) return StringMake("");
    
    /* First pass to determine size */
    va_start(args, format);
    required = vsnprintf(NULL, 0, format, args);
    va_end(args);
    
    if (required < 0) {
        return NULL;
    }
    
    /* Create string with appropriate capacity */
    result = StringMakeWithCapacity(NULL, required + 1);
    if (!result) {
        return NULL;
    }
    
    /* Second pass to format the string */
    priv = (StringPrivate*)result;
    va_start(args, format);
    vsnprintf(priv->data, required + 1, format, args);
    va_end(args);
    
    priv->length = required;
    
    return result;
}

String* StringFromInt(int value) {
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%d", value);
    return StringMake(buffer);
}

String* StringFromFloat(float value, int precision) {
    char buffer[64];
    char format[16];
    snprintf(format, sizeof(format), "%%.%df", precision);
    snprintf(buffer, sizeof(buffer), format, value);
    return StringMake(buffer);
}

String* StringFromDouble(double value, int precision) {
    char buffer[64];
    char format[16];
    snprintf(format, sizeof(format), "%%.%df", precision);
    snprintf(buffer, sizeof(buffer), format, value);
    return StringMake(buffer);
}

/* ======================================================================== */
/* String Array Utilities                                                   */
/* ======================================================================== */

void StringArray_Free(String** strings, size_t count) {
    if (!strings) return;
    
    size_t i;
    for (i = 0; i < count; i++) {
        if (strings[i]) {
            strings[i]->free();
        }
    }
    free(strings);
}

String* StringArray_Join(const char** strings, size_t count, const char* separator) {
    size_t total_len = 0;
    size_t sep_len;
    size_t i;
    String* result;
    
    if (!strings || count == 0) return StringMake("");
    if (!separator) separator = "";
    
    /* Calculate total length */
    sep_len = strlen(separator);
    
    for (i = 0; i < count; i++) {
        if (strings[i]) {
            total_len += strlen(strings[i]);
            if (i < count - 1) {
                total_len += sep_len;
            }
        }
    }
    
    /* Create result */
    result = StringMakeWithCapacity(NULL, total_len + 1);
    if (!result) return NULL;
    
    /* Join strings */
    for (i = 0; i < count; i++) {
        if (strings[i]) {
            result->append(strings[i]);
            if (i < count - 1) {
                result->append(separator);
            }
        }
    }
    
    return result;
}