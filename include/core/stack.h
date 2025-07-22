#include "types.h"

#ifndef _STACK_H
#define _STACK_H

    struct element{
        int x;
        int y;
    };

    struct stack{
        struct element *elements;
        int top;
        int size;
        int count;
    };

    int create(struct stack *s, int size);
    int push(struct stack *s, struct element e);
    int pop(struct stack *s, struct element *e);
    int is_empty(struct stack *s);
    int is_full(struct stack *s);

#endif