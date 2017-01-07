/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tomas@halman.net> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Tomas Halman
 * ----------------------------------------------------------------------------
 */

#include "vsjson.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *rectangle = 
"{"
"   \"name\" : \"rectangle\","
"   \"size\" : {"
"       \"height\" : 20,"
"       \"width\" : 30"
"   }"
"}";

typedef struct _shape_t {
    char *name;
    int height;
    int width;
} shape_t;

int shape_callback (const char *locator, const char *value, void *data)
{
    if (!data) return 1;
    
    shape_t *shape = (shape_t *)data;
    if (strcmp (locator,"name") == 0) {
        shape->name = vsjson_decode_string (value);
    }
    else if (strcmp (locator,"size/height") == 0) {
        shape->height = atoi (value);
    }
    else if (strcmp (locator,"size/width") == 0) {
        shape->width = atoi (value);
    }
    return 0;
}


int testing_callback (const char *locator, const char *value, void *data)
{
    if (data) {
        *(int *)data += 1;
    }
    return 0;
}



int main() {
    {
        printf (" * walking trough tokens ");
        // ---------------------------------
        vsjson_t *v = vsjson_new ("{ \"a\":\"avalue\"\n,\n\"b\": +31.4e-1, 3 : null, \"array\": [1,2,3]}");
        assert(v);
        const char *t = vsjson_first_token (v);
        while (t) {
            t = vsjson_next_token (v);
        }
        vsjson_destroy (&v);
        printf ("OK\n");
    }
    {
        printf (" * empty dict ");
        // ---------------------------------
        vsjson_t *v = vsjson_new ("{}  ");
        const char *t = vsjson_first_token (v);
        assert (!strcmp (t, "{"));
        t = vsjson_next_token (v);
        assert (!strcmp (t, "}"));
        t = vsjson_next_token (v);
        assert (t == NULL);
        vsjson_destroy (&v);
        printf ("OK\n");
    }
    {
        printf (" * empty string ");
        // ---------------------------------
        vsjson_t *v = vsjson_new ("");
        const char *t = vsjson_first_token (v);
        assert (t == NULL);
        vsjson_destroy (&v);
        printf ("OK\n");
    }
    {
        printf (" * broken json ");
        // ---------------------------------
        vsjson_t *v = vsjson_new ("[1");
        const char *t = vsjson_first_token (v);
        t = vsjson_next_token (v);
        assert (t == NULL);
        vsjson_destroy (&v);
        
        v = vsjson_new ("{ \"key:1");
        t = vsjson_first_token (v);
        t = vsjson_next_token (v);
        assert (t == NULL);
        vsjson_destroy (&v);
        printf ("OK\n");
    }
    {
        printf (" * decode ");
        // ---------------------------------
        char *s = "\"this is \\t json \\n string\"";
        char *d = vsjson_decode_string (s);
        assert (d);
        assert (strcmp (d, "this is \t json \n string" ) == 0);
        free (d);
        printf ("OK\n");        
    }
    {
        printf (" * encode ");
        // ---------------------------------
        char *s = "this\nis\t\"string\"";
        char *d = vsjson_encode_string (s);
        assert (d);
        assert (strcmp (d, "\"this\\nis\\t\\\"string\\\"\"" ) == 0);
        free (d);
        printf ("OK\n");
    }
    {
        printf (" * simple callback ");
        int called = 0;
        vsjson_t *v = vsjson_new ("{ \"key\":1, \"hey\": [\"jude\", \"you\"] }");
        vsjson_walk_trough (v, testing_callback, &called);
        assert (called == 3);
        vsjson_destroy (&v);
        printf ("OK\n");
    }
    {
        printf (" * filling callback ");
        // ---------------------------------
        shape_t shape;
        vsjson_t *v = vsjson_new (rectangle);
        vsjson_walk_trough (v, shape_callback, &shape);
        vsjson_destroy (&v);

        assert (shape.name);
        assert (strcmp (shape.name, "rectangle") == 0);
        assert (shape.height == 20);
        assert (shape.width == 30);
        free (shape.name);
        printf ("OK\n");
    }
    {
        printf (" * parse ");
        // ---------------------------------
        shape_t shape;
        int r = vsjson_parse (rectangle, shape_callback, &shape);
        assert (r == 0);
        assert (shape.name);
        assert (strcmp (shape.name, "rectangle") == 0);
        assert (shape.height == 20);
        assert (shape.width == 30);
        free (shape.name);
        printf ("OK\n");
    }
    {
        printf (" * callback func terminates early ");
        // -----------------------------------------
        int r = vsjson_parse (rectangle, shape_callback, NULL);
        assert (r == 1);
        printf ("OK\n");
    }
    return 0;
}
