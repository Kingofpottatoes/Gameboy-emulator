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

    int sequencer_counter;
    uint8_t sequencer_step;

    SquareChannel ch1;
    uint8_t nr10;
    SquareChannel ch2;

} APU;

typedef struct GameBoy GameBoy;


void apu_init(GameBoy *gb);
void apu_step(GameBoy *gb, int cycles);

void square_trigger(SquareChannel *ch);
void square_write_nrx1(SquareChannel *ch,uint8_t value);
#endif