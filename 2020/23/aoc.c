#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct _cup_t cup_t;
struct _cup_t
{
    struct _cup_t *next;

    int value;
};

struct _board
{
    cup_t *circle;

    cup_t *current;
    int    max_value;
    size_t n_rounds;

    cup_t* pick[3];

    cup_t  where[1000001];

    cup_t *unused;
};

cup_t * reverse(cup_t *c)
{
    cup_t *ret = NULL;

    while(c)
    {
        cup_t *i = c;
        c = c->next;
        i->next = ret;
        ret = i;
    }

    return ret;
}

struct _board*
board_setup( int* initial, size_t n, bool extended )
{
    struct _board* ret = calloc(1, sizeof(struct _board));

    cup_t *last = NULL;

    for (size_t i=0;i<(sizeof(ret->where)/sizeof(ret->where[0])); ++i)
    {
        ret->where[i].value = i;
    }

    last = ret->circle = &ret->where[initial[0]];
    for (size_t i=1; i<n; ++i)
    {
        last->next = &ret->where[initial[i]];
        last = last->next;

        if (initial[i] > ret->max_value)
        {
            ret->max_value = initial[i];
        }
    }

    if (extended)
    {
        for (; ret->max_value<1000000; ret->max_value++)
        {
            last->next = &ret->where[ret->max_value+1];
            last = last->next;
        }
    }

    ret->current = ret->circle;

    return ret;
}

cup_t *
find_dst(struct _board* brd)
{
    int current_val = brd->current->value;
    while (true)
    {
        current_val -= 1;
        if (current_val == 0)
        {
            current_val = brd->max_value;
        }

        size_t i=0;
        for (; i<3; ++i)
        {
            if (current_val == brd->pick[i]->value) break;
        }

        if (i==3)
        {
            return & brd->where[current_val];
        }
    }
}

size_t
board_round(struct _board* brd)
{
    for (size_t i=0; i<3; ++i)
    {
        if (brd->current->next)
        {
            brd->pick[i] = brd->current->next;
            brd->current->next = brd->pick[i]->next;
        }
        else
        {
            brd->pick[i] = brd->circle;
            brd->circle = brd->pick[i]->next;
        }
        brd->pick[i]->next = NULL;
    }

    cup_t *dst = find_dst(brd);
    for (size_t i=0; i<3; ++i)
    {
        brd->pick[2-i]->next = dst->next;
        dst->next = brd->pick[2-i];
    }

    brd->current = brd->current->next;
    if (! brd->current)
    {
        brd->current = brd->circle;
    }

    return ++brd->n_rounds;
}

int64_t
board_result_1(struct _board* brd)
{
    int64_t res = 0;

    cup_t *i = brd->where[1].next;
    for (int j=1; j<brd->max_value; ++j)
    {
        res = (res * 10) + i->value;
        i = i->next;
        if (! i) i = brd->circle;
    }

    return res;
}

int64_t
board_result_2(struct _board* brd)
{
    int64_t res;
    cup_t *i = brd->where[1].next;

    res = i->value;
    i = i->next;
    if (! i)
    {
        res *= brd->circle->value;
    }
    else
    {
        res *= i->value;
    }

    return res;
}

// static int assignment[] = { 3, 8, 9, 1, 2, 5, 4, 6, 7 }; <- test assignment
static int assignment[] = { 4, 8, 7, 9, 1, 2, 3, 6, 5 };

int
main(int argc, char **argv)
{
    struct _board *board = board_setup(assignment, sizeof(assignment)/sizeof(assignment[0]), false);

    while (board_round(board) < 10);

    printf("10 rounds result %ld\n", board_result_1(board));

    while (board_round(board) < 100);
    printf("100 rounds result %ld\n", board_result_1(board));

    free(board);

    board = board_setup(assignment, sizeof(assignment)/sizeof(assignment[0]), true);

    while (board_round(board) < 10000000L);
    printf("10mil rounds result %ld\n", board_result_2(board));

    return 0;
}
