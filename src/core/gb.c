#include "gb.h"
#include "cartridge.h"
#include "cpu.h"
#include <string.h>
#include <stdio.h>
#define CYCLES_PER_FRAME 70224

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
    else if (adresse == 0xFF44) {
        return 0x90;   // LY stub for Gameboy Doctor (no PPU yet)
    }
    else if (adresse==0xFFFF){
        return gb->ie;
    }
    else if (adresse==0xFF0F){
        return gb->iflag | 0xE0;
    }
    else if (adresse==0xFF04){
        return gb->timer.div_counter >> 8;
    }
    else if(adresse==0xFF05){
        return gb->timer.tima;
    }
    else if(adresse==0xFF06){
        return gb->timer.tma;
    }
    else if(adresse==0xFF07){
        return gb->timer.tac | 0xF8;
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
    else if (adresse == 0xFF01) {
        gb->serial_data = value;
    }
    else if (adresse == 0xFF02 && value == 0x81) {
        fprintf(stderr, "%c", gb->serial_data);
    }
    else if (adresse==0xFFFF){
        gb->ie=value;
    }
    else if (adresse==0xFF0F){
        gb->iflag=value;
    }
    else if (adresse==0xFF04){
        gb->timer.div_counter = 0;
    }
    else if(adresse==0xFF05){
        gb->timer.tima= value;
    }
    else if(adresse==0xFF06){
        gb->timer.tma= value;
    }
    else if(adresse==0xFF07){
        gb->timer.tac = value;
    }
    else{
        //TODO
    }
}

void gb_run_frame(GameBoy *gb){
    int cycles=gb->cycles_left;
    int cycle=0;
    while (cycles<CYCLES_PER_FRAME){
        cycle=cpu_step(gb);
        timer_step(gb,cycle);
        cycles+=cycle;
    }
    gb->cycles_left=cycles-70224;
}