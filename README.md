# vsjson
Very simple json parser

## Purpose of this project
Goals of this project are:
* no dependencies
* pure c
* one header and one c file, include them in your project and compile together
* easy to use
* focussed on simple cases, when big libraries are too much

## Simle callback API
Define you data structure, write callback function and use it.
Example:

```c

#include "vsjson.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// this is json we are going to parse
static char *rectangle = 
"{"
"   \"name\" : \"rectangle\","
"   \"size\" : {"
"       \"height\" : 20,"
"       \"width\" : 30"
"   }"
"}";

// our structure for data
typedef struct _shape_t {
    char *name;
    int height;
    int width;
} shape_t;

// callback function
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

// use it together
int main ()
{
    shape_t shape;
    int r = vsjson_parse (rectangle, shape_callback, &shape);
    assert (r == 0);
    assert (shape.name);
    assert (strcmp (shape.name, "rectangle") == 0);
    assert (shape.height == 20);
    assert (shape.width == 30);
    free (shape.name);
    printf ("OK\n");
    return 0;
}
```

## Known limitation

* String escape/unescape \uXXXX is not implemented
* Works fine with ASCII/UTF-8 json, UTF-16 is not supported
