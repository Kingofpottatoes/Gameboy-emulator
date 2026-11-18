#ifndef CARTRIDGE_H
#define CARTRIDGE_H


#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>


typedef enum{
    MBC_NONE,
    MBC1,
    MBC2,
    MBC3,
} MBC;

typedef struct {
    uint8_t *rom;
    uint8_t *ram;
    size_t rom_size;
    char title[17];
    char save_path[512];
    MBC mbc;
    uint8_t mbc_type;
    size_t ram_size;
    //for MBC3
    uint8_t rom_bank;
    uint8_t ram_bank;
    bool is_ram_activated;
    //for MBC1
    uint8_t bank_low;
    uint8_t bank_high;
    bool mode;
}Cartridge;



int cart_load(Cartridge *c,const char *rom_path);

uint8_t cart_read(Cartridge *c,uint16_t adresse);
void cart_write(Cartridge *c,uint16_t adresse,uint8_t value);

void cart_print_header(const Cartridge *);
void cart_free(Cartridge *c);



#endif