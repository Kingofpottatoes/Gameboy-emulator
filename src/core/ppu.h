#ifndef PPU_H
#define PPU_H
#include <stdint.h>

typedef struct {
    uint8_t lcdc;
    uint8_t stat;
    uint8_t ly;
    uint8_t lyc;
    int line_cycles;
    uint8_t scy;
    uint8_t scx;
    uint8_t bgp;
    uint8_t obp0;
    uint8_t obp1;
    uint8_t wx;
    uint8_t wy;

} PPU;

typedef struct GameBoy GameBoy;

void ppu_init(GameBoy *gb);
void ppu_step(GameBoy *gb, int cycles);

#endif