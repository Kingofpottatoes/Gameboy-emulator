#include "cpu.h"
#include <stdint.h>

typedef struct {
    uint16_t div_counter;
    uint8_t tima;
    uint8_t tma;
    uint8_t tac;
    int tima_counter;
} Timer;

typedef struct GameBoy GameBoy ;

void timer_init(GameBoy *gb);
void timer_step(GameBoy *gb,int cycles);