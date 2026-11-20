#include <raylib.h>
#include <stdint.h>
#include <stdio.h>
#include "../core/gb.h"

#define SCALE 4
#define WAITING_ROOM_SAMPLES_SIZE 8192

static int16_t waiting_room_samples[WAITING_ROOM_SAMPLES_SIZE];
static uint16_t sample_writer=0;
static uint16_t sample_reader=0;
static Texture2D screen;
static AudioStream Audio;
static Color pixels[144 * 160];
//COLORS
static const Color palette[4] = {
    {155, 188, 15, 255},
    {139, 172, 15, 255},
    {48, 98, 48, 255},
    {15, 56, 15, 255}
};

typedef struct { KeyboardKey Key_front; enum Keys key_core; } Key_Links;

static const Key_Links links[] = {
    {KEY_UP,UP},
    {KEY_DOWN,DOWN},
    {KEY_LEFT,LEFT},
    {KEY_RIGHT,RIGHT},
    {KEY_A,A},
    {KEY_D,B},
    {KEY_W,SELECT},
    {KEY_S,START}
};

void waiting_room_write(int16_t samples[],uint16_t sample_count){

    int i;
    for (i=0;i<sample_count;i++){
        uint16_t next_sample_pos=(sample_writer + 1) % WAITING_ROOM_SAMPLES_SIZE;
        if (next_sample_pos==sample_reader){
            return;
        }
        waiting_room_samples[sample_writer]=samples[i];
        sample_writer=next_sample_pos;
    }
}

void SendSound(void * buffer,unsigned int frames){
    unsigned int i;
    int16_t *samples = (int16_t *)buffer;
    for (i=0;i<frames;i++){
        if(sample_reader==sample_writer){
            samples[i]=0;
        }
        else {
            samples[i]=waiting_room_samples[sample_reader];
            sample_reader=(sample_reader+1)% WAITING_ROOM_SAMPLES_SIZE;
        }
    }
}

void Window_init(){
    InitWindow(160 * SCALE, 144 * SCALE,"Game Boy");
    InitAudioDevice();
    SetAudioStreamBufferSizeDefault(512);
    SetTargetFPS(60);
    Image Background=GenImageColor(160, 144, BLACK);
    screen=LoadTextureFromImage(Background);
    Audio=LoadAudioStream(44100, 16, 1);
    SetAudioStreamCallback(Audio, SendSound);
    PlayAudioStream(Audio);
    UnloadImage(Background);
}

int main(int argc, char *argv[]) {
    if(argc<2){
        printf("usage: gb <rom> \n");
        return 1;
    }
    GameBoy gb;
    int result=gb_init(&gb,argv[1]);
    if (result!=0){return 1;}
    Window_init();

    while (!WindowShouldClose()) {
        int key;
        for (key=0;key<8;key++){
            gb_set_button(&gb, links[key].key_core, IsKeyDown(links[key].Key_front));
        }
        printf("bank=%02X PC=%04X\n", gb.cart.rom_bank, gb.cpu.pc);
        printf("PC=%04X\n", gb.cpu.pc);
        gb_run_frame(&gb);
        waiting_room_write(gb.apu.samples, gb.apu.sample_fill);
        gb.apu.sample_fill=0;
        int x,y;
        for (y=0;y<144;y++){
            for (x=0;x<160;x++){
                pixels[y * 160 + x] = palette[gb.framebuffer[y][x]];
            }
        }
        UpdateTexture(screen, pixels);
        BeginDrawing();
        ClearBackground(BLACK);
        DrawTextureEx(screen, (Vector2){0, 0}, 0.0f, SCALE, WHITE);
        EndDrawing();
    }
    UnloadTexture(screen);
    UnloadAudioStream(Audio);
    CloseAudioDevice();
    CloseWindow();
    gb_free(&gb);
    return 0;
}