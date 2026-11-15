#include "cpu.h"
#include "gb.h"

void cpu_init(CPU *cpu){
    cpu->af=0x01B0;
    cpu->bc=0x0013;
    cpu->de=0x00d8;
    cpu->hl=0x014d;
    cpu->sp=0xfffe;
    cpu->pc=0x0100;
}

int cpu_step(GameBoy *gb){


    return 0;
}