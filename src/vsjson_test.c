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
#include <string.h>

int testing_callback (const char *locator, const char *value, void *data)
{
    printf("%s -> '%s'\n", locator, value);
    return 0;
}

int main() {
    vsjson *v = vsjson_new ("{ \"a\":\"avalue\"\n,\n\"b\": +31.4e-1, 3 : null, \"array\": [1,2,3]}");
    assert(v);
    const char *t = vsjson_first_token (v);
    while (t) {
        printf("%s\n", t);
        t = vsjson_next_token (v);
    }
    vsjson_destroy (&v);

    // empty dict
    v = vsjson_new ("{}  ");
    t = vsjson_first_token (v);
    assert (!strcmp (t, "{"));
    t = vsjson_next_token (v);
    assert (!strcmp (t, "}"));
    t = vsjson_next_token (v);
    assert (t == NULL);
    vsjson_destroy (&v);

    // empty string
    v = vsjson_new ("");
    t = vsjson_first_token (v);
    assert (t == NULL);
    vsjson_destroy (&v);

    // broken json
    v = vsjson_new ("[1");
    t = vsjson_first_token (v);
    t = vsjson_next_token (v);
    assert (t == NULL);
    vsjson_destroy (&v);

    v = vsjson_new ("{ \"key:1");
    t = vsjson_first_token (v);
    t = vsjson_next_token (v);
    assert (t == NULL);
    vsjson_destroy (&v);

    //v = vsjson_new ("{ \"key\":1, \"hey\": [\"jude\", \"you\"] }");
    v = vsjson_new ("{ \"key\":1, \"hey\": [\"jude\", \"you\"] }");
    //v = vsjson_new ("{ \"key\":1, \"hey\": {\"jude\":true}");
    vsjson_walk_trough (v, testing_callback, NULL);
    vsjson_destroy (&v);

    
    return 0;
}
