#include "gb.h"
#include "cartridge.h"
#include <string.h>

int gb_init(GameBoy *gb,const char *rom_path){
    memset(gb,0,sizeof(*gb));
    int result=cart_load(&gb->cart,rom_path);
    if (result!=0){return 1;}
    cpu_init(&gb->cpu);
    return 0;
}

void gb_free(GameBoy *gb){
    cart_free(&gb->cart);
}
uint8_t read8(GameBoy *gb,uint16_t adresse){
    if (adresse<=0x7fff && adresse<gb->cart.rom_size){
        //if we search a rom adresse

        return gb->cart.rom[adresse];
    }
    else if (adresse>=0xC000 && adresse<=0xdfff){
        //if its wram
        return gb->wram[adresse-0xC000];
    }
    else if (adresse>=0xff80 && adresse<=0xfffe){
        //if its hram
        return gb->hram[adresse-0xff80];
    }
    else{
        //TODO
        return 0xFF;
    }
}


void write8(GameBoy *gb, uint16_t adresse, uint8_t value){
    if (adresse>=0xC000 && adresse<=0xdfff){
        //if its wram
        gb->wram[adresse-0xC000]=value;
    }
    else if (adresse>=0xff80 && adresse<=0xfffe){
        //if its hram
        gb->hram[adresse-0xff80]=value;
    }
    else{
        //TODO
    }
}