#include <string.h>
#include <stdio.h>

#include "include/ds/json.h"

#define OUTPUT_BUFFER_SIZE 102400

int serialize_json_at(json_t * data, char * buf, uint32_t start_at) {
    uint32_t offs, len;
    switch (data->type) {
        case JSON_NULL:
            return sprintf(buf + start_at, "null");
        case JSON_BOOLEAN:
            if (data->int_value) {
                return sprintf(buf + start_at, "true");
            } else {
                return sprintf(buf + start_at, "false");
            }
        case JSON_INTEGER:
            return sprintf(buf + start_at, "%d", data->int_value);
        case JSON_STRING:
            return sprintf(buf + start_at, "\"%s\"", data->string_value);
        case JSON_ARRAY:
            offs = sprintf(buf + start_at, "[");
            len = json_array_length(data);
            for (uint32_t i = 0; i < len; i++) {
                offs += serialize_json_at(json_array_index(data, i), buf, start_at + offs);
                if (i != len - 1) {
                    offs += sprintf(buf + start_at + offs, ",");
                }
            }
            offs += sprintf(buf + start_at + offs, "]");
            return offs;
        case JSON_OBJECT:
            offs = sprintf(buf + start_at, "{");
            json_t * array = json_object_to_array(data);
            len = json_array_length(array);
            for (uint32_t i = 0; i < len; i++) {
                json_t * kv = json_array_index(array, i);
                offs += serialize_json_at(json_array_index(kv, 0), buf, start_at + offs);
                offs += sprintf(buf + start_at + offs, ":");
                offs += serialize_json_at(json_array_index(kv, 1), buf, start_at + offs);
                if (i != len - 1) {
                    offs += sprintf(buf + start_at + offs, ",");
                }
            }
            offs += sprintf(buf + start_at + offs, "}");
            return offs;
    }
    return 0;
}

char * serialize_json(json_t * data) {
    char * buf = calloc(1, OUTPUT_BUFFER_SIZE);
    int offs = serialize_json_at(data, buf, 0);
    buf[offs] = 0;
    return buf;
}

enum {
    J_T_NULL,
    J_T_TRUE,
    J_T_FALSE,
    J_T_STRING,
    J_T_INT,
    J_T_LEFT_BRACKET,
    J_T_RIGHT_BRACKET,
    J_T_LEFT_BRACE,
    J_T_RIGHT_BRACE,
    J_T_COMMA,
    J_T_COLON
};

struct json_token {
    char * value;
    uint32_t length;
    uint8_t type;
};

int tokenize_json(char * data, uint32_t data_len, struct json_token * tokens, uint32_t * tokens_cnt) {
    const char * breakers = ",[]{} ";
    const char * digits = "1234567890";
    int token_begin = 0;
    *tokens_cnt = 0;
    for (int i = 0; i < data_len; i++) {
        char token = data[i];
        switch (token) {
            case ' ':
            case '\n':
            case '\t':
                break;
            case 'n': // null
                token_begin = i;
                while (i < data_len && strchr(breakers, data[i]) == NULL) {
                    i += 1;
                }
                if (strncmp(data + token_begin, "null", (size_t) (i - token_begin)) != 0) {
                    return -1;
                }
                i--;
                tokens[*tokens_cnt].value = data + token_begin;
                tokens[*tokens_cnt].length = 4;
                tokens[*tokens_cnt].type = J_T_NULL;
                *tokens_cnt += 1;
                break;
            case 't': // true
                token_begin = i;
                while (i < data_len && strchr(breakers, data[i]) == NULL) {
                    i += 1;
                }
                if (strncmp(data + token_begin, "true", (size_t) (i - token_begin)) != 0) {
                    return -1;
                }
                i--;
                tokens[*tokens_cnt].value = data + token_begin;
                tokens[*tokens_cnt].length = 4;
                tokens[*tokens_cnt].type = J_T_TRUE;
                *tokens_cnt += 1;
                break;
            case 'f': // false
                token_begin = i;
                while (i < data_len && strchr(breakers, data[i]) == NULL) {
                    i += 1;
                }
                if (strncmp(data + token_begin, "false", (size_t) (i - token_begin)) != 0) {
                    return -1;
                }
                i--;
                tokens[*tokens_cnt].value = data + token_begin;
                tokens[*tokens_cnt].length = 5;
                tokens[*tokens_cnt].type = J_T_FALSE;
                *tokens_cnt += 1;
                break;
            case '"': // string
                token_begin = i;
                int ok = 0;
                i++;
                while (i < data_len) {
                    int quote = data[i] == '"';
                    int escape_1 = (i - token_begin) > 0 && data[i - 1] == '\\';
                    int escape_2 = (i - token_begin) > 1 && data[i - 2] == '\\';

                    if (quote) {
                        if (!escape_1 || escape_2) {
                            ok = 1;
                            break;
                        }
                    }

                    i += 1;
                }
                if (ok != 1) {
                    return -1;
                }
                tokens[*tokens_cnt].value = data + token_begin;
                tokens[*tokens_cnt].length = (uint32_t) (i - token_begin + 1);
                tokens[*tokens_cnt].type = J_T_STRING;
                *tokens_cnt += 1;
                break;
            case '[':
                tokens[*tokens_cnt].value = data + i;
                tokens[*tokens_cnt].length = 1;
                tokens[*tokens_cnt].type = J_T_LEFT_BRACKET;
                *tokens_cnt += 1;
                break;
            case ']':
                tokens[*tokens_cnt].value = data + i;
                tokens[*tokens_cnt].length = 1;
                tokens[*tokens_cnt].type = J_T_RIGHT_BRACKET;
                *tokens_cnt += 1;
                break;
            case '{':
                tokens[*tokens_cnt].value = data + i;
                tokens[*tokens_cnt].length = 1;
                tokens[*tokens_cnt].type = J_T_LEFT_BRACE;
                *tokens_cnt += 1;
                break;
            case '}':
                tokens[*tokens_cnt].value = data + i;
                tokens[*tokens_cnt].length = 1;
                tokens[*tokens_cnt].type = J_T_RIGHT_BRACE;
                *tokens_cnt += 1;
                break;
            case ',':
                tokens[*tokens_cnt].value = data + i;
                tokens[*tokens_cnt].length = 1;
                tokens[*tokens_cnt].type = J_T_COMMA;
                *tokens_cnt += 1;
                break;
            case ':':
                tokens[*tokens_cnt].value = data + i;
                tokens[*tokens_cnt].length = 1;
                tokens[*tokens_cnt].type = J_T_COLON;
                *tokens_cnt += 1;
                break;
            default:
                if (strchr(digits, data[i]) == NULL) {
                    return -1; // not a digit
                } // a digit
                token_begin = i;
                while (i < data_len && strchr(digits, data[i]) != NULL) {
                    i += 1;
                }
                tokens[*tokens_cnt].value = data + token_begin;
                tokens[*tokens_cnt].length = (uint32_t) (i - token_begin + 1);
                tokens[*tokens_cnt].type = J_T_INT;
                *tokens_cnt += 1;
                i--;
                break;
        }
    }
    return 0;
}

/*
 * json_value ::= primitive | json_array | json_object
 * primitive ::= json_null | json_true | json_false | json_string | json_int
 *
 * json_object ::= '{' {json_string ':' json_value ','} json_string ':' json_value '}' | '{' '}'
 * json_array ::= '[' {json_value ','} json_value ']' | '[' ']'
 */
json_t * parse_json(struct json_token * tokens, uint32_t tokens_cnt, uint32_t * passed_tokens) {
    if (tokens_cnt == 0) return NULL;
    uint32_t n_passed_tokens = 0;
    switch (tokens[0].type) {
        case J_T_NULL:
            *passed_tokens = 1;
            return create_json_null();
        case J_T_FALSE:
            *passed_tokens = 1;
            return create_json_boolean(0);
        case J_T_TRUE:
            *passed_tokens = 1;
            return create_json_boolean(1);
        case J_T_INT:
            *passed_tokens = 1;
            return create_json_integer((int) strtol(tokens[0].value, NULL, 10));
        case J_T_STRING:
            *passed_tokens = 1;
            char * data = calloc(tokens[0].length - 1, 1);
            memcpy(data, tokens[0].value + 1, tokens[0].length - 2);
            data[tokens[0].length - 2] = 0;
            json_t * string = create_json_string(data);
            free(data);
            return string;
        case J_T_LEFT_BRACKET:
            *passed_tokens = 1;
            tokens_cnt--;
            tokens++;
            n_passed_tokens = 0;
            json_t * array = create_json_array();
            if (tokens[0].type == J_T_RIGHT_BRACKET) {
                *passed_tokens += 1;
                return array;
            }
            while (1) {
                json_t * element = parse_json(tokens, tokens_cnt, &n_passed_tokens);
                if (element == NULL) {
                    json_destroy(array);
                    return NULL;
                }
                json_array_append(array, element);
                tokens += n_passed_tokens;
                *passed_tokens += n_passed_tokens;
                tokens_cnt -= n_passed_tokens;
                n_passed_tokens = 0;
                if (tokens_cnt <= 0) {
                    json_destroy(array);
                    return NULL;
                }
                if (tokens[0].type == J_T_COMMA) {
                    tokens_cnt--;
                    *passed_tokens += 1;
                    tokens++;
                    continue;
                } else if (tokens[0].type == J_T_RIGHT_BRACKET) {
                    *passed_tokens += 1;
                    return array;
                } else {
                    json_destroy(array);
                    return NULL;
                }
            }
        case J_T_LEFT_BRACE:
            *passed_tokens = 1;
            tokens_cnt--;
            tokens++;
            n_passed_tokens = 0;
            json_t * object = create_json_object();
            while (1) {
                json_t * property = parse_json(tokens, tokens_cnt, &n_passed_tokens);
                if (!property) {
                    json_destroy(object);
                    return NULL;
                }
                if (property->type != JSON_STRING) {
                    json_destroy(object);
                    json_destroy(property);
                    return NULL;
                }

                tokens_cnt -= n_passed_tokens;
                tokens += n_passed_tokens;
                *passed_tokens += n_passed_tokens;
                n_passed_tokens = 0;

                if (tokens_cnt <= 0) {
                    json_destroy(object);
                    json_destroy(property);
                    return NULL;
                }

                if (tokens[0].type != J_T_COLON) {
                    json_destroy(object);
                    json_destroy(property);
                    return NULL;
                }
                tokens_cnt--;
                tokens++;
                *passed_tokens += 1;

                json_t * value = parse_json(tokens, tokens_cnt, &n_passed_tokens);
                if (!value) {
                    json_destroy(object);
                    json_destroy(property);
                    return NULL;
                }
                json_set_property(object, property->string_value, value);

                tokens_cnt -= n_passed_tokens;
                tokens += n_passed_tokens;
                *passed_tokens += n_passed_tokens;
                n_passed_tokens = 0;

                if (tokens_cnt <= 0) {
                    json_destroy(object);
                    return NULL;
                }
                if (tokens[0].type == J_T_COMMA) {
                    tokens_cnt--;
                    *passed_tokens += 1;
                    tokens++;
                    continue;
                } else if (tokens[0].type == J_T_RIGHT_BRACE) {
                    *passed_tokens += 1;
                    return object;
                } else {
                    json_destroy(object);
                    return NULL;
                }
            }
        default:
            return NULL;
    }
}

json_t * deserialize_json(char * data, uint32_t data_len) {
    struct json_token tokens[OUTPUT_BUFFER_SIZE];
    uint32_t tokens_cnt;
    tokenize_json(data, data_len, tokens, &tokens_cnt);
    uint32_t passed_tokens = 0;
    return parse_json(tokens, tokens_cnt, &passed_tokens);
}