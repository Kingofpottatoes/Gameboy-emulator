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

void square_trigger(SquareChannel *ch){
    ch->is_canal_on=true;
    ch->volume= (ch->nrx2 & 0b11110000)>>4;
    uint16_t period=((ch->nrx4) & 0x07)<<8 |ch->nrx3;
    ch->sound_counter=(2048-period)*4;
    ch->envelope_timer=((ch->nrx2) & 0b00000111);
    if (ch->length_timer==0){
        ch->length_timer=64;
    }

}
void square_write_nrx1(SquareChannel *ch,uint8_t value){
    ch->nrx1=value;
    ch->length_timer=64 -((ch->nrx1) & 0b00111111);
}

static void square_step(SquareChannel *ch,int cycles){
    if (ch->is_canal_on){
        ch->sound_counter-=cycles;
        while (ch->sound_counter<=0){
            ch->pos_motif+=1;
            ch->pos_motif=ch->pos_motif%8;
            uint16_t period=((ch->nrx4) & 0x07)<<8 |ch->nrx3;
            ch->sound_counter+=(2048-period)*4;
        }
    }
}

static int16_t square_output(SquareChannel *ch){
    int16_t sample=0;
    if (!ch->is_canal_on){
            sample=0;
        }
    else{
        uint8_t motif=ch->nrx1>>6;
        uint8_t pas = motifs[motif][ch->pos_motif];
        if (pas==1){
            sample=ch->volume*400;
        }
        else{
            sample=-ch->volume*400;
        }
    }
    return sample;
}

static void square_envelope_tick(SquareChannel *ch){
    uint8_t env_period=((ch->nrx2) & 0b00000111);
    uint8_t direction =((ch->nrx2) & 0b00001000)>>3;
    if (env_period!=0){
        ch->envelope_timer-=1;
        if(ch->envelope_timer==0){
            ch->envelope_timer=env_period;
            if (direction==1 && ch->volume<15){
                ch->volume+=1;
            }
            else if (direction==0 && ch->volume>0) {
                ch->volume-=1;
            }
        }
    }
}

static void square_length_tick(SquareChannel *ch){
    if ((((ch->nrx4 & (1 <<6))>>6) ==1) && (ch->length_timer>0)){
        ch->length_timer-=1;
        if (ch->length_timer==0){
            ch->is_canal_on=false;
        }
    }
}

void apu_step(GameBoy *gb, int cycles){
    square_step(&gb->apu.ch2, cycles);
    square_step(&gb->apu.ch1,cycles);
    int16_t sample=0;
    gb->apu.time_counter+=cycles*44100;
    while (gb->apu.time_counter>=4194304){
        gb->apu.time_counter-=4194304;
        sample=square_output(&gb->apu.ch2)+square_output(&gb->apu.ch1);
        if(gb->apu.sample_fill<1024){
            gb->apu.samples[gb->apu.sample_fill]=sample;
            gb->apu.sample_fill+=1;
        }
    }
    gb->apu.sequencer_counter+=cycles;
    while (gb->apu.sequencer_counter>=8192){
            gb->apu.sequencer_counter-=8192;
            gb->apu.sequencer_step=(gb->apu.sequencer_step+1)%8;
            if (gb->apu.sequencer_step==7){
                square_envelope_tick(&gb->apu.ch2);
                square_envelope_tick(&gb->apu.ch1);
            }
            if (gb->apu.sequencer_step%2==0){
                square_length_tick(&gb->apu.ch2);
                square_length_tick(&gb->apu.ch1);
            }
    }
}