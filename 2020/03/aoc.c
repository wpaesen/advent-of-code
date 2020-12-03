#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

static char buffer[256];

struct record
{
    uint64_t pattern;
    size_t   len;
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

static void table_push(struct table* t, size_t len, uint64_t pattern)
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
    r->len = len;
    r->pattern = pattern;

    if ((r->len == 0) || (r->len > 64))
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

static size_t test1(struct record *r, size_t pos)
{
    size_t ofs = pos % r->len;
    uint64_t mask = 1 << ofs;
    if (r->pattern & mask)
    {
        return 1;
    }

    return 0;
}

#if 0
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
#endif

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
        int64_t pattern = 0, mask = 1;
        size_t i = 0;

        for (i=0; (rec[i] != '\0') && (rec[i] != '\n'); ++i)
        {
            if (rec[i] == '#')
            {
                pattern |= mask;
            }
            mask = mask << 1;
        }

        table_push(table, i, pattern);
    }

    fclose(data);

    printf("Got %zu records\n", table->fill);

    struct slope
    {
        size_t steps_r;
        size_t steps_d;

        size_t pos_r;
        size_t pos_d;

        size_t n_trees;
        size_t n_checks;
    };

    struct slope slopes[5] = {
        { 1, 1, },
        { 3, 1, },
        { 5, 1, },
        { 7, 1, },
        { 1, 2, },
    };

    for (size_t i=0; i<table->fill; ++i)
    {
        struct record *r = table->records + i;

        for (size_t j=0; j<(sizeof(slopes)/sizeof(slopes[0])); ++j)
        {
            struct slope *s = slopes + j;
            if (i == s->pos_d)
            {
                s->n_trees += test1(r, s->pos_r);
                s->pos_r += s->steps_r;
                s->pos_d += s->steps_d;
                s->n_checks++;
            }
        }
    }

    size_t answer = 1;
    for (size_t j=0; j<(sizeof(slopes)/sizeof(slopes[0])); ++j)
    {
        struct slope *s = slopes + j;
        printf("slope r=%zu,d=%zu : n_trees=%zu, n_checks=%zu\n", s->steps_r, s->steps_d, s->n_trees, s->n_checks);
        answer *= s->n_trees;
    }

    printf("answer b : %zu\n", answer);

    table_cleanup(table);

    return 0;
}
