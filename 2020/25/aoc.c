#include <stdio.h>
#include <string.h>

static size_t mask = 20201227;

size_t find_loop_size(size_t subject, size_t value)
{
    size_t res = 1;
    for (size_t i=1;; ++i)
    {
        res = (res * subject)%mask;
        if (res ==  value) return i;
    }
}

size_t transform(size_t subject, size_t loopsize)
{
    size_t res = 1;
    while (loopsize--)
    {
        res = (res*subject)%mask;
    }

    return res;
}

int
main(int argc, char **argv)
{
    size_t pubkeys[2];

    if ((argc == 2) && (strcmp(argv[1], "--test") == 0))
    {
        printf("Running example testcases\n");
        pubkeys[0] = 5764801;
        pubkeys[1] = 17807724;
    }
    else
    {
        pubkeys[0] = 10943862;
        pubkeys[1] = 12721030;
    }

    size_t loopsizes[2];
    size_t encryptionkeys[2];

    for (size_t i=0; i<2; ++i)
    {
        loopsizes[i] = find_loop_size(7, pubkeys[i]);
        printf("Loop size %lu      : %lu\n", i+1, loopsizes[i] );
    }

    for (size_t i= 0; i<2; ++i)
    {
        encryptionkeys[i] = transform(pubkeys[i], loopsizes[(i+1)%2]);
        printf("Encryption key %lu : %lu\n", i+1, encryptionkeys[i]);
    }

    return 0;
}
