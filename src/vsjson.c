/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tomas@halman.net> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Tomas Halman
 * ----------------------------------------------------------------------------
 */

#include "vsjson.h"
#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>

struct vsjson {
    int state;
    const char *cursor;
    char *text;
    char *token;
    int tokensize;
};

vsjson *vsjson_new (const char *json)
{
    if (!json) return NULL;
    vsjson *self = (vsjson *)malloc (sizeof (vsjson));
    if (!self) return NULL;
    
    memset(self, 0, sizeof(vsjson));
    self->text = strdup (json);
    return self;
}

const char *_vsjson_set_token (vsjson *self, const char *ptr, size_t len)
{
    if (!ptr || !self) return NULL;
    
    if (!len) len = strlen (ptr);
    if (self->tokensize > len + 1) {
        // fits in
        strncpy (self->token, ptr, len);
        self->token[len] = 0;
        return self->token;
    }
    if (self->token) {
        free (self->token);
        self->token = NULL;
        self->tokensize = 0;
    }
    self->token = (char *) malloc (len + 1);
    if (!self->token) return NULL;
    strncpy (self->token, ptr, len);
    self->token[len] = 0;
    self->tokensize = len+1;
    return self->token;
}

const char* _vsjson_seek_to_next_token(vsjson *self)
{
    if (!self) return NULL;
    
    while (true) {
        if (self->cursor == NULL) return NULL;
        if (! isspace (self->cursor[0])) return self->cursor;
        self->cursor++;
    }
}

const char* _vsjson_find_next_token(vsjson *self, const char *start)
{
    if (!self) return NULL;
    
    const char *p = start;
    if (!start) p = self->text;
    while (true) {
        if (*p == 0) return NULL;
        if (!isspace(*p)) return p;
        p++;
    }
}

const char* _vsjson_find_string_end(vsjson *self, const char *start)
{
    if (!self || !start) return NULL;

    const char *p = start;
    if (*p != '"') return NULL;
    ++p;
    while (true) {
        switch(*p) {
        case 0:
            return NULL;
        case '\\':
            ++p;
            if (*p == 0) return NULL;
            break;
        case '"':
                return ++p;
        }
        ++p;
    }
}

const char* _vsjson_find_number_end(vsjson *self, const char *start)
{
    if (!self || !start) return NULL;

    const char *p = start;
    if (!(isdigit (*p) || *p == '-' || *p  == '+')) return NULL;
    ++p;
    while (true) {
        if (*p == 0) return NULL;
        if(isdigit (*p) || *p == '.' || *p == 'e' || *p == 'E' || *p == '-' || *p == '+') {
            ++p;
        } else {
            return p;
        }
    }
}

const char* _vsjson_find_keyword_end(vsjson *self, const char *start)
{
    if (!self || !start) return NULL;

    const char *p = start;
    if (!isalpha (*p)) return NULL;
    ++p;
    while (true) {
        if (*p == 0) return NULL;
        if(isalpha (*p)) {
            ++p;
        } else {
            return p;
        }
    }
}

const char* _vsjson_find_token_end(vsjson *self, const char *start)
{
    if (!self || !start) return NULL;

    const char *p = start;
    if (strchr ("{}[]:,",*p)) {
        return ++p;
    }
    if (*p == '"') {
        return _vsjson_find_string_end (self, p);
    }
    if (strchr ("+-0123456789", *p)) {
        return _vsjson_find_number_end (self, p);
    }
    if (isalpha (*p)) {
        return _vsjson_find_keyword_end (self, p);
    }
    return NULL;
}

const char* vsjson_first_token (vsjson *self)
{
    if (!self) return NULL;
    self->cursor = _vsjson_find_next_token (self, NULL);
    if (!self->cursor) return NULL;
    const char *p = _vsjson_find_token_end (self, self->cursor);
    if (p) {
        _vsjson_set_token (self, self->cursor, p - self->cursor);
        self->cursor = p;
        return self->token;
    }
    return NULL;
}

const char* vsjson_next_token (vsjson *self)
{
    if (!self) return NULL;
    self->cursor = _vsjson_find_next_token (self, self->cursor);
    if (!self->cursor) return NULL;
    const char *p = _vsjson_find_token_end (self, self->cursor);
    if (p) {
        _vsjson_set_token (self, self->cursor, p - self->cursor);
        self->cursor = p;
        return self->token;
    }
    return NULL;
}

void vsjson_destroy (vsjson **self_p)
{
    if (!self_p) return;
    if (!*self_p) return;
    vsjson *self = *self_p;
    if (self->text) free (self->text);
    if (self->token) free (self->token);
    free (self);
    *self_p = NULL;
}

int _vsjson_walk_array (vsjson *self, const char *prefix, vsjson_callback_t *func, void *data);

int _vsjson_walk_object (vsjson *self, const char *prefix, vsjson_callback_t *func, void *data)
{
    int result = 0;
    char *locator = NULL;
    char *key;

    const char *token = vsjson_next_token (self);
    while (token) {
        // token should be key or }
        switch (token[0]) {
        case '}':
            goto cleanup;
        case '"':
            // TODO
            key = strdup (token);
            token = vsjson_next_token (self);
            if (strcmp (token, ":") != 0) {
                result = -1;
                goto cleanup;
            }
            token = vsjson_next_token (self);
            if (!token) {
                result = -1;
                goto cleanup;
            }
            asprintf(&locator, "%s%s%s", prefix, VSJSON_SEPARATOR, key);
            switch (token[0]) {
            case '{':
                _vsjson_walk_object (self, locator, func, data);
                break;
            case '[':
                _vsjson_walk_array (self, locator, func, data);
                break;
            case ':':
            case ',':
            case '}':
            case ']':
                result = -1;
                goto cleanup;
            default:
                // this is the value
                func (locator, token, data);
                break;
            }
            free (locator);
            locator = NULL;
            free (key);
            key = NULL;
            break;
        default:
            // this is wrong
            result = -1;
            goto cleanup;
        }
        token = vsjson_next_token (self);
        // now the token can be only '}' or ','
        if (!token) {
            result = -1;
            goto cleanup;
        }
        switch (token[0]) {
        case ',':
            token = vsjson_next_token (self);
            break;
        case '}':
            break;
        default:
            result = -1;
            goto cleanup;
        }
    }
 cleanup:
    if (locator) free (locator);
    if (key) free (key);
    return result;
}

int _vsjson_walk_array (vsjson *self, const char *prefix, vsjson_callback_t *func, void *data)
{
    int index = 0;
    int result = 0;
    char *locator = NULL;

    const char *token = vsjson_next_token (self);
    while (token) {
        asprintf(&locator, "%s%s%i", prefix, VSJSON_SEPARATOR, index);
        // token should be value or ]
        switch (token[0]) {
        case ']':
            goto cleanup;
        case ':':
        case ',':
        case '}':
            result = -1;
            goto cleanup;
        case '{':
            _vsjson_walk_object (self, locator, func, data);
            break;
        case '[':
            _vsjson_walk_array (self, locator, func, data);
            break;
        default:
            func (locator, token, data);
            break;
        }
        free (locator);
        locator = NULL;

        token = vsjson_next_token (self);
        // now the token can be only ']' or ','
        if (!token) {
            result = -1;
            goto cleanup;
        }
        switch (token[0]) {
        case ',':
            token = vsjson_next_token (self);
            ++index;
            break;
        case ']':
            break;
        default:
            result = -1;
            goto cleanup;
        }
    }
 cleanup:
    if (locator) free (locator);
    return result;
}

int vsjson_walk_trough (vsjson *self, vsjson_callback_t *func, void *data)
{
    if (!self || !func) return -1;

    int result = 0;
    
    const char *token = vsjson_first_token (self);
    if (token) {
        switch (token[0]) {
        case '{':
            result = _vsjson_walk_object (self, "", func, data);
            break;
        case '[':
            result = _vsjson_walk_array (self, "", func, data);
            break;
        default:
            // this is bad
            result = -1;
            break;
        }
    }
    token = vsjson_next_token (self);
    if (token) result = -1;
    return result;
}

char *vsjson_decode_string (const char *string)
{
    if (!string) return NULL;
    return strdup (string);
}


char *vsjson_encode_string (const char *string)
{
    if (!string) return NULL;
    return strdup (string);
}

