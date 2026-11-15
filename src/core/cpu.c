#include "cpu.h"
#include "gb.h"
#include <stdio.h>
#include <stdlib.h>

static uint8_t fetch8(GameBoy *gb){
    uint8_t instruction=read8(gb,gb->cpu.pc);
    gb->cpu.pc+=1;
    return instruction;
}

static uint16_t fetch16(GameBoy *gb){
    uint8_t low=fetch8(gb);
    uint8_t high=fetch8(gb);
    //get a 16 bit instruction
    uint16_t instruction=(high << 8) | low;
    return instruction;
}



void cpu_init(CPU *cpu){
    cpu->af=0x01B0;
    cpu->bc=0x0013;
    cpu->de=0x00d8;
    cpu->hl=0x014d;
    cpu->sp=0xfffe;
    cpu->pc=0x0100;
}

int cpu_step(GameBoy *gb){
    uint8_t opcode=fetch8(gb);
    switch (opcode) {
        case 0x00:{return 4;}
        case 0xC3 :{gb->cpu.pc=fetch16(gb);return 16;}
        default:{
            printf("unknown opcode %02X at pc-1 : %04X \n",opcode,gb->cpu.pc-1);
            exit(1);
        
        }

    
    }
    return 0;
}