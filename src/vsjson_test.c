

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

    return 0;
}
