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
            printf("unknown index %02X\n",index);
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
static uint16_t read_r16(GameBoy *gb,uint8_t index){
    switch (index){
        case 0:{return gb->cpu.bc;}
        case 1:{return gb->cpu.de;}
        case 2:{return gb->cpu.hl;}
        case 3:{return gb->cpu.sp;}
        default:{
            printf("unknown index %02X\n",index);
            return 0xFFFF;
        }
    }
}

static void write_r16(GameBoy *gb,uint8_t index,uint16_t value){
    switch (index){
        case 0:{gb->cpu.bc=value;break;}
        case 1:{gb->cpu.de=value;break;}
        case 2:{gb->cpu.hl=value;break;}
        case 3:{gb->cpu.sp=value;break;}
        default:{
            printf("unknown index %02X \n",index);
            break;
        }
    }
}

static void push16(GameBoy *gb, uint16_t value){
    gb->cpu.sp-=1;
    write8(gb, gb->cpu.sp, value >> 8);
    gb->cpu.sp-=1;
    write8(gb, gb->cpu.sp, value & 0xFF);
}
static uint16_t pop16(GameBoy *gb){
    uint8_t low=read8(gb,gb->cpu.sp);
    gb->cpu.sp+=1;
    uint8_t high=read8(gb,gb->cpu.sp);
    gb->cpu.sp+=1;
    return (high<<8)+low;
}
static uint16_t read_r16stk(GameBoy *gb,uint8_t index){
    switch (index){
        case 0:{return gb->cpu.bc;}
        case 1:{return gb->cpu.de;}
        case 2:{return gb->cpu.hl;}
        case 3:{return gb->cpu.af;}
        default:{
            printf("unknown index %02X\n",index);
            return 0xFFFF;
        }
    }
}
static void write_r16stk(GameBoy *gb,uint8_t index,uint16_t value){
    switch (index){
        case 0:{gb->cpu.bc=value;break;}
        case 1:{gb->cpu.de=value;break;}
        case 2:{gb->cpu.hl=value;break;}
        case 3:{gb->cpu.af = value & 0xFFF0;;break;}
        default:{
            printf("unknown index %02X \n",index);
            break;
        }
    }
}

static uint16_t r16mem_addr(GameBoy *gb,uint8_t index){
    switch (index){
        case 0:{
            //BC
            return gb->cpu.bc;
        }
        case 1:{
            //DE
            return gb->cpu.de;
        }
        case 2:{
            //HL+
            uint16_t hl=gb->cpu.hl;
            gb->cpu.hl+=1;
            return hl;
        }
        case 3:{
            //HL-
            uint16_t hl=gb->cpu.hl;
            gb->cpu.hl-=1;
            return hl;
        }
        default:{
            printf("unknown index %02X \n",index);
            exit(1);
        }
    }
}
static bool check_cond(GameBoy *gb, uint8_t cc){
        switch (cc){
            case 0:{
                if(get_flag(&gb->cpu,FLAG_Z)==0){return true;}
                else{return false;}
            }
            case 1:{
                if(get_flag(&gb->cpu,FLAG_Z)==1){return true;}
                else{return false;}
            }
            case 2:{
                if(get_flag(&gb->cpu,FLAG_C)==0){return true;}
                else{return false;}
            }
            case 3:{
                if(get_flag(&gb->cpu,FLAG_C)==1){return true;}
                else{return false;}
            }
            default:{
                printf("unknown condition %02X \n",cc);
                exit(1);
            }
        }   

}
static int cb_step(GameBoy *gb){
    uint8_t opcode=fetch8(gb);
    uint8_t xx=opcode >> 6;
    uint8_t yyy=(opcode>>3) & 0b00000111;
    uint8_t zzz=opcode & 0b00000111;
    uint8_t value=read_r8(gb, zzz);
    bool Z,N,H,C;
    uint8_t r;
    switch (xx){
        case 0:{
            switch (yyy){
                case 0:{
                    //RLC
                    r=(value << 1)|(value>>7);
                    C=value & 0b10000000;
                    break;
                }
                case 1:{
                    //RRC
                    r=(value >> 1)|(value<<7);
                    C=value & 0x01;
                    break;
                }
                case 2:{
                    //RL
                    uint8_t c=get_flag(&gb->cpu,FLAG_C);
                    r=(value<<1)|c;
                    C=value & 0b10000000;
                    break;
                }
                case 3:{
                    //RR
                    uint8_t c=get_flag(&gb->cpu,FLAG_C);
                    r=(value>>1)|(c<<7);
                    C=value & 0x01;
                    break;
                }
                case 4:{
                    //SLA
                    r=value<<1;
                    C=value & 0b10000000;
                    break;
                }
                case 5:{
                    //SRA
                    r=(value >> 1) | (value & 0x80);
                    C=value & 0x01;
                    break;
                }
                case 6:{
                    //SWAP
                    r=(value << 4) | (value >> 4);
                    C=false;
                    break;
                }
                case 7:{
                    //SRL
                    r=value >> 1;
                    C=value & 0x01;
                    break;
                }    
            }
            Z=r==0;
            N=false;
            H=false;
            write_r8(gb, zzz, r);
            set_flag(&gb->cpu,FLAG_Z,Z);
            set_flag(&gb->cpu,FLAG_H,H);
            set_flag(&gb->cpu,FLAG_N,N);
            set_flag(&gb->cpu,FLAG_C,C);
            if (zzz==6){return 16;}
            else{return 8;}            
        }
        case 1:{
            //BIT
            bool bit = ((value >> yyy) & 1 )==0;
            if (bit){
                Z=true;
                N=false;
                H=true;
            }
            else {
                Z=false;
                N=false;
                H=true;
            }
            set_flag(&gb->cpu,FLAG_Z,Z);
            set_flag(&gb->cpu,FLAG_N,N);
            set_flag(&gb->cpu,FLAG_H,H);
            if (zzz==6){return 12;}
            else{return 8;}
        }
        case 2:{
            //RES
            uint8_t bit = value & ~(1 << yyy);
            write_r8(gb, zzz, bit);
            if (zzz==6){return 16;}
            else{return 8;}
        }
        case 3:{
            //SET
            uint8_t bit = value | (1 << yyy);
            write_r8(gb, zzz, bit);
            if (zzz==6){return 16;}
            else{return 8;}
        }



    }
    return 0;
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
    uint8_t target = (opcode >> 4) & 0x03;
    bool Z,N,H,C;
    switch (xx) {
        case 0:{
            switch (opcode & 0xCF){
                case 0x01:{
                    //LD
                    write_r16(gb,target,fetch16(gb));
                    return 12;
                }
                case 0x03:{
                    // INC r16
                    write_r16(gb,target,read_r16(gb, target)+1);
                    return 8;
                }
                case 0x0B:{
                    // DEC r16
                    write_r16(gb,target,read_r16(gb, target)-1);
                    return 8;
                }
                case 0x09:{
                    //ADD HL
                    int HL=gb->cpu.hl;
                    int value=read_r16(gb, target);
                    int r=HL+value;
                    N=false;
                    H=(HL & 0xFFF) + (value & 0xFFF) > 0xFFF;
                    C=r>0xFFFF;
                    gb->cpu.hl=r;
                    set_flag(&gb->cpu,FLAG_N,N);
                    set_flag(&gb->cpu,FLAG_H,H);
                    set_flag(&gb->cpu,FLAG_C,C);
                    return 8;
                }
                case 0x2:{
                    uint16_t addr=r16mem_addr(gb, target);
                    write8(gb,addr,gb->cpu.a);
                    return 8;
                }
                case 0x0A:{
                    uint16_t addr=r16mem_addr(gb, target);
                    gb->cpu.a=read8(gb, addr);
                    return 8;
                }
            }
            switch (opcode & 0xC7){
                case 0x06:{
                    //LD r8,n8
                    write_r8(gb,yyy,fetch8(gb));
                    if(yyy==6){return 12;}
                    else{return 8;}
                }
                case 0x07:{
                    switch (yyy){
                        case 0:{
                            //RLCA
                            uint8_t pushed_bit=(gb->cpu.a>>7);
                            gb->cpu.a=(gb->cpu.a <<1)+pushed_bit;
                            Z=false;
                            N=false;
                            H=false;
                            C=pushed_bit;
                            set_flag(&gb->cpu,FLAG_Z,Z);
                            set_flag(&gb->cpu,FLAG_N,N);
                            set_flag(&gb->cpu,FLAG_H,H);
                            set_flag(&gb->cpu,FLAG_C,C);
                            break;
                        }
                        case 1:{
                            //RRCA
                            uint8_t pushed_bit=gb->cpu.a & 0x01;
                            gb->cpu.a=(gb->cpu.a >>1)+ (pushed_bit<<7);
                            Z=false;
                            N=false;
                            H=false;
                            C=pushed_bit;
                            set_flag(&gb->cpu,FLAG_Z,Z);
                            set_flag(&gb->cpu,FLAG_N,N);
                            set_flag(&gb->cpu,FLAG_H,H);
                            set_flag(&gb->cpu,FLAG_C,C);
                            break;
                        }
                        case 2:{
                            //RLA
                            C=get_flag(&gb->cpu,FLAG_C);
                            uint8_t pushed_bit=(gb->cpu.a>>7);
                            gb->cpu.a=(gb->cpu.a <<1)+C;
                            Z=false;
                            N=false;
                            H=false;
                            C=pushed_bit;
                            set_flag(&gb->cpu,FLAG_Z,Z);
                            set_flag(&gb->cpu,FLAG_N,N);
                            set_flag(&gb->cpu,FLAG_H,H);
                            set_flag(&gb->cpu,FLAG_C,C);
                            break;
                        }
                        case 3:{
                            //RRA
                            C=get_flag(&gb->cpu,FLAG_C);
                            uint8_t pushed_bit=gb->cpu.a & 0x01;
                            gb->cpu.a=(gb->cpu.a >>1)+ (C<<7);
                            Z=false;
                            N=false;
                            H=false;
                            C=pushed_bit;
                            set_flag(&gb->cpu,FLAG_Z,Z);
                            set_flag(&gb->cpu,FLAG_N,N);
                            set_flag(&gb->cpu,FLAG_H,H);
                            set_flag(&gb->cpu,FLAG_C,C);
                            break;
                        }
                        case 4:{
                            //DAA
                            uint8_t A=gb->cpu.a;
                            Z=get_flag(&gb->cpu, FLAG_Z);
                            N=get_flag(&gb->cpu, FLAG_N);
                            H=get_flag(&gb->cpu, FLAG_H);
                            C=get_flag(&gb->cpu, FLAG_C);
                            if (!N){
                                if(C || A>0x99){
                                    A+=0x60;
                                    C=true;
                                }
                                if (H || (A & 0x0F) >0x9){
                                    A+=0x06;
                                }
                            }
                            else {
                                if (C){
                                    A-=0x60;
                                }
                                if(H){
                                    A-=0x06;
                                }
                            }
                            Z=(A==0);
                            H=false;
                            gb->cpu.a=A;
                            set_flag(&gb->cpu,FLAG_Z,Z);
                            set_flag(&gb->cpu,FLAG_H,H);
                            set_flag(&gb->cpu,FLAG_C,C);
                            break;
                        }
                        case 5:{
                            //CPL
                            gb->cpu.a=~gb->cpu.a;
                            N=true;
                            H=true;
                            set_flag(&gb->cpu,FLAG_N,N);
                            set_flag(&gb->cpu,FLAG_H,H);
                            break;
                        }
                        case 6:{
                            //SCF
                            N=false;
                            H=false;
                            C=true;
                            set_flag(&gb->cpu,FLAG_N,N);
                            set_flag(&gb->cpu,FLAG_H,H);
                            set_flag(&gb->cpu,FLAG_C,C);
                            break;
                        }
                        case 7:{
                            //CCF
                            N=false;
                            H=false;
                            C=!get_flag(&gb->cpu,FLAG_C);
                            set_flag(&gb->cpu,FLAG_N,N);
                            set_flag(&gb->cpu,FLAG_H,H);
                            set_flag(&gb->cpu,FLAG_C,C);
                            break;
                        }
                    }
                    return 4;
                }
                case 0x04:{
                    //INC r8
                    uint8_t value=read_r8(gb, yyy);
                    uint8_t r=value+1;
                    Z=r==0;
                    N=false;
                    H=(value & 0xF) == 0xF;
                    write_r8(gb, yyy, r);
                    set_flag(&gb->cpu,FLAG_Z,Z);
                    set_flag(&gb->cpu,FLAG_N,N);
                    set_flag(&gb->cpu,FLAG_H,H);
                    if(yyy==6){return 12;}
                    else{return 4;}  
                }
                case 0x05:{
                    //DEC r8
                    uint8_t value=read_r8(gb, yyy);
                    uint8_t r=value-1;
                    Z=r==0;
                    N=true;
                    H=(value & 0xF) == 0;
                    write_r8(gb, yyy, r);
                    set_flag(&gb->cpu,FLAG_Z,Z);
                    set_flag(&gb->cpu,FLAG_N,N);
                    set_flag(&gb->cpu,FLAG_H,H);
                    if(yyy==6){return 12;}
                    else{return 4;}  
                }
            }
            switch (opcode & 0xE7){
                case 0x20:{
                    //JR cc e8
                    int8_t e8=(int8_t)fetch8(gb);
                    bool cc=check_cond(gb, (opcode >> 3) & 0x03);
                    if(cc){
                        gb->cpu.pc+=e8;
                        return 12;
                    }
                    else{
                        return 8;
                    }
                }
            }
            switch(opcode){
                case 0x18:{
                    //JR e8
                    int8_t e8=fetch8(gb);
                    gb->cpu.pc+=e8;
                    return 12;
                }
                case 0x08:{
                    //LD (n16), SP
                    uint16_t addr=fetch16(gb);
                    write8(gb, addr,(gb->cpu.sp & 0xFF));
                    write8(gb, addr+1,(gb->cpu.sp >>8));
                    return 20;
                }
                case 0x10:{
                    //STOP TODO but not for now
                    fetch8(gb);
                    return 4;
                }
                case 0 :{
                    //NOP
                    return 4;
                }
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
            switch (opcode & 0xCF){
                case 0xC5:{
                    //PUSH
                    uint16_t value=read_r16stk(gb,target);
                    push16(gb,value);
                    return 16;
                }
                case 0xC1:{
                    //POP
                    uint16_t value =pop16(gb);
                    write_r16stk(gb, target, value);
                    return 12;
                }
            }
            switch (opcode & 0xC7){
                case 0xC6:{
                    alu(gb, yyy, fetch8(gb));
                    return 8;
                }
                case 0xC7:{
                    //RST n
                    push16(gb, gb->cpu.pc);
                    gb->cpu.pc=yyy*8;
                    return 16;
                }
            }
            switch (opcode & 0xE7){
                case 0xC2:{
                    //JP cc, n16
                    uint16_t addr=fetch16(gb);
                    bool cc=check_cond(gb, (opcode >> 3) & 0x03);
                    if(cc){
                        gb->cpu.pc=addr;
                        return 16;
                    }
                    return 12;

                }
                case 0xC4:{
                    //CALL cc,n16
                    uint16_t addr=fetch16(gb);
                    bool cc=check_cond(gb, (opcode >> 3) & 0x03);
                    if (cc){
                        push16(gb, gb->cpu.pc);
                        gb->cpu.pc=addr;
                        return 24;
                    }
                    return 12;
                }
                case 0xC0:{
                    //RET cc
                    bool cc=check_cond(gb, (opcode >> 3) & 0x03);
                    if (cc){
                        gb->cpu.pc=pop16(gb);
                        return 20;
                    }
                    return 8;

                }
            }
            switch(opcode){
                case 0xCB:{
                    return cb_step(gb);
                }
                case 0xF3:{
                    //DI
                    gb->cpu.ime = false;
                    return 4;
                }
                case 0xFB:{
                    gb->cpu.ime = true;
                    // TODO: delayed by one instruction
                    return 4;
                }
                case 0xD9:{
                    //RETI
                    gb->cpu.pc=pop16(gb);
                    gb->cpu.ime=true;
                    return 16;
                }
                case 0xF9:{
                    gb->cpu.sp=gb->cpu.hl;
                    return 8;
                }
                case 0xE8:{
                    uint8_t sp=gb->cpu.sp;
                    int8_t e8=(int8_t)fetch8(gb);
                    gb->cpu.sp+=e8;
                    Z=false;
                    N=false;
                    H=(sp & 0xF) + (e8 & 0xF) > 0xF;
                    C=(sp & 0xFF) + (e8 & 0xFF) > 0xFF;
                    set_flag(&gb->cpu, FLAG_Z,Z);
                    set_flag(&gb->cpu, FLAG_N,N);
                    set_flag(&gb->cpu, FLAG_H,H);
                    set_flag(&gb->cpu, FLAG_C,C);
                    return 16;
                }
                case 0xF8:{
                    int8_t e8=(int8_t)fetch8(gb);
                    gb->cpu.hl=gb->cpu.sp+e8;
                    Z=false;
                    N=false;
                    H=(gb->cpu.sp & 0xF) + (e8 & 0xF) > 0xF;
                    C=(gb->cpu.sp & 0xFF) + (e8 & 0xFF) > 0xFF;
                    set_flag(&gb->cpu, FLAG_Z,Z);
                    set_flag(&gb->cpu, FLAG_N,N);
                    set_flag(&gb->cpu, FLAG_H,H);
                    set_flag(&gb->cpu, FLAG_C,C);
                    return 12;
                }
                case 0xE0:{
                    //LDH n8,A
                    write8(gb, 0xFF00 + fetch8(gb), gb->cpu.a);
                    return 12;
                }
                case 0xF0:{
                    //LDH A, n8
                    gb->cpu.a=read8(gb,0xFF00 + fetch8(gb));
                    return 12;
                }
                case 0xE2:{
                    //LD C,A
                    write8(gb, 0xFF00 + gb->cpu.c, gb->cpu.a);
                    return 8;
                }
                case 0xF2:{
                    gb->cpu.a=read8(gb,0xFF00 + gb->cpu.c);
                    return 8;
                }
                case 0xEA:{
                    //LD n16, A
                    uint16_t addr=fetch16(gb);
                    write8(gb, addr, gb->cpu.a);
                    return 16;
                }
                case 0xFA:{
                    //LD A,n16
                    gb->cpu.a=read8(gb, fetch16(gb));
                    return 16;
                }
                case 0xE9:{
                    //JP HL
                    gb->cpu.pc=gb->cpu.hl;
                    return 4;
                }
                case 0xC3:{
                    uint16_t dest=fetch16(gb);
                    gb->cpu.pc=dest;
                    return 16;
                }
                case 0xCD:{
                    //CALL n16
                    uint16_t addr=fetch16(gb);
                    push16(gb,gb->cpu.pc);
                    gb->cpu.pc=addr;
                    return 24;
                }
                case 0xC9:{
                    //RET
                    gb->cpu.pc=pop16(gb);
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
