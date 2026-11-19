#ifndef APU_H
#define APU_H
#include <stdint.h>
#include <stdbool.h>


typedef struct {
    int16_t samples[1024];
    uint16_t sample_fill;
    int32_t time_counter;
    uint8_t envelope_timer;
    uint8_t length_timer;


    uint8_t nr21;
    uint8_t nr22;
    uint8_t nr23;
    uint8_t nr24;

    
    int sequencer_counter;
    int sound_counter;
    uint8_t pos_motif;
    uint8_t sequencer_step;
    uint8_t volume;
    bool is_canal_on;
} APU;

typedef struct GameBoy GameBoy;


void apu_init(GameBoy *gb);
void apu_step(GameBoy *gb, int cycles);

void apu_trigger_ch2(GameBoy *gb);
void apu_write_n21(GameBoy *gb,uint8_t value);
#endif