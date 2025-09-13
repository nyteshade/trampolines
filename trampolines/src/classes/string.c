/**
 * @file string.c
 * @brief Implementation of String with comprehensive string manipulation using trampolines
 */

#include <trampolines/string.h>
#include <trampoline.h>  /* Expects trampoline.h in system includes */
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

static bool char_is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
}

/* ======================================================================== */
/* Core String Access Functions (using new TF_ macros)                      */
/* ======================================================================== */

static TF_Getter(string_c_str, String, StringPrivate, const char*)
    return private->data ? private->data : "";
}

static TF_Getter(string_length, String, StringPrivate, size_t)
    return private->length;
}

static TF_Getter(string_capacity, String, StringPrivate, size_t)
    return private->capacity;
}

static TF_Getter(string_is_empty, String, StringPrivate, bool)
    return private->length == 0;
}

static TF_Unary(char, string_char_at, String, StringPrivate, size_t, index)
    if (index >= private->length) return '\0';
    return private->data[index];
}

/* ======================================================================== */
/* String Modification Functions                                            */
/* ======================================================================== */

static TF_Unary(bool, string_append, String, StringPrivate, const char*, str)
    size_t add_len;
    size_t new_len;
    
    if (!str || !*str) return true;
    
    add_len = strlen(str);
    new_len = private->length + add_len;
    
    if (!string_ensure_capacity(private, new_len + 1)) return false;
    
    memcpy(private->data + private->length, str, add_len + 1);
    private->length = new_len;
    return true;
}

static TF_Unary(bool, string_append_char, String, StringPrivate, char, ch)
    if (!string_ensure_capacity(private, private->length + 2)) return false;
    
    private->data[private->length++] = ch;
    private->data[private->length] = '\0';
    return true;
}

static bool string_append_format(String* self, const char* format, ...) {
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

static TF_Unary(bool, string_prepend, String, StringPrivate, const char*, str)
    size_t add_len;
    size_t new_len;
    
    if (!str || !*str) return true;
    
    add_len = strlen(str);
    new_len = private->length + add_len;
    
    if (!string_ensure_capacity(private, new_len + 1)) return false;
    
    /* Move existing content forward */
    memmove(private->data + add_len, private->data, private->length + 1);
    /* Copy new content to beginning */
    memcpy(private->data, str, add_len);
    
    private->length = new_len;
    return true;
}

static TF_Dyadic(bool, string_insert, String, StringPrivate, size_t, index, const char*, str)
    size_t add_len;
    size_t new_len;
    
    if (!str || !*str) return true;
    if (index > private->length) return false;
    
    if (index == 0) return string_prepend(self, str);
    if (index == private->length) return string_append(self, str);
    
    add_len = strlen(str);
    new_len = private->length + add_len;
    
    if (!string_ensure_capacity(private, new_len + 1)) return false;
    
    /* Move tail forward */
    memmove(private->data + index + add_len, 
            private->data + index, 
            private->length - index + 1);
    
    /* Insert new content */
    memcpy(private->data + index, str, add_len);
    
    private->length = new_len;
    return true;
}

static TF_Dyadic(size_t, string_replace, String, StringPrivate, const char*, find, const char*, replace)
    char* pos;
    size_t find_len;
    size_t replace_len;
    size_t count = 0;
    char* temp;
    size_t temp_len;
    size_t temp_capacity;
    char* current;
    
    if (!find || !*find) return 0;
    if (!replace) replace = "";
    
    find_len = strlen(find);
    replace_len = strlen(replace);
    
    /* Count occurrences first */
    pos = private->data;
    while ((pos = strstr(pos, find)) != NULL) {
        count++;
        pos += find_len;
    }
    
    if (count == 0) return 0;
    
    /* Calculate new length */
    temp_len = private->length + count * (replace_len - find_len);
    temp_capacity = temp_len + 1;
    
    /* Allocate temporary buffer */
    temp = malloc(temp_capacity);
    if (!temp) return 0;
    
    /* Perform replacements */
    current = private->data;
    temp_len = 0;
    
    while ((pos = strstr(current, find)) != NULL) {
        size_t segment_len = pos - current;
        
        /* Copy segment before match */
        memcpy(temp + temp_len, current, segment_len);
        temp_len += segment_len;
        
        /* Copy replacement */
        memcpy(temp + temp_len, replace, replace_len);
        temp_len += replace_len;
        
        /* Move past the find string */
        current = pos + find_len;
    }
    
    /* Copy remaining part */
    strcpy(temp + temp_len, current);
    
    /* Replace the data */
    free(private->data);
    private->data = temp;
    private->length = strlen(temp);
    private->capacity = temp_capacity;
    
    return count;
}

static TF_Dyadic(bool, string_replace_first, String, StringPrivate, const char*, find, const char*, replace)
    char* pos;
    size_t find_len;
    size_t replace_len;
    size_t new_len;
    
    if (!find || !*find) return false;
    if (!replace) replace = "";
    
    pos = strstr(private->data, find);
    if (!pos) return false;
    
    find_len = strlen(find);
    replace_len = strlen(replace);
    new_len = private->length - find_len + replace_len;
    
    if (new_len + 1 > private->capacity) {
        if (!string_ensure_capacity(private, new_len + 1)) return false;
    }
    
    /* Move tail to make room */
    if (replace_len != find_len) {
        memmove(pos + replace_len, pos + find_len, 
                strlen(pos + find_len) + 1);
    }
    
    /* Insert replacement */
    memcpy(pos, replace, replace_len);
    private->length = new_len;
    
    return true;
}

static TF_Nullary(string_clear, String, StringPrivate)
    if (private->data) {
        private->data[0] = '\0';
        private->length = 0;
    }
}

static TF_Unary(bool, string_set, String, StringPrivate, const char*, str)
    size_t new_len;
    
    if (!str) str = "";
    new_len = strlen(str);
    
    if (!string_ensure_capacity(private, new_len + 1)) return false;
    
    memcpy(private->data, str, new_len + 1);
    private->length = new_len;
    return true;
}

static TF_Nullary(string_reverse, String, StringPrivate)
    size_t i;
    size_t j;
    char temp;
    
    if (private->length <= 1) return;
    
    for (i = 0, j = private->length - 1; i < j; i++, j--) {
        temp = private->data[i];
        private->data[i] = private->data[j];
        private->data[j] = temp;
    }
}

static TF_Nullary(string_to_upper_case_in_place, String, StringPrivate)
    size_t i;
    for (i = 0; i < private->length; i++) {
        private->data[i] = toupper((unsigned char)private->data[i]);
    }
}

static TF_Nullary(string_to_lower_case_in_place, String, StringPrivate)
    size_t i;
    for (i = 0; i < private->length; i++) {
        private->data[i] = tolower((unsigned char)private->data[i]);
    }
}

/* ======================================================================== */
/* String Creation Functions (Returns New String)                           */
/* ======================================================================== */

String* StringMake(const char* str);
String* StringMakeWithCapacity(const char* str, size_t capacity);

static TF_Dyadic(String*, string_substring, String, StringPrivate, size_t, start, size_t, length)
    String* result;
    StringPrivate* res_priv;
    
    if (start >= private->length) return StringMake("");
    
    if (length == 0 || start + length > private->length) {
        length = private->length - start;
    }
    
    result = StringMakeWithCapacity(NULL, length + 1);
    if (!result) return NULL;
    
    res_priv = (StringPrivate*)result;
    memcpy(res_priv->data, private->data + start, length);
    res_priv->data[length] = '\0';
    res_priv->length = length;
    
    return result;
}

static TF_Getter(string_trim, String, StringPrivate, String*)
    size_t start = 0;
    size_t end = private->length;
    
    /* Find first non-whitespace */
    while (start < private->length && char_is_whitespace(private->data[start])) {
        start++;
    }
    
    /* Find last non-whitespace */
    while (end > start && char_is_whitespace(private->data[end - 1])) {
        end--;
    }
    
    return string_substring(self, start, end - start);
}

static TF_Getter(string_trim_left, String, StringPrivate, String*)
    size_t start = 0;
    while (start < private->length && char_is_whitespace(private->data[start])) {
        start++;
    }
    
    return string_substring(self, start, 0);
}

static TF_Getter(string_trim_right, String, StringPrivate, String*)
    size_t end = private->length;
    while (end > 0 && char_is_whitespace(private->data[end - 1])) {
        end--;
    }
    
    return string_substring(self, 0, end);
}

static TF_Getter(string_to_upper_case, String, StringPrivate, String*)
    String* result = StringMake(private->data);
    if (result) {
        result->toUpperCaseInPlace();
    }
    return result;
}

static TF_Getter(string_to_lower_case, String, StringPrivate, String*)
    String* result = StringMake(private->data);
    if (result) {
        result->toLowerCaseInPlace();
    }
    return result;
}

static TF_Getter(string_clone, String, StringPrivate, String*)
    return StringMake(private->data);
}

static TF_Unary(String*, string_repeat, String, StringPrivate, size_t, count)
    size_t new_len;
    String* result;
    StringPrivate* res_priv;
    size_t i;
    
    if (count == 0) return StringMake("");
    if (count == 1) return string_clone(self);
    
    new_len = private->length * count;
    result = StringMakeWithCapacity(NULL, new_len + 1);
    if (!result) return NULL;
    
    res_priv = (StringPrivate*)result;
    for (i = 0; i < count; i++) {
        memcpy(res_priv->data + (i * private->length), private->data, private->length);
    }
    res_priv->data[new_len] = '\0';
    res_priv->length = new_len;
    
    return result;
}

/* ======================================================================== */
/* String Searching Functions                                               */
/* ======================================================================== */

static TF_Unary(bool, string_contains, String, StringPrivate, const char*, needle)
    if (!needle) return false;
    return strstr(private->data, needle) != NULL;
}

static TF_Unary(bool, string_starts_with, String, StringPrivate, const char*, prefix)
    size_t prefix_len;
    
    if (!prefix) return false;
    
    prefix_len = strlen(prefix);
    if (prefix_len > private->length) return false;
    
    return memcmp(private->data, prefix, prefix_len) == 0;
}

static TF_Unary(bool, string_ends_with, String, StringPrivate, const char*, suffix)
    size_t suffix_len;
    
    if (!suffix) return false;
    
    suffix_len = strlen(suffix);
    if (suffix_len > private->length) return false;
    
    return memcmp(private->data + private->length - suffix_len, suffix, suffix_len) == 0;
}

static TF_Unary(size_t, string_index_of, String, StringPrivate, const char*, needle)
    char* found;
    
    if (!needle) return (size_t)-1;
    
    found = strstr(private->data, needle);
    if (!found) return (size_t)-1;
    
    return found - private->data;
}

static TF_Unary(size_t, string_last_index_of, String, StringPrivate, const char*, needle)
    size_t needle_len;
    char* pos;
    char* last = NULL;
    
    if (!needle) return (size_t)-1;
    
    needle_len = strlen(needle);
    if (needle_len > private->length) return (size_t)-1;
    
    pos = private->data;
    while ((pos = strstr(pos, needle)) != NULL) {
        last = pos;
        pos++;
    }
    
    if (!last) return (size_t)-1;
    return last - private->data;
}

static TF_Unary(size_t, string_index_of_any, String, StringPrivate, const char*, chars)
    size_t i;
    
    if (!chars) return (size_t)-1;
    
    for (i = 0; i < private->length; i++) {
        if (strchr(chars, private->data[i])) {
            return i;
        }
    }
    
    return (size_t)-1;
}

static TF_Unary(size_t, string_count, String, StringPrivate, const char*, needle)
    size_t count = 0;
    size_t needle_len;
    char* pos;
    
    if (!needle || !*needle) return 0;
    
    needle_len = strlen(needle);
    pos = private->data;
    
    while ((pos = strstr(pos, needle)) != NULL) {
        count++;
        pos += needle_len;
    }
    
    return count;
}

/* ======================================================================== */
/* String Splitting and Joining                                             */
/* ======================================================================== */

static TF_Dyadic(String**, string_split, String, StringPrivate, const char*, delimiter, size_t*, out_count)
    size_t delim_len;
    size_t count = 1;
    char* pos;
    String** result;
    size_t i;
    
    if (!delimiter || !out_count) return NULL;
    
    *out_count = 0;
    delim_len = strlen(delimiter);
    
    if (delim_len == 0) {
        /* Split into individual characters */
        count = private->length;
        if (count == 0) return NULL;
        
        result = calloc(count, sizeof(String*));
        if (!result) return NULL;
        
        for (i = 0; i < private->length; i++) {
            result[i] = StringMakeWithCapacity(NULL, 2);
            if (!result[i]) {
                StringArray_Free(result, i);
                return NULL;
            }
            result[i]->appendChar(private->data[i]);
        }
        *out_count = private->length;
    } else {
        /* Count parts */
        pos = private->data;
        while ((pos = strstr(pos, delimiter)) != NULL) {
            count++;
            pos += delim_len;
        }
        
        /* Allocate array */
        result = calloc(count, sizeof(String*));
        if (!result) return NULL;
        
        /* Split by delimiter */
        char* start;
        char* end;
        size_t idx = 0;
        
        start = private->data;
        
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

static TF_Dyadic(String**, string_split_any, String, StringPrivate, const char*, chars, size_t*, out_count)
    size_t count = 1;
    size_t i;
    String** result;
    char* start;
    size_t idx = 0;
    
    if (!chars || !out_count) return NULL;
    
    *out_count = 0;
    
    /* Count parts */
    for (i = 0; i < private->length; i++) {
        if (strchr(chars, private->data[i])) {
            count++;
        }
    }
    
    /* Allocate array */
    result = calloc(count, sizeof(String*));
    if (!result) return NULL;
    
    /* Perform split */
    start = private->data;
    
    for (i = 0; i < private->length; i++) {
        if (strchr(chars, private->data[i])) {
            size_t part_len;
            StringPrivate* part_priv;
            
            part_len = (private->data + i) - start;
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
            start = private->data + i + 1;
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

static TF_Unary(String**, string_split_lines, String, StringPrivate, size_t*, out_count)
    (void)private; /* Suppress unused warning */
    return string_split_any(self, "\r\n", out_count);
}

static TF_Dyadic(String*, string_join, String, StringPrivate, String**, strings, size_t, count)
    size_t total_len = 0;
    size_t i;
    String* result;
    
    if (!strings || count == 0) return StringMake("");
    
    /* Calculate total length */
    for (i = 0; i < count; i++) {
        if (strings[i]) {
            total_len += strings[i]->length();
            if (i < count - 1) {
                total_len += private->length;
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
                result->append(private->data);
            }
        }
    }
    
    return result;
}

/* ======================================================================== */
/* String Comparison Functions                                              */
/* ======================================================================== */

static TF_Unary(int, string_compare, String, StringPrivate, const char*, other)
    if (!other) return 1;
    return strcmp(private->data, other);
}

static TF_Unary(int, string_compare_ignore_case, String, StringPrivate, const char*, other)
    const char* s1;
    const char* s2;
    
    if (!other) return 1;
    
    s1 = private->data;
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

static TF_Unary(bool, string_equals, String, StringPrivate, const char*, other)
    (void)private; /* Suppress unused warning */
    return string_compare(self, other) == 0;
}

static TF_Unary(bool, string_equals_ignore_case, String, StringPrivate, const char*, other)
    (void)private; /* Suppress unused warning */
    return string_compare_ignore_case(self, other) == 0;
}

/* ======================================================================== */
/* String Utilities                                                         */
/* ======================================================================== */

static TF_Getter(string_is_integer, String, StringPrivate, bool)
    char* endptr;
    
    if (private->length == 0) return false;
    
    strtol(private->data, &endptr, 10);
    return *endptr == '\0';
}

static TF_Getter(string_is_float, String, StringPrivate, bool)
    char* endptr;
    
    if (private->length == 0) return false;
    
    strtod(private->data, &endptr);
    return *endptr == '\0';
}

static TF_Getter(string_is_alpha, String, StringPrivate, bool)
    size_t i;
    
    if (private->length == 0) return false;
    
    for (i = 0; i < private->length; i++) {
        if (!isalpha((unsigned char)private->data[i])) {
            return false;
        }
    }
    return true;
}

static TF_Getter(string_is_digit, String, StringPrivate, bool)
    size_t i;
    
    if (private->length == 0) return false;
    
    for (i = 0; i < private->length; i++) {
        if (!isdigit((unsigned char)private->data[i])) {
            return false;
        }
    }
    return true;
}

static TF_Getter(string_is_alpha_numeric, String, StringPrivate, bool)
    size_t i;
    
    if (private->length == 0) return false;
    
    for (i = 0; i < private->length; i++) {
        if (!isalnum((unsigned char)private->data[i])) {
            return false;
        }
    }
    return true;
}

static TF_Getter(string_is_whitespace, String, StringPrivate, bool)
    size_t i;
    
    if (private->length == 0) return true;
    
    for (i = 0; i < private->length; i++) {
        if (!char_is_whitespace(private->data[i])) {
            return false;
        }
    }
    return true;
}

static TF_Unary(int, string_to_int, String, StringPrivate, int, default_value)
    char* endptr;
    long value;
    
    if (private->length == 0) return default_value;
    
    value = strtol(private->data, &endptr, 10);
    if (*endptr != '\0') return default_value;
    if (value > INT_MAX || value < INT_MIN) return default_value;
    
    return (int)value;
}

static TF_Unary(float, string_to_float, String, StringPrivate, float, default_value)
    char* endptr;
    float value;
    
    if (private->length == 0) return default_value;
    
    value = strtof(private->data, &endptr);
    if (*endptr != '\0') return default_value;
    
    return value;
}

static TF_Unary(double, string_to_double, String, StringPrivate, double, default_value)
    char* endptr;
    double value;
    
    if (private->length == 0) return default_value;
    
    value = strtod(private->data, &endptr);
    if (*endptr != '\0') return default_value;
    
    return value;
}

static TF_Getter(string_hash, String, StringPrivate, size_t)
    size_t hash = 5381;
    size_t i;
    
    for (i = 0; i < private->length; i++) {
        hash = ((hash << 5) + hash) + private->data[i];
    }
    
    return hash;
}

static TF_Getter(string_to_string, String, StringPrivate, String*)
    (void)private; /* Suppress unused warning */
    return string_clone(self);
}

/* ======================================================================== */
/* Memory Management Functions                                              */
/* ======================================================================== */

static TF_Unary(bool, string_reserve, String, StringPrivate, size_t, new_capacity)
    return string_ensure_capacity(private, new_capacity);
}

static TF_Getter(string_shrink_to_fit, String, StringPrivate, bool)
    char* new_data;
    size_t new_capacity = private->length + 1;
    
    if (new_capacity >= private->capacity) return true;
    
    new_data = realloc(private->data, new_capacity);
    if (!new_data) return false;
    
    private->data = new_data;
    private->capacity = new_capacity;
    return true;
}

static TF_Nullary(string_free, String, StringPrivate)
    if (private) {
        if (private->data) {
            free(private->data);
        }
        trampoline_tracker_free_by_context(self);
        free(private);
    }
}

/* ======================================================================== */
/* String Creation Functions                                                */
/* ======================================================================== */

static String* string_make_internal(const char* str, size_t initial_capacity) {
    size_t str_len = str ? strlen(str) : 0;
    if (initial_capacity < str_len + 1) {
        initial_capacity = str_len + 1;
    }
    
    /* Use new TA_Allocate macro */
    TA_Allocate(String, StringPrivate);
    
    if (!private) return NULL;
    
    /* Allocate string buffer */
    private->data = calloc(initial_capacity, 1);
    if (!private->data) {
        free(private);
        return NULL;
    }
    
    /* Initialize fields */
    if (str) {
        memcpy(private->data, str, str_len);
        private->length = str_len;
    }
    private->capacity = initial_capacity;
    
    /* Create trampoline functions using trampoline_monitor */
    /* Core access */
    public->cStr = trampoline_monitor(string_c_str, public, 0, &tracker);
    public->length = trampoline_monitor(string_length, public, 0, &tracker);
    public->capacity = trampoline_monitor(string_capacity, public, 0, &tracker);
    public->isEmpty = trampoline_monitor(string_is_empty, public, 0, &tracker);
    public->charAt = trampoline_monitor(string_char_at, public, 1, &tracker);
    
    /* Modification */
    public->append = trampoline_monitor(string_append, public, 1, &tracker);
    public->appendChar = trampoline_monitor(string_append_char, public, 1, &tracker);
    public->appendFormat = trampoline_monitor(string_append_format, public, 2, &tracker);
    public->prepend = trampoline_monitor(string_prepend, public, 1, &tracker);
    public->insert = trampoline_monitor(string_insert, public, 2, &tracker);
    public->replace = trampoline_monitor(string_replace, public, 2, &tracker);
    public->replaceFirst = trampoline_monitor(string_replace_first, public, 2, &tracker);
    public->clear = trampoline_monitor(string_clear, public, 0, &tracker);
    public->set = trampoline_monitor(string_set, public, 1, &tracker);
    public->reverse = trampoline_monitor(string_reverse, public, 0, &tracker);
    public->toUpperCaseInPlace = trampoline_monitor(string_to_upper_case_in_place, public, 0, &tracker);
    public->toLowerCaseInPlace = trampoline_monitor(string_to_lower_case_in_place, public, 0, &tracker);
    
    /* Creation */
    public->substring = trampoline_monitor(string_substring, public, 2, &tracker);
    public->trim = trampoline_monitor(string_trim, public, 0, &tracker);
    public->trimLeft = trampoline_monitor(string_trim_left, public, 0, &tracker);
    public->trimRight = trampoline_monitor(string_trim_right, public, 0, &tracker);
    public->toUpperCase = trampoline_monitor(string_to_upper_case, public, 0, &tracker);
    public->toLowerCase = trampoline_monitor(string_to_lower_case, public, 0, &tracker);
    public->clone = trampoline_monitor(string_clone, public, 0, &tracker);
    public->repeat = trampoline_monitor(string_repeat, public, 1, &tracker);
    
    /* Searching */
    public->contains = trampoline_monitor(string_contains, public, 1, &tracker);
    public->startsWith = trampoline_monitor(string_starts_with, public, 1, &tracker);
    public->endsWith = trampoline_monitor(string_ends_with, public, 1, &tracker);
    public->indexOf = trampoline_monitor(string_index_of, public, 1, &tracker);
    public->lastIndexOf = trampoline_monitor(string_last_index_of, public, 1, &tracker);
    public->indexOfAny = trampoline_monitor(string_index_of_any, public, 1, &tracker);
    public->count = trampoline_monitor(string_count, public, 1, &tracker);
    
    /* Splitting */
    public->split = trampoline_monitor(string_split, public, 2, &tracker);
    public->splitAny = trampoline_monitor(string_split_any, public, 2, &tracker);
    public->splitLines = trampoline_monitor(string_split_lines, public, 1, &tracker);
    public->join = trampoline_monitor(string_join, public, 2, &tracker);
    
    /* Comparison */
    public->compare = trampoline_monitor(string_compare, public, 1, &tracker);
    public->compareIgnoreCase = trampoline_monitor(string_compare_ignore_case, public, 1, &tracker);
    public->equals = trampoline_monitor(string_equals, public, 1, &tracker);
    public->equalsIgnoreCase = trampoline_monitor(string_equals_ignore_case, public, 1, &tracker);
    
    /* Utilities */
    public->isInteger = trampoline_monitor(string_is_integer, public, 0, &tracker);
    public->isFloat = trampoline_monitor(string_is_float, public, 0, &tracker);
    public->isAlpha = trampoline_monitor(string_is_alpha, public, 0, &tracker);
    public->isDigit = trampoline_monitor(string_is_digit, public, 0, &tracker);
    public->isAlphaNumeric = trampoline_monitor(string_is_alpha_numeric, public, 0, &tracker);
    public->isWhitespace = trampoline_monitor(string_is_whitespace, public, 0, &tracker);
    public->toInt = trampoline_monitor(string_to_int, public, 1, &tracker);
    public->toFloat = trampoline_monitor(string_to_float, public, 1, &tracker);
    public->toDouble = trampoline_monitor(string_to_double, public, 1, &tracker);
    public->hash = trampoline_monitor(string_hash, public, 0, &tracker);
    public->toString = trampoline_monitor(string_to_string, public, 0, &tracker);
    
    /* Memory management */
    public->reserve = trampoline_monitor(string_reserve, public, 1, &tracker);
    public->shrinkToFit = trampoline_monitor(string_shrink_to_fit, public, 0, &tracker);
    public->free = trampoline_monitor(string_free, public, 0, &tracker);
    
    /* Validate all trampolines were created successfully */
    if (!trampoline_validate(tracker)) {
        free(private->data);
        free(private);
        return NULL;
    }
    
    return public;
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