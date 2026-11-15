#ifndef CARTRIDGE_H
#define CARTRIDGE_H


#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


typedef struct {
    uint8_t *rom;
    size_t rom_size;
    char title[17];
    uint8_t mbc_type;
    size_t ram_size;
}Cartridge;

int cart_load(Cartridge *c,const char *rom_path);
void cart_print_header(const Cartridge *);
void cart_free(Cartridge *c);



#endif