/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tomas@halman.net> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Tomas Halman
 * ----------------------------------------------------------------------------
 * this source can be found at https://github.com/thalman/vsjson
 */

#ifndef __VSJSON_H
#define __VSJSON_H

#ifdef __cplusplus
extern "C" {
#endif

#define VSJSON_SEPARATOR '/'

typedef struct _vsjson_t vsjson_t;

//  Callback function prototype; gets called for each parsable item.
//  You MUST return 0 if you wish to continue parsing, non-zero otherwise.
typedef int (vsjson_callback_t)(const char *locator, const char *value, void *data);

//  Main (and easiest) way to parse a json encoded input string and execute a
//  callback function for each item.
//  Returns 0 on success, non-zero otherwise
int
    vsjson_parse (const char *json, vsjson_callback_t *func, void *data);

//  Decode json string
//  Returns newly allocated decoded string or NULL on error.
//  You MUST free returned string with `free ()` or `zstr_free ()`
char *
    vsjson_decode_string (const char *string);

//  Encode json string
//  Returns newly allocated encoded string or NULL on error.
//  You MUST free returned string with `free ()` or `zstr_free ()`
char *
    vsjson_encode_string (const char *string);

//  Create a new parser object from input json string
//  Returns NULL if not enough memory.
//  You must call `vsjson_destroy ()` to destroy newly allocated parser object.
vsjson_t *
    vsjson_new (const char *json);

//  Destroy parser object
void
    vsjson_destroy (vsjson_t **self_p);

//  Get json item separator character
char
    vsjson_separator (vsjson_t *self);

//  Set json item separator character
void
    vsjson_set_separator (vsjson_t *self, char separator);

//  Return exclude character or '\0' if not set.
char
    vsjson_exclude (vsjson_t *self);

//  Set exclude character. Use '\0' to disable.
//  If set, this character is removed from each property's key.
void
    vsjson_set_exclude (vsjson_t *self, char exclude);

// get first json token, usually "[" or "{"
// tokens are [ ] { } , : string (quote included)
// number or keyword like null
const char* vsjson_first_token (vsjson_t *self);

// get next json token
// walk trough json like this:
//     vsjson *parser = vsjson_new (jsonString);
//     const char *token = vsjson_first_token (parser);
//     while (token) {
//        printf ("%s ", token);
//        token = vsjson_next_token (parser);
//     }
//     printf ("\n");
//     vsjson_destroy (&parser);
const char* vsjson_next_token (vsjson_t *self);

//  Returns 0 on success, non-zero otherwise
int vsjson_walk_through (vsjson_t *self, vsjson_callback_t *func, void *data);


#ifdef __cplusplus
}
#endif

#endif // __VSJSON_H
