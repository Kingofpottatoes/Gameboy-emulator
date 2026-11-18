#ifndef CARTRIDGE_H
#define CARTRIDGE_H


#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


typedef struct {
    uint8_t *rom;
    uint8_t *ram;
    size_t rom_size;
    char title[17];
    char save_path[512];
    uint8_t mbc_type;
    size_t ram_size;
    uint8_t rom_bank;
    uint8_t ram_bank;
    bool is_ram_activated;
}Cartridge;

int cart_load(Cartridge *c,const char *rom_path);

uint8_t cart_read(Cartridge *c,uint16_t adresse);
void cart_write(Cartridge *c,uint16_t adresse,uint8_t value);

void cart_print_header(const Cartridge *);
void cart_free(Cartridge *c);



#endif