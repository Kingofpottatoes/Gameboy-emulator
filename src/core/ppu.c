#include "ppu.h"
#include "gb.h"

static void render_scanline(GameBoy *gb){
    int x;
    for (x=0;x<160;x++){
        gb->framebuffer[gb->ppu.ly][x]=(gb->ppu.ly/36)%4;
    }
}

void ppu_step(GameBoy *gb, int cycles){
    if (((gb->ppu.lcdc>>7) & 1)==0){
        gb->ppu.ly=0;
        gb->ppu.line_cycles=0;
        gb->ppu.stat=gb->ppu.stat & (0xFC);//set to mode 0
        return;
    }
    uint8_t old_mode = gb->ppu.stat & 0x03;
    gb->ppu.line_cycles+=cycles;
    if(gb->ppu.line_cycles >= 456){
        gb->ppu.line_cycles-=456;
        gb->ppu.ly+=1;
        if(gb->ppu.ly==144){
            gb->iflag|= 0x01;
        }
        if(gb->ppu.ly>153){
            gb->ppu.ly=0;
        }
    }
    if (gb->ppu.ly >= 144){
        gb->ppu.stat=(gb->ppu.stat & (0xFC))|1;//set to mode 1
    }
    else if (gb->ppu.line_cycles<80){
        gb->ppu.stat=(gb->ppu.stat & (0xFC))|2;//set to mode 2
    }
    else if (gb->ppu.line_cycles < 252){
        gb->ppu.stat=(gb->ppu.stat & (0xFC))|3;//set to mode 3
    }
    else {
        gb->ppu.stat=gb->ppu.stat & (0xFC);//set to mode 0
    }
    if(gb->ppu.ly==gb->ppu.lyc){
        gb->ppu.stat |= 0x04;
    }
    else{
        gb->ppu.stat &= ~0x04;
    }
    if (old_mode == 3 && (gb->ppu.stat & 0x03)==0){
        render_scanline(gb);
    }

}

void ppu_init(GameBoy *gb){
    gb->ppu.lcdc=0x91;
    gb->ppu.stat=0x85;
    gb->ppu.bgp=0xFC;
    gb->ppu.obp0=0xFF;
    gb->ppu.obp1=0xFF;
    gb->iflag=0xE1;
}