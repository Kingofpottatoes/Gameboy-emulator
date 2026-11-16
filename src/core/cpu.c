#include "cpu.h"
#include "gb.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>


static void unknow_opcode(GameBoy *gb,uint8_t opcode){
    printf("unknown opcode %02X at pc-1 : %04X \n",opcode,gb->cpu.pc-1);
    exit(1);
}
static void unknow_op(GameBoy *gb,uint8_t op){
    printf("unknown operand %02X at pc-1 : %04X \n",op,gb->cpu.pc-1);
    exit(1);
}


static uint8_t fetch8(GameBoy *gb){
    uint8_t operand=read8(gb,gb->cpu.pc);
    gb->cpu.pc+=1;
    return operand;
}

static uint16_t fetch16(GameBoy *gb){
    uint8_t low=fetch8(gb);
    uint8_t high=fetch8(gb);
    //get a 16 bit operand
    uint16_t operand=(high << 8) | low;
    return operand;
}

static void set_flag(CPU*cpu,uint8_t mask,bool value){
    if (value){
        cpu->f|=mask;   
    }
    else {
        cpu->f&=~mask;
    }
}

static bool get_flag(const CPU *cpu,uint8_t mask){
    return (cpu->f & mask)!=0;
}

static uint8_t read_r8(GameBoy *gb,uint8_t index){
    switch (index){
        case 0:{return gb->cpu.b;}
        case 1:{return gb->cpu.c;}
        case 2:{return gb->cpu.d;}
        case 3:{return gb->cpu.e;}
        case 4:{return gb->cpu.h;}
        case 5:{return gb->cpu.l;}
        case 6:{return read8(gb,gb->cpu.hl);}
        case 7:{return gb->cpu.a;}
        default:{
            printf("unknown index %02X",index);
            return 0xFF;
        }
    }
}

static void write_r8(GameBoy *gb,uint8_t index,uint8_t value){
    switch (index){
        case 0:{gb->cpu.b=value;break;}
        case 1:{gb->cpu.c=value;break;}
        case 2:{gb->cpu.d=value;break;}
        case 3:{gb->cpu.e=value;break;}
        case 4:{gb->cpu.h=value;break;}
        case 5:{gb->cpu.l=value;break;}
        case 6:{write8(gb, gb->cpu.hl,value);break;}
        case 7:{gb->cpu.a=value;break;}
        default:{
            printf("unknown index %02X \n",index);
        }
    }
}

static void alu(GameBoy *gb,uint8_t op,uint8_t value){
    uint8_t a=gb->cpu.a;
    uint8_t c=get_flag(&gb->cpu, FLAG_C);
    bool Z,N,H,C;
    int r;
    switch(op){
        case 0:{
            //ADD
            r=a+value;
            Z=(r &0xFF)==0;
            N=false;
            H=(a & 0xF) + (value & 0xF) > 0xF;
            C=r>0xFF;
            break;
        }
        case 1:{
            //ADC
            r=a+value + c;
            Z=(r &0xFF)==0;
            N=false;
            H=(a & 0xF) + (value & 0xF) + c> 0xF;
            C=r>0xFF;
            break;
        }
        case 2:{
            //SUB
            r=a-value;
            Z=(r &0xFF)==0;
            N=true;
            H=(a & 0xF) < (value & 0xF);
            C=r<0;
            break;
        }
        case 3:{
            //SBC
            r=a-value-c;
            Z=(r & 0xFF) == 0;
            N=true;
            H=(a & 0xF) < (value & 0xF) + c;
            C=r<0;
            break;
        }
        case 4:{
            //AND
            r=a & value;
            Z=r==0;
            N=false;
            H=true;//strangeness of the GameBoy \(._.)/
            C=false;
            break;
        }
        case 5:{
            //XOR
            r=a ^ value;
            Z=r==0;
            N=false;
            H=false;
            C=false;
            break;
        }
        case 6:{
            //OR
            r=a | value;
            Z=r==0;
            N=false;
            H=false;
            C=false;
            break;
        }
        case 7:{
            //CP
            r =a - value;
            Z=(r &0xFF)==0;
            N=true;
            H=(a & 0xF) < (value & 0xF);
            C=r<0;
            break;

        }
        default:{unknow_op(gb,op);}
    }
    if (op!=7){gb->cpu.a=(uint8_t) r;}

    set_flag(&gb->cpu,FLAG_Z,Z);
    set_flag(&gb->cpu,FLAG_N,N);
    set_flag(&gb->cpu,FLAG_H,H);
    set_flag(&gb->cpu,FLAG_C,C);
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
    uint8_t xx=opcode >> 6;
    uint8_t yyy=(opcode>>3) & 0b00000111;
    uint8_t zzz=opcode & 0b00000111;
    switch (xx) {
        case 0:{
            switch(opcode){
                case 0 :{return 4;}
                default:{unknow_opcode(gb,opcode);}
            }
            break;
        }
        case 1:{
            if (opcode==0x76){
                //HALT
                unknow_opcode(gb,opcode);
                //todo
                break;
            }
            uint8_t value=read_r8(gb, zzz);
            write_r8(gb, yyy, value);
            if (yyy==6 || zzz==6){return 8;}
            else{return 4;}
            break;
        }
        case 2:{
            //ALU
            uint8_t value=read_r8(gb, zzz);
            alu(gb, yyy, value);
            if(zzz==6){return 8;}
            else{return 4;}
            break;
        }
        case 3:{
            switch(opcode){
                case 0xC3:{
                    uint16_t dest=fetch16(gb);
                    gb->cpu.pc=dest;
                    return 16;
                }
                default:{unknow_opcode(gb,opcode);}
            }
            
            break;
        }
        default:{
            printf("unknown opcode %02X at pc-1 : %04X \n",opcode,gb->cpu.pc-1);
            exit(1);
        
        }
    
    }
    return 0;
}
