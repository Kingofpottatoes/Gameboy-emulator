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

static const uint8_t base[8]={
    8,16,32,48,64,80,96,112
    
};

static uint16_t square_get_period(SquareChannel *ch){
    return ((ch->nrx4) & 0x07)<<8 |ch->nrx3;
}
static void square_set_period(SquareChannel *ch,uint16_t period){
    ch->nrx4=(ch->nrx4 & 0b11111000)|((period >> 8) & 0b111);
    ch->nrx3=period & 0b0000000011111111;
}
static uint32_t get_lfsr_step(APU *apu){
    uint8_t shift=(apu->ch4.nrx3 & 0b11110000)>>4;
    uint8_t divider=apu->ch4.nrx3 & 0b00000111;
    return base[divider]<<shift;
}

void square_trigger(SquareChannel *ch){
    ch->is_canal_on=true;
    ch->volume= (ch->nrx2 & 0b11110000)>>4;
    uint16_t period=square_get_period(ch);
    ch->sound_counter=(2048-period)*4;
    ch->envelope_timer=((ch->nrx2) & 0b00000111);
    if (ch->length_timer==0){
        ch->length_timer=64;
    }

}

void apu_trigger_ch1(APU *apu){
    square_trigger(&apu->ch1);
    apu->sweep_timer=(apu->nr10 & 0b01110000)>>4;
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
            uint16_t period=square_get_period(ch);
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
static void sweep_tick(APU *apu){
    uint8_t shift=apu->nr10 & 0b00000111;
    uint8_t direction=(apu->nr10 & 0b00001000)>>3;
    uint8_t rythme=(apu->nr10 & 0b01110000)>>4;
    if (rythme==0){return;}
    else {
        apu->sweep_timer-=1;
        if (apu->sweep_timer!=0){return;}
        else {
            apu->sweep_timer=rythme;
        }
    }
    uint16_t period=square_get_period(&apu->ch1);
    if (shift==0){return;}
    if (direction==0){
        period+=(period>>shift);
    }
    else{
        period-=(period>>shift);
    }
    if (period>2047){apu->ch1.is_canal_on=false;return;}
    square_set_period(&apu->ch1,period);
}

void apu_trigger_ch3(APU *apu){
    if (((apu->nr30)&0b10000000)>>6==0){return;}
    apu->ch3_wave_pos=0;
    uint16_t period=((apu->nr34) & 0x07)<<8 | apu->nr33;
    apu->ch3_wave_counter=(2048-period) * 2;
    if (apu->ch3_lenght_counter==0){
        apu->ch3_lenght_counter=256;
    }
    apu->is_chanel3_on=true;
}

static void ch3_step(APU *apu, int cycles){
    if (apu->is_chanel3_on){
        apu->ch3_wave_counter-=cycles;
        while (apu->ch3_wave_counter<=0){
            apu->ch3_wave_pos+=1;
            apu->ch3_wave_pos=apu->ch3_wave_pos%32;
            uint16_t period=((apu->nr34) & 0x07)<<8 |apu->nr33;
            apu->ch3_wave_counter+=(2048-period)*2;
        }
    }
}

static int16_t ch3_output(APU *apu){
    int16_t sample=0;
    uint8_t volume= (apu->nr32 & 0b01100000)>>5;
    if (!apu->is_chanel3_on){
        sample=0;
        return sample;
    }
    sample=apu->ch3_wave_ram[apu->ch3_wave_pos/2];
    if (apu->ch3_wave_pos%2==0){
        sample=(sample & 0b11110000)>>4;
    }
    else{
        sample=(sample & 0b00001111);
    }
    switch (volume){
        case 0:{return 0;break;}//mute
        case 1:{break;}//100%
        case 2:{sample=sample/2;break;}//50%
        case 3 :{sample=sample/4;break;}//25%
    }
    sample=(2*sample -15)*400;
    return sample;
}

static void ch3_length_tick(APU *apu){
    if ((((apu->nr34 & (1 <<6))>>6) ==1) && (apu->ch3_lenght_counter>0)){
        apu->ch3_lenght_counter-=1;
        if (apu->ch3_lenght_counter==0){
            apu->is_chanel3_on=false;
        }
    }
}


void apu_trigger_ch4(APU *apu){
    apu->ch4.is_canal_on=true;
    apu->ch4_lfsr=0b111111111111111;
    apu->ch4.volume= (apu->ch4.nrx2 & 0b11110000)>>4;
    apu->ch4.sound_counter=get_lfsr_step(apu);
    apu->ch4.envelope_timer=((apu->ch4.nrx2) & 0b00000111);
    if (apu->ch4.length_timer==0){
        apu->ch4.length_timer=64;
    }

}

static void ch4_step(APU *apu, int cycles){
    if (apu->ch4.is_canal_on){
        apu->ch4.sound_counter-=cycles;
        uint32_t step=get_lfsr_step(apu);
        while (apu->ch4.sound_counter<=0){
            apu->ch4.sound_counter+=step;
            uint8_t r=(apu->ch4_lfsr & 1)^((apu->ch4_lfsr >>1)&1);
            apu->ch4_lfsr=(r<<14)|(apu->ch4_lfsr>>1);
            if (((apu->ch4.nrx3>>3)&1)==1){
                apu->ch4_lfsr=apu->ch4_lfsr & ~(1 << 6);
                apu->ch4_lfsr=apu->ch4_lfsr|r << 6;
            }
        }
    }
}
static int16_t ch4_output(APU *apu){
    if (!apu->ch4.is_canal_on){return 0;}
    else {
        if ((apu->ch4_lfsr & 1)==0){
            return apu->ch4.volume *400;
        }
        else{
            return -apu->ch4.volume *400;
        }
    }
}
void apu_step(GameBoy *gb, int cycles){
    square_step(&gb->apu.ch2, cycles);
    square_step(&gb->apu.ch1,cycles);
    ch3_step(&gb->apu, cycles);
    ch4_step(&gb->apu,cycles);
    int16_t sample=0;
    gb->apu.time_counter+=cycles*44100;
    while (gb->apu.time_counter>=4194304){
        gb->apu.time_counter-=4194304;
        sample=square_output(&gb->apu.ch2)+square_output(&gb->apu.ch1)+ch3_output(&gb->apu)+ch4_output(&gb->apu);
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
                square_envelope_tick(&gb->apu.ch4);
            }
            if (gb->apu.sequencer_step%2==0){
                square_length_tick(&gb->apu.ch2);
                square_length_tick(&gb->apu.ch1);
                ch3_length_tick(&gb->apu);
                square_length_tick(&gb->apu.ch4);
            }
            if (gb->apu.sequencer_step==2 ||gb->apu.sequencer_step==6){
                sweep_tick(&gb->apu);
            }
    }
}