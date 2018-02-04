/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <tomas@halman.net> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Tomas Halman
 * ----------------------------------------------------------------------------
 * this source can be found at https://github.com/thalman/vsjson
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

static char *exclude =
"{"
"    \"client\" : \"Alfonz Tekvicka\","
"    \"mlm/server\" : {"
"        \"name\" : \"Jon/Doe\","
"        \"bi/nd\" : {"
"            \"endpoint\" : \"tcp://*:9999\""
"        },"         
"        \"security\" : {"
"            \"mechanism\" : \"plain\""
"        }"
"    },"
"    \"bin/d\" : \"no\""
"}";

typedef struct _shape_t {
    char *name;
    int height;
    int width;
} shape_t;

typedef struct _exclude_t {
    char *client;
    char *mlmserver_name;
    char *mlmserver_bind_endpoint;
    char *mlmserver_security_mechanism;
    char *bind;
} exclude_t;

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

int
exclude_callback (const char *locator, const char *value, void *data)
{
    assert (locator);
    assert (value);

    exclude_t *exclude = (exclude_t *) data;

    if (strcmp (locator, "client") == 0) {
        exclude->client = vsjson_decode_string (value);
    }
    else
    if (strcmp (locator, "mlmserver/name") == 0) {
        exclude->mlmserver_name = vsjson_decode_string (value);
    }
    else
    if (strcmp (locator, "mlmserver/bind/endpoint") == 0) {
        exclude->mlmserver_bind_endpoint = vsjson_decode_string (value);
    }
    else
    if (strcmp (locator, "mlmserver/security/mechanism") == 0) {
        exclude->mlmserver_security_mechanism = vsjson_decode_string (value);
    }
    else
    if (strcmp (locator, "bind") == 0) {
        exclude->bind = vsjson_decode_string (value);
    }
    else
        return -1;

    return 0;
}

int
separator_callback (const char *locator, const char *value, void *data)
{
    assert (locator);
    assert (value);

    exclude_t *exclude = (exclude_t *) data;

    if (strcmp (locator, "client") == 0) {
        exclude->client = vsjson_decode_string (value);
    }
    else
    if (strcmp (locator, "mlm/server#name") == 0) {
        exclude->mlmserver_name = vsjson_decode_string (value);
    }
    else
    if (strcmp (locator, "mlm/server#bi/nd#endpoint") == 0) {
        exclude->mlmserver_bind_endpoint = vsjson_decode_string (value);
    }
    else
    if (strcmp (locator, "mlm/server#security#mechanism") == 0) {
        exclude->mlmserver_security_mechanism = vsjson_decode_string (value);
    }
    else
    if (strcmp (locator, "bin/d") == 0) {
        exclude->bind = vsjson_decode_string (value);
    }
    else
        return -1;

    return 0;
}


int main() {
    {
        printf (" * walking through tokens ");
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
        vsjson_walk_through (v, testing_callback, &called);
        assert (called == 3);
        vsjson_destroy (&v);
        printf ("OK\n");
    }
    {
        printf (" * filling callback ");
        // ---------------------------------
        shape_t shape;
        vsjson_t *v = vsjson_new (rectangle);
        vsjson_walk_through (v, shape_callback, &shape);
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
    {
        printf (" * invalid json token ");
        // -----------------------------------------
        int called = 0;
        vsjson_t *v = vsjson_new ("{ \"key\" : NIL }");
        int r = vsjson_walk_through (v, testing_callback, &called);
        assert (called == 0);
        assert (r == -3);
        vsjson_destroy (&v);
        printf ("OK\n");
    }
    {
        printf (" * primitive json ");
        // -----------------------------------------
        int called = 0;
        vsjson_t *v = vsjson_new ("true");
        int r = vsjson_walk_through (v, testing_callback, &called);
        assert (called == 1);
        assert (r == 0);
        vsjson_destroy (&v);
        printf ("OK\n");
    }
    {
        printf (" * invalid list ");
        // -----------------------------------------
        int called = 0;
        vsjson_t *v = vsjson_new ("true, false");
        int r = vsjson_walk_through (v, testing_callback, &called);
        assert (r != 0);
        vsjson_destroy (&v);
        printf ("OK\n");
    }
    {
        printf (" * encode/decode ");
        // -----------------------------------------
        const char *text = "A/B\n\t\\C";
        char *encoded = vsjson_encode_string (text);
        char *decoded = vsjson_decode_string (encoded);
        assert (strcmp (text,decoded)==0);
        free (encoded);
        free (decoded);
        printf ("OK\n");
    }
    {
        printf (" * exclude ");
        // -----------------------------------------
        vsjson_t *vsjson = vsjson_new (exclude);
        assert (vsjson);

        vsjson_set_exclude (vsjson, '/');
        assert (vsjson_exclude (vsjson) == '/');

        exclude_t exclude;
        memset (&exclude, 0, sizeof (exclude_t));

        int rv = vsjson_walk_through (vsjson, exclude_callback, &exclude);
        assert (rv == 0);

        assert (strcmp (exclude.client, "Alfonz Tekvicka") == 0);
        assert (strcmp (exclude.mlmserver_name, "Jon/Doe") == 0);
        assert (strcmp (exclude.mlmserver_bind_endpoint, "tcp://*:9999") == 0);
        assert (strcmp (exclude.mlmserver_security_mechanism, "plain") == 0);
        assert (strcmp (exclude.bind, "no") == 0);

        vsjson_destroy (&vsjson);
        printf ("OK\n");
    }
    {
        printf (" * separator ");
        // -----------------------------------------
        vsjson_t *vsjson = vsjson_new (exclude);
        assert (vsjson);

        vsjson_set_separator (vsjson, '#');
        assert (vsjson_separator (vsjson) == '#');

        exclude_t exclude;
        memset (&exclude, 0, sizeof (exclude_t));

        int rv = vsjson_walk_through (vsjson, separator_callback, &exclude);
        assert (rv == 0);

        assert (strcmp (exclude.client, "Alfonz Tekvicka") == 0);
        assert (strcmp (exclude.mlmserver_name, "Jon/Doe") == 0);
        assert (strcmp (exclude.mlmserver_bind_endpoint, "tcp://*:9999") == 0);
        assert (strcmp (exclude.mlmserver_security_mechanism, "plain") == 0);
        assert (strcmp (exclude.bind, "no") == 0);

        vsjson_destroy (&vsjson);
        printf ("OK\n");
    }
    return 0;
}
