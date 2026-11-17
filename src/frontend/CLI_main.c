#include "../core/cpu.h"
#include "../core/gb.h"
#include <stdio.h>


static void gameboy_doctor_logs(GameBoy *gb){
    printf("A:%02X F:%02X B:%02X C:%02X D:%02X E:%02X H:%02X L:%02X SP:%04X PC:%04X PCMEM:%02X,%02X,%02X,%02X\n",
    gb->cpu.a,gb->cpu.f,gb->cpu.b,gb->cpu.c,gb->cpu.d,gb->cpu.e,gb->cpu.h,gb->cpu.l,gb->cpu.sp,gb->cpu.pc,
    read8(gb, gb->cpu.pc),read8(gb, gb->cpu.pc+1),read8(gb, gb->cpu.pc+2),read8(gb, gb->cpu.pc+3)
    );
}

int main(int argc, char *argv[]){
    if(argc<2){
        printf("usage: gb <rom> \n");
        return 1;
    }
    GameBoy gb;
    int result=gb_init(&gb,argv[1]);
    if (result!=0){return 1;}
    
    while (1){
        gameboy_doctor_logs(&gb);
        int cycles=cpu_step(&gb);
        timer_step(&gb,cycles);
    }
}