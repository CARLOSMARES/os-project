// include/timer.h
#ifndef TIMER_H
#define TIMER_H
#include <stdint.h>
void pit_init(uint32_t hz); // típicamente 100 o 1000 Hz
uint64_t timer_ticks(void);
#endif
