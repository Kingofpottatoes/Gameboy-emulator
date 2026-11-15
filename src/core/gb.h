#include "cpu.h"
#include "cartridge.h"
#include <stdint.h>


typedef struct GameBoy {
    CPU cpu;
    Cartridge cart;
    uint8_t wram[0x2000]; //ram currently working
    uint8_t hram[0x7F];
} GameBoy;

int gb_init(GameBoy *gb,const char *rom_path);
void free_gb(GameBoy *gb);
uint8_t read8(GameBoy *gb,uint16_t adresse);
void write8(GameBoy *gb,uint16_t adresse,uint8_t value);