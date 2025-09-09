// include/keyboard.h
#ifndef KEYBOARD_H
#define KEYBOARD_H
void keyboard_init(void);
int keyboard_read_scancode(void); // no bloqueante; -1 si vacío
#endif
