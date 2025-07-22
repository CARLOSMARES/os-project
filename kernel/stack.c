#include "core/stack.h"

int create(struct stack *p, int nro_elementos)
{

    if (nro_elementos < 0)
    {
        return 0;
    }

    struct element arreglo_elementos[nro_elementos];
    p->elements = arreglo_elementos;
    p->top = nro_elementos - 1;
    p->count = 0;
    return 1;
}

int push(struct stack *p, struct element elem)
{
    if (p == NULL)
    {
        return -1;
    }
    if (is_full(p) == 1)
    {
        return -2;
    }
    p->elements[p->count] = elem;
    p->count++;
    return 1;
}

int pop(struct stack *p, struct element *e)
{
    if (p == NULL)
    {
        return -1;
    }
    if (is_empty(p))
    {
        return -2;
    }
    struct element resp = (*p).elements[(*p).count - 1];
    p->count--;
    e->x = resp.x;
    e->y = resp.y;
    return 1;
}

int is_empty(struct stack *p)
{
    if (p->count == 0)
    {
        return 1;
    }
    return 0;
}

int is_full(struct stack *p)
{
    if (p->count > p->top)
    {
        return 1;
    }
    return 0;
}

int count(struct stack p)
{
    return p.count;
}