#include "apu.h"
#include "gb.h"
#include <stdbool.h>
#include <stdint.h>



static const uint8_t motifs[4][8] = {
    {0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,1},
    {1,0,0,0,0,1,1,1},
    {0,1,1,1,1,1,1,0}
};

void apu_trigger_ch2(GameBoy *gb){
    gb->apu.is_canal_on=true;
    gb->apu.volume= (gb->apu.nr22 & 0b11110000)>>4;
    uint16_t period=((gb->apu.nr24) & 0x07)<<8 |gb->apu.nr23;
    gb->apu.sound_counter=(2048-period)*4;

}


void apu_step(GameBoy *gb, int cycles){
    if (gb->apu.is_canal_on){
        gb->apu.sound_counter-=cycles;
        while (gb->apu.sound_counter<=0){
            gb->apu.pos_motif+=1;
            gb->apu.pos_motif=gb->apu.pos_motif%8;
            uint16_t period=((gb->apu.nr24) & 0x07)<<8 |gb->apu.nr23;
            gb->apu.sound_counter+=(2048-period)*4;
        }
    }
    int16_t sample=0;
    gb->apu.time_counter+=cycles*44100;
    while (gb->apu.time_counter>=4194304){
        gb->apu.time_counter-=4194304;
        if (!gb->apu.is_canal_on){
            sample=0;
        }
        else{
            uint8_t motif=gb->apu.nr21>>6;
            uint8_t pas = motifs[motif][gb->apu.pos_motif];
            if (pas==1){
                sample=gb->apu.volume*400;
            }
            else{
                sample=-gb->apu.volume*400;
            }
        }
        if(gb->apu.sample_fill<1024){
            gb->apu.samples[gb->apu.sample_fill]=sample;
            gb->apu.sample_fill+=1;
        }
    }
}