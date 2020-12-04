#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

static char buffer[1024];

enum field
{
FLD_BYR = 0,
FLD_IYR = 1,
FLD_EYR = 2,
FLD_HGT = 3,
FLD_HCL = 4,
FLD_ECL = 5,
FLD_PID = 6,
FLD_CID = 7,
FLD_N
};

static char fieldnames[FLD_N][4] =
{
"byr",
"iyr",
"eyr",
"hgt",
"hcl",
"ecl",
"pid",
"cid",
};

static char *eyecolors[7] =
{
"amb",
"blu",
"brn",
"gry",
"grn",
"hzl",
"oth"
};


struct record
{
    struct record *next;

    unsigned field_map;

    char*    fields[FLD_N];
};

static struct record*
_record_new()
{
    return (struct record *) calloc(1, sizeof(struct record));
}

static void
_record_free(struct record *r)
{
    if (r)
    {
        for (size_t i=0; i<FLD_N; ++i)
        {
            free(r->fields[i]);
        }
        free(r);
    }
}

static struct record*
_reverse(struct record *t)
{
    struct record *ret = NULL;

    while (t)
    {
        struct record *i = t;
        t = t->next;

        i->next = ret;
        ret = i;
    }

    return ret;
}

#if 0
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

#endif

static struct record *table = NULL;

int
main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s datafilename\n", argv[0]);
        exit(-1);
    }

    /* Read all records into memory */
    FILE* data = fopen(argv[1], "r");
    if (! data)
    {
        perror("fopen failed: ");
        exit(-1);
    }

    struct record *cr = NULL;

    buffer[0] = ' ';
    buffer[1] = ' ';
    buffer[2] = ' ';

    char* line;
    while ((line = fgets(buffer+3, sizeof(buffer)-3, data)))
    {
        char *sep = strchr(line, ':');
        if (! sep && cr)
        {
            /* empty line, end of record separator */
            cr->next = table;
            table = cr;
            cr = NULL;
        }

        while (sep)
        {
            if (! cr)
            {
                cr = _record_new();
            }

            char *fld = sep - 3;
            size_t fld_i = 0;
            for (fld_i=0; fld_i<FLD_N; ++fld_i)
            {
               if (memcmp(fld, fieldnames[fld_i], 3) == 0)
                   break;
            }

            if (fld_i != FLD_N)
            {
                /* get field character */
                size_t len = 0;
                for (len = 0; ! isspace(sep[len+1]) && (sep[len+1] != '\0'); ++len);

                cr->fields[fld_i] = strndup(sep+1, len);
                cr->field_map |= (1<<fld_i);
            }

            sep = strchr(sep+1, ':');
        }
    }

    if (cr && cr->field_map)
    {
        cr->next = table;
        table = cr;
    }

    fclose(data);

    table = _reverse(table);
    /* NOTE table is in reverse now.  */

    size_t n_valid = 0;
    size_t n_complete = 0;
    size_t n_records = 0;

    unsigned mask = (~(1<<FLD_CID)) & ((1<<FLD_N) - 1);

// #define FAIL(msg) { printf(msg "\n"); continue; }
#define FAIL(msg) continue

    for (cr = table; cr; cr = cr->next)
    {
        n_records++;

        if ((cr->field_map & mask) != mask ) continue;

        n_complete++;

        /*  byr (Birth Year) - four digits; at least 1920 and at most 2002. */
        if (strlen(cr->fields[FLD_BYR]) != 4) FAIL("BYR length");
        long byr = strtol(cr->fields[FLD_BYR], NULL, 10);
        if ((byr < 1920) || (byr > 2002)) FAIL("BYR content");

        /* iyr (Issue Year) - four digits; at least 2010 and at most 2020. */
        if (strlen(cr->fields[FLD_IYR]) != 4) FAIL("IYR length");
        long iyr = strtol(cr->fields[FLD_IYR], NULL, 10);
        if ((iyr <2010) || (iyr > 2020)) FAIL("IYR content");

        /* eyr (Expiration Year) - four digits; at least 2020 and at most 2030. */
        if (strlen(cr->fields[FLD_EYR]) != 4) FAIL("EYR length");
        long eyr = strtol(cr->fields[FLD_EYR], NULL, 10);
        if ((eyr <2020) || (eyr > 2030)) FAIL("EYR content");


        /* hgt (Height) - a number followed by either cm or in:
         * If cm, the number must be at least 150 and at most 193.
         *  If in, the number must be at least 59 and at most 76.
         */
        char *unit;
        long hgt = strtol(cr->fields[FLD_HGT], &unit, 10);
        if (! unit) continue;
        if (strncmp(unit, "cm", 2) == 0)
        {
            if ((hgt < 150) || (hgt > 193)) FAIL("HGT content");
        }
        else if (strncmp(unit, "in", 2) == 0)
        {
            if ((hgt < 59) || (hgt > 76)) FAIL("HGT content");
        }
        else
        {
            FAIL("HGT UNIT");
        }

        /* hcl (Hair Color) - a # followed by exactly six characters 0-9 or a-f. */
        if (strlen(cr->fields[FLD_HCL]) != 7) FAIL("HCL length");
        if (cr->fields[FLD_HCL][0] != '#') FAIL("HCL prefix");
        bool hcl_ok = true;
        for (size_t i=0; (i<6) && hcl_ok; ++i)
        {
            if (! isxdigit(cr->fields[FLD_HCL][i+1]))
                hcl_ok = false;
        }
        if (! hcl_ok) FAIL("HCL content");

        /* ecl (Eye Color) - exactly one of: amb blu brn gry grn hzl oth. */
        bool ecl_ok = false;
        for (size_t i=0; i<(sizeof(eyecolors) / sizeof(eyecolors[0])); ++i)
        {
            if (strcmp(eyecolors[i], cr->fields[FLD_ECL]) == 0)
            {
                ecl_ok = true;
                break;
            }
        }
        if (! ecl_ok) FAIL("ECL content");

        /*  pid (Passport ID) - a nine-digit number, including leading zeroes. */
        if (strlen(cr->fields[FLD_PID]) != 9) continue;
        bool pid_ok = true;
        for (size_t i=0; (i<9) && pid_ok; ++i)
        {
            if ( !isdigit(cr->fields[FLD_PID][i]) )
                pid_ok = false;
        }

        if (! pid_ok) FAIL("PID content");

        /* cid (Country ID) - ignored, missing or not. */
        n_valid++;
    }

    printf("Complete passwords %zu out of %zu\n", n_complete, n_records);
    printf("Valid passwords %zu out of %zu\n", n_valid, n_records);

    while (table)
    {
        struct record *r = table;

        table = table->next;

        _record_free(r);
    }

    return 0;
}
