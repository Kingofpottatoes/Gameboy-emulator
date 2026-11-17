#include "gb.h"


void timer_step(GameBoy *gb,int cycles){
    static const int periods[4] = {1024, 16, 64, 256};
    int period = periods[gb->timer.tac & 0x03];   // e.g. code 1 -> periods[1] -> 16
    gb->timer.div_counter += cycles;
    if (((gb->timer.tac>>2)&1)==0){return;}
    gb->timer.tima_counter += cycles;
    while (gb->timer.tima_counter>=period){
        gb->timer.tima_counter-=period;
        if (gb->timer.tima==0xFF){
            gb->timer.tima=gb->timer.tma;
            gb->iflag |= 0x04;
        }
        else{gb->timer.tima+=1;}

    }
}

void timer_init(GameBoy *gb){
    gb->timer.div_counter=0xAB00;
    gb->timer.tac=0xF8;
}