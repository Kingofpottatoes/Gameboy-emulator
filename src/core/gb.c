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
    ppu_init(gb);
    timer_init(gb);
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
    else if (adresse>=0x8000 && adresse<=0x9FFF){
        //if its VRAM
        return gb->vram[adresse-0x8000];
    }
    else if (adresse>=0xFE00 && adresse<=0xFE9F){
        //if its OAM
        return gb->oam[adresse-0xFE00];
    }
    else if (adresse == 0xFF44) {
        return gb->ppu.ly;   //PPU implemented
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
    else if (adresse==0xFF40){
        return gb->ppu.lcdc;
    }
    else if (adresse==0xFF41){
        return gb->ppu.stat | 0x80;
    }
    else if (adresse==0xFF45){
        return gb->ppu.lyc;
    }
    else if (adresse==0xFF42){
        return gb->ppu.scy;
    }
    else if (adresse==0xFF43){
        return gb->ppu.scx;
    }
    else if (adresse==0xFF47){
        return gb->ppu.bgp;
    }
    else if (adresse==0xFF48){
        return gb->ppu.obp0;
    }
    else if (adresse==0xFF49){
        return gb->ppu.obp1;
    }
    else if (adresse==0xFF4A){
        return gb->ppu.wy;
    }
    else if (adresse==0xFF4B){
        return gb->ppu.wx;
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
    else if (adresse>=0x8000 && adresse<=0x9FFF){
        //if its VRAM
        gb->vram[adresse-0x8000]=value;
    }
    else if (adresse>=0xFE00 && adresse<=0xFE9F){
        //if its OAM
        gb->oam[adresse-0xFE00]=value;
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
    else if (adresse==0xFF40){
        gb->ppu.lcdc=value;
    }
    else if (adresse==0xFF41){
        gb->ppu.stat=(gb->ppu.stat & 0x07) | (value & 0x78);
    }
    else if (adresse==0xFF45){
        gb->ppu.lyc=value;
    }
    else if (adresse==0xFF42){
        gb->ppu.scy=value;
    }
    else if (adresse==0xFF43){
        gb->ppu.scx=value;
    }
    else if (adresse==0xFF47){
        gb->ppu.bgp=value;
    }
    else if (adresse==0xFF48){
        gb->ppu.obp0=value;
    }
    else if (adresse==0xFF49){
        gb->ppu.obp1=value;
    }
    else if (adresse==0xFF4A){
        gb->ppu.wy=value;
    }
    else if (adresse==0xFF4B){
        gb->ppu.wx=value;
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
        ppu_step(gb, cycle);
        cycles+=cycle;
    }
    gb->cycles_left=cycles-70224;
}