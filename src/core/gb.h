#ifndef GB_H
#define GB_H
#include "cpu.h"
#include "cartridge.h"
#include "timer.h"
#include "ppu.h"
#include "apu.h"
#include <stdint.h>


typedef struct GameBoy {
    CPU cpu;
    Cartridge cart;
    Timer timer;
    PPU ppu;
    APU apu;
    uint8_t wram[0x2000]; //ram currently working
    uint8_t hram[0x7F];
    uint8_t vram[0x2000];
    uint8_t oam[0xA0];
    uint8_t serial_data;
    uint8_t ie;
    uint8_t iflag;
    uint8_t framebuffer[144][160]; //DA SCREEEEN
    int cycles_left; //when we dont do exactly 70 224 cycles by frame
    uint8_t joypad_select;
    uint8_t buttons;
} GameBoy;


enum Keys{
    RIGHT,
    LEFT,
    UP,
    DOWN,
    A,
    B,
    START,
    SELECT,
};

int gb_init(GameBoy *gb,const char *rom_path);
void gb_free(GameBoy *gb);
uint8_t read8(GameBoy *gb,uint16_t adresse);
void write8(GameBoy *gb,uint16_t adresse,uint8_t value);
void gb_run_frame(GameBoy *gb);
void gb_set_button(GameBoy *gb, uint8_t button, bool pressed);
#endif