#include "cpu.h"
#include "cartridge.h"
#include "timer.h"
#include <stdint.h>


typedef struct GameBoy {
    CPU cpu;
    Cartridge cart;
    Timer timer;
    uint8_t wram[0x2000]; //ram currently working
    uint8_t hram[0x7F];
    uint8_t serial_data;
    uint8_t ie;
    uint8_t iflag;
} GameBoy;

int gb_init(GameBoy *gb,const char *rom_path);
void gb_free(GameBoy *gb);
uint8_t read8(GameBoy *gb,uint16_t adresse);
void write8(GameBoy *gb,uint16_t adresse,uint8_t value);