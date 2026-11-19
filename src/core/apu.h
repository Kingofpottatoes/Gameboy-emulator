#ifndef APU_H
#define APU_H
#include <stdint.h>
#include <stdbool.h>




typedef struct {

    uint8_t nrx1;
    uint8_t nrx2;
    uint8_t nrx3;
    uint8_t nrx4;

    int sound_counter;
    uint8_t pos_motif;
    uint8_t volume;
    bool is_canal_on;
    uint8_t envelope_timer;
    uint8_t length_timer;

} SquareChannel;


typedef struct {
    int16_t samples[1024];
    uint16_t sample_fill;
    int32_t time_counter;
    uint8_t sweep_timer;

    int sequencer_counter;
    uint8_t sequencer_step;
    //chanel 1
    SquareChannel ch1;
    uint8_t nr10;
    //channel 2
    SquareChannel ch2;
    //channel 3
    int16_t ch3_lenght_counter;
    bool is_chanel3_on;
    uint8_t ch3_wave_ram[16];
    uint8_t ch3_wave_pos;
    uint16_t ch3_wave_counter;
    uint8_t nr30;
    uint8_t nr31;
    uint8_t nr32;
    uint8_t nr33;
    uint8_t nr34;



} APU;

typedef struct GameBoy GameBoy;


void apu_init(GameBoy *gb);
void apu_step(GameBoy *gb, int cycles);

void square_trigger(SquareChannel *ch);
void apu_trigger_ch1(APU *apu);
void square_write_nrx1(SquareChannel *ch,uint8_t value);

void apu_trigger_ch3(APU *apu);
#endif