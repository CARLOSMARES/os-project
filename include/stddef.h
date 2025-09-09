#ifndef _STDDEF_H
#define _STDDEF_H

// Tipo para tamaños de objetos
typedef unsigned int size_t;

// Tipo para diferencias de punteros
typedef int ptrdiff_t;

// Valor nulo para punteros
#ifndef NULL
#define NULL ((void *)0)
#endif

// Macro offsetof: devuelve el offset de un miembro dentro de una estructura
#define offsetof(type, member) ((size_t)&(((type *)0)->member))

#endif /* _STDDEF_H */
