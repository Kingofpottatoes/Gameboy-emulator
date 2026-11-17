#include "ppu.h"
#include "gb.h"
#include <stdint.h>

static void render_scanline(GameBoy *gb){

    uint16_t map_base;
    if(((gb->ppu.lcdc>>3)& 1)!=0){
        map_base=0x9C00;
    }
    else{
        map_base=0x9800;
    }
    uint16_t py = (gb->ppu.ly + gb->ppu.scy) & 0xFF;
    uint16_t tile_row = py / 8;
    uint8_t line_in_tile = py % 8;
    int x;
    uint8_t px,tile_col,high_color,low_color,color;
    uint16_t tile_addr;
    for (x=0;x<160;x++){
        px = (x + gb->ppu.scx) & 0xFF;
        tile_col = px / 8;
        uint8_t coord_in_line=px%8;
        uint8_t i_tile=read8(gb,map_base+tile_row*32 +tile_col);
        if (((gb->ppu.lcdc>>4)&1)==1){
            tile_addr=0x8000 + i_tile * 16;
        }
        else{
            tile_addr=0x9000 + (int8_t)i_tile * 16;
        }
        low_color=read8(gb,tile_addr +line_in_tile*2);
        high_color=read8(gb,1+tile_addr +line_in_tile*2);
        color=((high_color>>(7-coord_in_line))&1)*2+((low_color>>(7-coord_in_line))&1);
        uint8_t nuance=((gb->ppu.bgp>>color*2)&0b00000011);
        gb->framebuffer[gb->ppu.ly][x]=nuance;
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