#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

static char buffer[256];

struct record
{
    unsigned min;
    unsigned max;

    char     token;

    char     password[128];
};

struct table
{
    size_t   fill;
    size_t   size;

    struct record *records;
};

static void table_init(struct table* t)
{
    t->fill = 0;
    t->size = 0;
    t->records = NULL;
}

static void table_push(struct table* t, unsigned min, unsigned max, char token, char* password)
{
    if (t->fill >= t->size)
    {
        t->size += 1024;
        t->records = realloc(t->records, t->size * sizeof(t->records[0]));
        if (! t->records)
        {
            perror("realloc failed: ");
            exit(-1);
        }
    }

    struct record* r = &t->records[t->fill++];
    r->min = min;
    r->max = max;
    r->token = token;

    int pwlen = snprintf(r->password, sizeof(r->password), "%s", password);

    if ((r->min == 0) || (r->min > pwlen))
    {
        fprintf(stderr, "Data error\n");
        exit(-1);
    }

    if ((r->max == 0) || (r->max > pwlen))
    {
        fprintf(stderr, "Data error\n");
        exit(-1);
    }
}

static void table_cleanup(struct table *t)
{
    t->fill = 0;
    t->size = 0;
    free(t->records);
    t->records = 0;
}

static size_t test1(struct record *r)
{
    size_t matched = 0;

    for (size_t i=0; (r->password[i] != '\0') && (i < sizeof(r->password)); ++i )
    {
        if (r->password[i] == r->token) matched++;
    }

    if ((r->min <= matched) && (r->max >= matched))
    {
        return 1;
    }

    return 0;
}

static size_t test2(struct record *r)
{
    bool c[2];

    c[0] = (r->password[r->min-1] == r->token);
    c[1] = (r->password[r->max-1] == r->token);

    if ((c[0] || c[1]))
    {
        if (!(c[0] && c[1])) return 1;
    }

    return 0;
}

int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s datafilename\n", argv[0]);
        exit(-1);
    }

    struct table table[1];

    table_init(table);

    /* Read all records into memory */
    FILE* data = fopen(argv[1], "r");
    if (! data)
    {
        perror("fopen failed: ");
        exit(-1);
    }

    char* rec;
    while ((rec = fgets(buffer, sizeof(buffer), data)))
    {
        int min, max;
        char token;
        char *saveptr = NULL;

        char *col = strtok_r(rec, "-", &saveptr);
        if (! col)
        {
            fprintf(stderr, "Parse error\n");
            exit(-1);
        }

        min = strtol(col, NULL, 0);

        col = strtok_r(NULL, " ", &saveptr);
        if (! col)
        {
            fprintf(stderr, "Parse error\n");
            exit(-1);
        }
        max = strtol(col, NULL, 0);

        col = strtok_r(NULL, ":", &saveptr);
        if (! col)
        {
            fprintf(stderr, "Parse error\n");
            exit(-1);
        }
        token = col[0];

        col = strtok_r(NULL, "\n", &saveptr);
        if (! col)
        {
            fprintf(stderr, "Parse error\n");
            exit(-1);
        }

        if (isspace(col[0]))
        {
            col++;
        }

        table_push(table, min, max, token, col);
    }

    fclose(data);

    printf("Got %zu records\n", table->fill);

    size_t valid_a = 0;
    size_t valid_b = 0;
    for (size_t i=0; i<table->fill; ++i)
    {
        struct record *r = table->records + i;

        valid_a += test1(r);
        valid_b += test2(r);
    }

    printf("Case a : %zu valid passwords\n", valid_a);
    printf("Case b : %zu valid passwords\n", valid_b);

    table_cleanup(table);

    return 0;
}
