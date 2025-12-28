/* ======================================================================== */
/* JSON Trampoline Class Implementation                  */
/* ======================================================================== */

#include <trampoline/trampoline.h>
#include <trampoline/macros.h>
#include <trampoline/classes/json.h>
#include <trampoline/classes/string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>

/* ======================================================================== */
/* Private Structures                            */
/* ======================================================================== */

typedef struct JsonValue JsonValue;
typedef struct JsonPair JsonPair;

struct JsonValue {
  JsonType type;
  union {
    bool boolean;
    double number;
    char* string;
    JsonValue** array;
    JsonPair* object;
  } data;
  size_t size;    /* For arrays and objects */
  size_t capacity;  /* For arrays and objects */
};

struct JsonPair {
  char* key;
  JsonValue* value;
  JsonPair* next;
};

typedef struct {
  Json public;
  JsonValue* value;
} JsonPrivate;

typedef struct {
  JsonArray public;
  JsonValue* value;  /* Must be array type */
} JsonArrayPrivate;

typedef struct {
  JsonObject public;
  JsonValue* value;  /* Must be object type */
} JsonObjectPrivate;

/* ======================================================================== */
/* Forward Declarations                          */
/* ======================================================================== */

static JsonValue* json_value_create(JsonType type);
static void json_value_free(JsonValue* value);
static JsonValue* json_value_clone(JsonValue* value);
static bool json_value_equals(JsonValue* a, JsonValue* b);
static char* json_value_stringify(JsonValue* value, int indent, int current_depth);

/* Parser functions */
static JsonValue* parse_value(const char** ptr);
static JsonValue* parse_object(const char** ptr);
static JsonValue* parse_array(const char** ptr);
static JsonValue* parse_string(const char** ptr);
static JsonValue* parse_number(const char** ptr);
static JsonValue* parse_literal(const char** ptr, const char* literal, JsonValue* result);
static void skip_whitespace(const char** ptr);
static char* parse_string_value(const char** ptr);

/* ======================================================================== */
/* Helper Functions                            */
/* ======================================================================== */

static JsonValue* json_value_create(JsonType type) {
  JsonValue* value = calloc(1, sizeof(JsonValue));
  if (!value) return NULL;
  value->type = type;
  return value;
}

static void json_value_free(JsonValue* value) {
  size_t i;
  JsonPair *pair, *next;

  if (!value) return;

  switch (value->type) {
    case JSON_STRING:
      free(value->data.string);
      break;

    case JSON_ARRAY:
      for (i = 0; i < value->size; i++) {
        json_value_free(value->data.array[i]);
      }
      free(value->data.array);
      break;

    case JSON_OBJECT:
      pair = value->data.object;
      while (pair) {
        next = pair->next;
        free(pair->key);
        json_value_free(pair->value);
        free(pair);
        pair = next;
      }
      break;

    default:
      break;
  }

  free(value);
}

static JsonValue* json_value_clone(JsonValue* value) {
  JsonValue* clone;
  size_t i;
  JsonPair *pair, *new_pair, **tail;

  if (!value) return NULL;

  clone = json_value_create(value->type);
  if (!clone) return NULL;

  switch (value->type) {
    case JSON_BOOL:
      clone->data.boolean = value->data.boolean;
      break;

    case JSON_NUMBER:
      clone->data.number = value->data.number;
      break;

    case JSON_STRING:
      clone->data.string = strdup(value->data.string);
      if (!clone->data.string) {
        free(clone);
        return NULL;
      }
      break;

    case JSON_ARRAY:
      clone->capacity = value->size;
      clone->size = value->size;
      if (clone->size > 0) {
        clone->data.array = calloc(clone->capacity, sizeof(JsonValue*));
        if (!clone->data.array) {
          free(clone);
          return NULL;
        }
        for (i = 0; i < value->size; i++) {
          clone->data.array[i] = json_value_clone(value->data.array[i]);
          if (!clone->data.array[i]) {
            clone->size = i;
            json_value_free(clone);
            return NULL;
          }
        }
      }
      break;

    case JSON_OBJECT:
      clone->size = value->size;
      tail = &clone->data.object;
      pair = value->data.object;
      while (pair) {
        new_pair = malloc(sizeof(JsonPair));
        if (!new_pair) {
          json_value_free(clone);
          return NULL;
        }
        new_pair->key = strdup(pair->key);
        new_pair->value = json_value_clone(pair->value);
        new_pair->next = NULL;

        if (!new_pair->key || !new_pair->value) {
          free(new_pair->key);
          json_value_free(new_pair->value);
          free(new_pair);
          json_value_free(clone);
          return NULL;
        }

        *tail = new_pair;
        tail = &new_pair->next;
        pair = pair->next;
      }
      break;

    default:
      break;
  }

  return clone;
}

static bool json_value_equals(JsonValue* a, JsonValue* b) {
  size_t i;
  JsonPair *pa, *pb;

  if (!a && !b) return true;
  if (!a || !b) return false;
  if (a->type != b->type) return false;

  switch (a->type) {
    case JSON_NULL:
      return true;

    case JSON_BOOL:
      return a->data.boolean == b->data.boolean;

    case JSON_NUMBER:
      return fabs(a->data.number - b->data.number) < 1e-10;

    case JSON_STRING:
      return strcmp(a->data.string, b->data.string) == 0;

    case JSON_ARRAY:
      if (a->size != b->size) return false;
      for (i = 0; i < a->size; i++) {
        if (!json_value_equals(a->data.array[i], b->data.array[i])) {
          return false;
        }
      }
      return true;

    case JSON_OBJECT:
      if (a->size != b->size) return false;
      pa = a->data.object;
      while (pa) {
        /* Find matching key in b */
        pb = b->data.object;
        while (pb) {
          if (strcmp(pa->key, pb->key) == 0) {
            if (!json_value_equals(pa->value, pb->value)) {
              return false;
            }
            break;
          }
          pb = pb->next;
        }
        if (!pb) return false; /* Key not found in b */
        pa = pa->next;
      }
      return true;
  }

  return false;
}

/* ======================================================================== */
/* JSON Parser Implementation                        */
/* ======================================================================== */

static void skip_whitespace(const char** ptr) {
  while (**ptr && isspace((unsigned char)**ptr)) {
    (*ptr)++;
  }
}

static char* parse_string_value(const char** ptr) {
  const char* p;
  char* result;
  char* dst;
  size_t len;
  unsigned int hex;

  if (**ptr != '"') return NULL;
  (*ptr)++;
  p = *ptr;

  /* Find end of string and calculate length */
  len = 0;
  while (*p && *p != '"') {
    if (*p == '\\') {
      p++;
      if (*p) p++;
    } else {
      p++;
    }
    len++;
  }

  if (*p != '"') return NULL;

  /* Allocate result with extra space for escapes */
  result = malloc(len + 1);
  if (!result) return NULL;

  /* Copy and process escapes */
  dst = result;
  while (**ptr && **ptr != '"') {
    if (**ptr == '\\') {
      (*ptr)++;
      switch (**ptr) {
        case '"': *dst++ = '"'; break;
        case '\\': *dst++ = '\\'; break;
        case '/': *dst++ = '/'; break;
        case 'b': *dst++ = '\b'; break;
        case 'f': *dst++ = '\f'; break;
        case 'n': *dst++ = '\n'; break;
        case 'r': *dst++ = '\r'; break;
        case 't': *dst++ = '\t'; break;
        case 'u':
          /* Simple unicode escape - just store as UTF-8 if ASCII */
          (*ptr)++;
          if (sscanf(*ptr, "%4x", &hex) == 1) {
            if (hex < 128) {
              *dst++ = (char)hex;
            } else {
              /* Skip non-ASCII unicode for simplicity */
              *dst++ = '?';
            }
            *ptr += 3;
          }
          break;
        default:
          *dst++ = **ptr;
          break;
      }
      (*ptr)++;
    } else {
      *dst++ = **ptr;
      (*ptr)++;
    }
  }
  *dst = '\0';

  if (**ptr == '"') (*ptr)++;

  return result;
}

static JsonValue* parse_string(const char** ptr) {
  JsonValue* value;
  char* str;

  str = parse_string_value(ptr);
  if (!str) return NULL;

  value = json_value_create(JSON_STRING);
  if (!value) {
    free(str);
    return NULL;
  }

  value->data.string = str;
  return value;
}

static JsonValue* parse_number(const char** ptr) {
  JsonValue* value;
  char* end;
  double num;

  errno = 0;
  num = strtod(*ptr, &end);

  if (end == *ptr || errno != 0) {
    return NULL;
  }

  value = json_value_create(JSON_NUMBER);
  if (!value) return NULL;

  value->data.number = num;
  *ptr = end;

  return value;
}

static JsonValue* parse_literal(const char** ptr, const char* literal, JsonValue* result) {
  size_t len = strlen(literal);

  if (strncmp(*ptr, literal, len) == 0) {
    *ptr += len;
    return result;
  }

  json_value_free(result);
  return NULL;
}

static JsonValue* parse_array(const char** ptr) {
  JsonValue* array;
  JsonValue* element;
  JsonValue** new_data;

  if (**ptr != '[') return NULL;
  (*ptr)++;

  array = json_value_create(JSON_ARRAY);
  if (!array) return NULL;

  skip_whitespace(ptr);

  if (**ptr == ']') {
    (*ptr)++;
    return array;
  }

  while (1) {
    element = parse_value(ptr);
    if (!element) {
      json_value_free(array);
      return NULL;
    }

    /* Grow array if needed */
    if (array->size >= array->capacity) {
      size_t new_capacity = array->capacity ? array->capacity * 2 : 4;
      new_data = realloc(array->data.array, new_capacity * sizeof(JsonValue*));
      if (!new_data) {
        json_value_free(element);
        json_value_free(array);
        return NULL;
      }
      array->data.array = new_data;
      array->capacity = new_capacity;
    }

    array->data.array[array->size++] = element;

    skip_whitespace(ptr);

    if (**ptr == ']') {
      (*ptr)++;
      return array;
    }

    if (**ptr != ',') {
      json_value_free(array);
      return NULL;
    }
    (*ptr)++;
    skip_whitespace(ptr);
  }
}

static JsonValue* parse_object(const char** ptr) {
  JsonValue* object;
  JsonPair* pair;
  JsonPair** tail;
  char* key;
  JsonValue* value;

  if (**ptr != '{') return NULL;
  (*ptr)++;

  object = json_value_create(JSON_OBJECT);
  if (!object) return NULL;

  skip_whitespace(ptr);

  if (**ptr == '}') {
    (*ptr)++;
    return object;
  }

  tail = &object->data.object;

  while (1) {
    skip_whitespace(ptr);

    /* Parse key */
    key = parse_string_value(ptr);
    if (!key) {
      json_value_free(object);
      return NULL;
    }

    skip_whitespace(ptr);

    if (**ptr != ':') {
      free(key);
      json_value_free(object);
      return NULL;
    }
    (*ptr)++;

    skip_whitespace(ptr);

    /* Parse value */
    value = parse_value(ptr);
    if (!value) {
      free(key);
      json_value_free(object);
      return NULL;
    }

    /* Create pair */
    pair = malloc(sizeof(JsonPair));
    if (!pair) {
      free(key);
      json_value_free(value);
      json_value_free(object);
      return NULL;
    }

    pair->key = key;
    pair->value = value;
    pair->next = NULL;

    *tail = pair;
    tail = &pair->next;
    object->size++;

    skip_whitespace(ptr);

    if (**ptr == '}') {
      (*ptr)++;
      return object;
    }

    if (**ptr != ',') {
      json_value_free(object);
      return NULL;
    }
    (*ptr)++;
  }
}

static JsonValue* parse_value(const char** ptr) {
  JsonValue* null_val;
  JsonValue* true_val;
  JsonValue* false_val;

  skip_whitespace(ptr);

  switch (**ptr) {
    case '{':
      return parse_object(ptr);

    case '[':
      return parse_array(ptr);

    case '"':
      return parse_string(ptr);

    case 't':
      true_val = json_value_create(JSON_BOOL);
      if (!true_val) return NULL;
      true_val->data.boolean = true;
      return parse_literal(ptr, "true", true_val);

    case 'f':
      false_val = json_value_create(JSON_BOOL);
      if (!false_val) return NULL;
      false_val->data.boolean = false;
      return parse_literal(ptr, "false", false_val);

    case 'n':
      null_val = json_value_create(JSON_NULL);
      return parse_literal(ptr, "null", null_val);

    case '-':
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      return parse_number(ptr);

    default:
      return NULL;
  }
}

/* ======================================================================== */
/* Stringify Implementation                        */
/* ======================================================================== */

static void append_string(String* str, const char* s) {
  const char* p = s;

  str->append("\"");

  while (*p) {
    switch (*p) {
      case '"': str->append("\\\""); break;
      case '\\': str->append("\\\\"); break;
      case '\b': str->append("\\b"); break;
      case '\f': str->append("\\f"); break;
      case '\n': str->append("\\n"); break;
      case '\r': str->append("\\r"); break;
      case '\t': str->append("\\t"); break;
      default:
        if ((unsigned char)*p < 32) {
          char buf[7];
          sprintf(buf, "\\u%04x", (unsigned char)*p);
          str->append(buf);
        } else {
          str->appendChar(*p);
        }
        break;
    }
    p++;
  }

  str->append("\"");
}

static char* json_value_stringify(JsonValue* value, int indent, int current_depth) {
  String* str;
  char* result;
  char number_buf[64];
  size_t i;
  JsonPair* pair;
  int j;

  if (!value) return strdup("null");

  str = StringMake("");
  if (!str) return NULL;

  switch (value->type) {
    case JSON_NULL:
      str->append("null");
      break;

    case JSON_BOOL:
      str->append(value->data.boolean ? "true" : "false");
      break;

    case JSON_NUMBER:
      sprintf(number_buf, "%.17g", value->data.number);
      str->append(number_buf);
      break;

    case JSON_STRING:
      append_string(str, value->data.string);
      break;

    case JSON_ARRAY:
      str->append("[");
      if (indent > 0 && value->size > 0) {
        for (i = 0; i < value->size; i++) {
          str->append("\n");
          for (j = 0; j < (current_depth + 1) * indent; j++) {
            str->append(" ");
          }

          result = json_value_stringify(value->data.array[i], indent, current_depth + 1);
          if (result) {
            str->append(result);
            free(result);
          }

          if (i < value->size - 1) {
            str->append(",");
          }
        }
        str->append("\n");
        for (j = 0; j < current_depth * indent; j++) {
          str->append(" ");
        }
      } else {
        for (i = 0; i < value->size; i++) {
          result = json_value_stringify(value->data.array[i], 0, 0);
          if (result) {
            str->append(result);
            free(result);
          }
          if (i < value->size - 1) {
            str->append(",");
          }
        }
      }
      str->append("]");
      break;

    case JSON_OBJECT:
      str->append("{");
      pair = value->data.object;
      if (indent > 0 && pair) {
        while (pair) {
          str->append("\n");
          for (j = 0; j < (current_depth + 1) * indent; j++) {
            str->append(" ");
          }

          append_string(str, pair->key);
          str->append(": ");

          result = json_value_stringify(pair->value, indent, current_depth + 1);
          if (result) {
            str->append(result);
            free(result);
          }

          if (pair->next) {
            str->append(",");
          }
          pair = pair->next;
        }
        str->append("\n");
        for (j = 0; j < current_depth * indent; j++) {
          str->append(" ");
        }
      } else {
        while (pair) {
          append_string(str, pair->key);
          str->append(":");

          result = json_value_stringify(pair->value, 0, 0);
          if (result) {
            str->append(result);
            free(result);
          }

          if (pair->next) {
            str->append(",");
          }
          pair = pair->next;
        }
      }
      str->append("}");
      break;
  }

  result = strdup(str->cStr());
  str->free();

  return result;
}

/* ======================================================================== */
/* Json Class Implementation                         */
/* ======================================================================== */

static TF_Getter(json_type, Json, JsonPrivate, JsonType)
  return private->value ? private->value->type : JSON_NULL;
}

static TF_Getter(json_isNull, Json, JsonPrivate, bool)
  return private->value && private->value->type == JSON_NULL;
}

static TF_Getter(json_isBool, Json, JsonPrivate, bool)
  return private->value && private->value->type == JSON_BOOL;
}

static TF_Getter(json_isNumber, Json, JsonPrivate, bool)
  return private->value && private->value->type == JSON_NUMBER;
}

static TF_Getter(json_isString, Json, JsonPrivate, bool)
  return private->value && private->value->type == JSON_STRING;
}

static TF_Getter(json_isArray, Json, JsonPrivate, bool)
  return private->value && private->value->type == JSON_ARRAY;
}

static TF_Getter(json_isObject, Json, JsonPrivate, bool)
  return private->value && private->value->type == JSON_OBJECT;
}

static TF_Getter(json_getBool, Json, JsonPrivate, bool)
  if (private->value && private->value->type == JSON_BOOL) {
    return private->value->data.boolean;
  }
  return false;
}

static TF_Getter(json_getNumber, Json, JsonPrivate, double)
  if (private->value && private->value->type == JSON_NUMBER) {
    return private->value->data.number;
  }
  return 0.0;
}

static TF_Getter(json_getString, Json, JsonPrivate, const char*)
  if (private->value && private->value->type == JSON_STRING) {
    return private->value->data.string;
  }
  return NULL;
}

static TF_Getter(json_getArray, Json, JsonPrivate, JsonArray*)
  (void)private; /* Suppress unused warning */
  /* JsonArray wrapper not yet implemented */
  return NULL;
}

static TF_Getter(json_getObject, Json, JsonPrivate, JsonObject*)
  (void)private; /* Suppress unused warning */
  /* JsonObject wrapper not yet implemented */
  return NULL;
}

static TF_Nullary(json_setNull, Json, JsonPrivate)
  json_value_free(private->value);
  private->value = json_value_create(JSON_NULL);
}

static TF_Unary(void, json_setBool, Json, JsonPrivate, bool, value)
  json_value_free(private->value);
  private->value = json_value_create(JSON_BOOL);
  if (private->value) {
    private->value->data.boolean = value;
  }
}

static TF_Unary(void, json_setNumber, Json, JsonPrivate, double, value)
  json_value_free(private->value);
  private->value = json_value_create(JSON_NUMBER);
  if (private->value) {
    private->value->data.number = value;
  }
}

static TF_Unary(void, json_setString, Json, JsonPrivate, const char*, value)
  json_value_free(private->value);
  if (value) {
    private->value = json_value_create(JSON_STRING);
    if (private->value) {
      private->value->data.string = strdup(value);
      if (!private->value->data.string) {
        free(private->value);
        private->value = NULL;
      }
    }
  } else {
    private->value = json_value_create(JSON_NULL);
  }
}

static TF_Nullary(json_setArray, Json, JsonPrivate)
  json_value_free(private->value);
  private->value = json_value_create(JSON_ARRAY);
}

static TF_Nullary(json_setObject, Json, JsonPrivate)
  json_value_free(private->value);
  private->value = json_value_create(JSON_OBJECT);
}

/* Array operations */
static TF_Getter(json_arraySize, Json, JsonPrivate, size_t)
  if (private->value && private->value->type == JSON_ARRAY) {
    return private->value->size;
  }
  return 0;
}

static TF_Unary(Json*, json_arrayGet, Json, JsonPrivate, size_t, index)
  Json* result;
  JsonPrivate* result_priv;

  if (!private->value || private->value->type != JSON_ARRAY) {
    return NULL;
  }

  if (index >= private->value->size) {
    return NULL;
  }

  /* Create wrapper for the value */
  result = malloc(sizeof(Json));
  result_priv = malloc(sizeof(JsonPrivate));

  if (!result || !result_priv) {
    free(result);
    free(result_priv);
    return NULL;
  }

  /* We need to create a new Json object wrapping the array element */
  /* For now, return NULL - proper implementation would require */
  /* creating all the trampoline functions for the new Json object */
  free(result);
  free(result_priv);
  return NULL;
}

static TF_Unary(void, json_arrayAdd, Json, JsonPrivate, Json*, value)
  JsonValue** new_data;
  JsonPrivate* value_priv;

  if (!private->value || private->value->type != JSON_ARRAY || !value) {
    return;
  }

  /* Get the JsonPrivate from the value */
  value_priv = (JsonPrivate*)((char*)value - offsetof(JsonPrivate, public));

  /* Grow array if needed */
  if (private->value->size >= private->value->capacity) {
    size_t new_capacity = private->value->capacity ? private->value->capacity * 2 : 4;
    new_data = realloc(private->value->data.array, new_capacity * sizeof(JsonValue*));
    if (!new_data) return;
    private->value->data.array = new_data;
    private->value->capacity = new_capacity;
  }

  /* Clone the value to add */
  private->value->data.array[private->value->size] = json_value_clone(value_priv->value);
  if (private->value->data.array[private->value->size]) {
    private->value->size++;
  }
}

/* Object operations */
static TF_Getter(json_objectSize, Json, JsonPrivate, size_t)
  if (private->value && private->value->type == JSON_OBJECT) {
    return private->value->size;
  }
  return 0;
}

static TF_Unary(bool, json_objectHas, Json, JsonPrivate, const char*, key)
  JsonPair* pair;

  if (!private->value || private->value->type != JSON_OBJECT || !key) {
    return false;
  }

  pair = private->value->data.object;
  while (pair) {
    if (strcmp(pair->key, key) == 0) {
      return true;
    }
    pair = pair->next;
  }

  return false;
}

static TF_Unary(Json*, json_objectGet, Json, JsonPrivate, const char*, key)
  JsonPair* pair;
  Json* result;
  JsonPrivate* result_priv;

  if (!private->value || private->value->type != JSON_OBJECT || !key) {
    return NULL;
  }

  pair = private->value->data.object;
  while (pair) {
    if (strcmp(pair->key, key) == 0) {
      /* Create wrapper for the value */
      result = malloc(sizeof(Json));
      result_priv = malloc(sizeof(JsonPrivate));

      if (!result || !result_priv) {
        free(result);
        free(result_priv);
        return NULL;
      }

      /* We need to create a new Json object wrapping the object value */
      /* For now, return NULL - proper implementation would require */
      /* creating all the trampoline functions for the new Json object */
      free(result);
      free(result_priv);
      return NULL;
    }
    pair = pair->next;
  }

  return NULL;
}

static TF_Dyadic(void, json_objectSet, Json, JsonPrivate, const char*, key, Json*, value)
  JsonPair* pair;
  JsonPair* new_pair;
  JsonPrivate* value_priv;

  if (!private->value || private->value->type != JSON_OBJECT || !key || !value) {
    return;
  }

  /* Get the JsonPrivate from the value */
  value_priv = (JsonPrivate*)((char*)value - offsetof(JsonPrivate, public));

  /* Check if key already exists */
  pair = private->value->data.object;
  while (pair) {
    if (strcmp(pair->key, key) == 0) {
      /* Replace existing value */
      json_value_free(pair->value);
      pair->value = json_value_clone(value_priv->value);
      return;
    }
    pair = pair->next;
  }

  /* Add new key-value pair */
  new_pair = malloc(sizeof(JsonPair));
  if (!new_pair) return;

  new_pair->key = strdup(key);
  new_pair->value = json_value_clone(value_priv->value);
  new_pair->next = private->value->data.object;

  if (!new_pair->key || !new_pair->value) {
    free(new_pair->key);
    json_value_free(new_pair->value);
    free(new_pair);
    return;
  }

  private->value->data.object = new_pair;
  private->value->size++;
}

static TF_Getter(json_stringify, Json, JsonPrivate, char*)
  return json_value_stringify(private->value, 0, 0);
}

static TF_Unary(char*, json_prettyPrint, Json, JsonPrivate, int, indent_size)
  return json_value_stringify(private->value, indent_size, 0);
}

static TF_Getter(json_clone, Json, JsonPrivate, Json*)
  JsonValue* cloned = json_value_clone(private->value);
  if (!cloned) return NULL;

  /* Need to create a full Json object with trampolines */
  /* For now, return NULL */
  json_value_free(cloned);
  return NULL;
}

static TF_Unary(bool, json_equals, Json, JsonPrivate, Json*, other)
  JsonPrivate* other_priv;

  if (!other) return false;

  other_priv = (JsonPrivate*)((char*)other - offsetof(JsonPrivate, public));
  return json_value_equals(private->value, other_priv->value);
}

static TF_Nullary(json_free, Json, JsonPrivate)
  if (private) {
    json_value_free(private->value);
    trampoline_tracker_free_by_context(self);
    free(private);
  }
}

/* ======================================================================== */
/* Helper to create Json objects with trampolines             */
/* ======================================================================== */

static Json* json_make_with_value(JsonValue* value) {
  TA_Allocate(Json, JsonPrivate);

  if (!value)
    return NULL;

  /* Allocate structure */
  private = calloc(1, sizeof(JsonPrivate));
  if (!private) {
    json_value_free(value);
    return NULL;
  }

  public = (Json*)private;
  private->value = value;

  /* Set up trampolines */
  /* Type inspection */
  TAFunction(type, json_type, 0);
  TAFunction(isNull, json_isNull, 0);
  TAFunction(isBool, json_isBool, 0);
  TAFunction(isNumber, json_isNumber, 0);
  TAFunction(isString, json_isString, 0);
  TAFunction(isArray, json_isArray, 0);
  TAFunction(isObject, json_isObject, 0);

  /* Value properties */
  TAProperty(getBool, setBool, json_getBool, json_setBool);
  TAProperty(getNumber, setNumber, json_getNumber, json_setNumber);
  TAProperty(getString, setString, json_getString, json_setString);
  TAProperty(getArray, setArray, json_getArray, json_setArray);
  TAProperty(getObject, setObject, json_getObject, json_setObject);

  /* Array operations */
  TAFunction(arraySize, json_arraySize, 0);
  TAFunction(arrayGet, json_arrayGet, 1);
  TAFunction(arrayAdd, json_arrayAdd, 1);

  /* Object operations */
  TAFunction(objectSize, json_objectSize, 0);
  TAFunction(objectHas, json_objectHas, 1);
  TAFunction(objectGet, json_objectGet, 1);
  TAFunction(objectSet, json_objectSet, 2);

  /* Serialization */
  TAFunction(stringify, json_stringify, 0);
  TAFunction(prettyPrint, json_prettyPrint, 1);

  /* Utility */
  TAFunction(clone, json_clone, 0);
  TAFunction(equals, json_equals, 1);
  TAFunction(free, json_free, 0);

  /* Validate all trampolines were created successfully */
  if (!trampoline_validate(tracker)) {
    json_value_free(value);
    free(private);
    return NULL;
  }

  return public;
}

/* ======================================================================== */
/* Factory Functions                             */
/* ======================================================================== */

Json* JsonMakeNull(void) {
  JsonValue* value = json_value_create(JSON_NULL);
  return json_make_with_value(value);
}

Json* JsonMakeBool(bool val) {
  JsonValue* value = json_value_create(JSON_BOOL);
  if (value) {
    value->data.boolean = val;
  }
  return json_make_with_value(value);
}

Json* JsonMakeNumber(double val) {
  JsonValue* value = json_value_create(JSON_NUMBER);
  if (value) {
    value->data.number = val;
  }
  return json_make_with_value(value);
}

Json* JsonMakeString(const char* str) {
  JsonValue* value;

  if (str) {
    value = json_value_create(JSON_STRING);
    if (value) {
      value->data.string = strdup(str);
      if (!value->data.string) {
        free(value);
        value = json_value_create(JSON_NULL);
      }
    }
  } else {
    value = json_value_create(JSON_NULL);
  }

  return json_make_with_value(value);
}

Json* JsonMakeArray(void) {
  JsonValue* value = json_value_create(JSON_ARRAY);
  return json_make_with_value(value);
}

Json* JsonMakeObject(void) {
  JsonValue* value = json_value_create(JSON_OBJECT);
  return json_make_with_value(value);
}

Json* JsonParse(const char* json_string) {
  const char* ptr;
  JsonValue* value;

  if (!json_string) return NULL;

  ptr = json_string;
  value = parse_value(&ptr);

  if (!value) return NULL;

  /* Check for trailing content */
  skip_whitespace(&ptr);
  if (*ptr != '\0') {
    json_value_free(value);
    return NULL;
  }

  return json_make_with_value(value);
}

Json* JsonParseFile(const char* filename) {
  FILE* file;
  char* buffer;
  long file_size;
  Json* result;

  if (!filename) return NULL;

  file = fopen(filename, "rb");
  if (!file) return NULL;

  /* Get file size */
  fseek(file, 0, SEEK_END);
  file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  /* Allocate buffer */
  buffer = malloc(file_size + 1);
  if (!buffer) {
    fclose(file);
    return NULL;
  }

  /* Read file */
  if (fread(buffer, 1, file_size, file) != (size_t)file_size) {
    free(buffer);
    fclose(file);
    return NULL;
  }

  buffer[file_size] = '\0';
  fclose(file);

  /* Parse JSON */
  result = JsonParse(buffer);
  free(buffer);

  return result;
}
