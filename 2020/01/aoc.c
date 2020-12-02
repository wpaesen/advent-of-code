#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

static char buffer[256];

struct table
{
   size_t   fill;
   size_t   size;
   int64_t *records;
};

static void table_init(struct table* t)
{
    t->fill = 0;
    t->size = 0;
    t->records = NULL;
}

static void table_push(struct table* t, int64_t record)
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

    t->records[t->fill++] = record;
}

static void table_cleanup(struct table *t)
{
    t->fill = 0;
    t->size = 0;
    free(t->records);
    t->records = 0;
}

static void test1(int64_t a, int64_t b)
{
    if ((a + b) != 2020)
    {
        return;
    }

    printf("(%ld, %ld) S=2020 P=%ld\n", a, b, a*b);
}

static void test2(int64_t a, int64_t b, int64_t c)
{
    if ((a + b + c) != 2020)
    {
        return;
    }

    printf("(%ld, %ld, %ld) S=2020 P=%ld\n", a, b, c, a*b*c);
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
        table_push(table, strtoll(rec, NULL, 0));
    }

    fclose(data);

    printf("Got %zu records\n", table->fill);

    for (size_t i=0; i<table->fill-1; ++i)
    {
        for (size_t j=i+1; j<table->fill; ++j)
        {
            test1(table->records[i], table->records[j]);

            for (size_t k=j+1; k<table->fill; ++k)
            {
                test2(table->records[i], table->records[j], table->records[k]);
            }
        }
    }

    table_cleanup(table);

    return 0;
}
