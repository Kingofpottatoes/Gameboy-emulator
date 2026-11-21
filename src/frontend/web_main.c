#include <raylib.h>
#include <stdint.h>
#include <emscripten/emscripten.h>
#include "../core/gb.h"
#include <stdio.h>

static GameBoy gb;
static bool rom_loaded=false;
static bool load_error=false;
static bool audio_loaded=false;
#define SCALE 4
#define WAITING_ROOM_SAMPLES_SIZE 8192

static int16_t waiting_room_samples[WAITING_ROOM_SAMPLES_SIZE];
static uint16_t sample_writer=0;
static uint16_t sample_reader=0;
static Texture2D screen;
static AudioStream Audio;
static float current_volume=1;
static Color pixels[144 * 160];
//COLORS
static const Color palette_Green[4] = {
    {155, 188, 15, 255},
    {139, 172, 15, 255},
    {48, 98, 48, 255},
    {15, 56, 15, 255}
};
// Grayscale
static const Color palette_White[4] ={
    {224, 224, 224, 255},
    {160, 160, 160, 255},
    {96, 96, 96, 255},
    {32, 32, 32, 255}
};

// Blue
static const Color palette_Blue[4] ={
    {224, 240, 255, 255},
    {124, 170, 230, 255},
    {44, 88, 160, 255},
    {12, 28, 72, 255}
};
// Night
static const Color palette_Black[4] ={
    {16, 18, 32, 255},
    {56, 62, 98, 255},
    {130, 140, 180, 255},
    {220, 225, 240, 255}
};

static const Color *selected_palette=palette_Green;
int count_waiters_in_waiting_room(){
    return (sample_writer -sample_reader + WAITING_ROOM_SAMPLES_SIZE) % WAITING_ROOM_SAMPLES_SIZE;
}

void waiting_room_write(int16_t samples[],uint16_t sample_count){
    int i;
    for (i=0;i<sample_count;i+=2){
        uint16_t next_sample_pos=(sample_writer + 2) % WAITING_ROOM_SAMPLES_SIZE;
        if (WAITING_ROOM_SAMPLES_SIZE - 1 - count_waiters_in_waiting_room()<2){
            return;
        }
        waiting_room_samples[sample_writer]=samples[i];
        waiting_room_samples[(sample_writer+1)%WAITING_ROOM_SAMPLES_SIZE]=samples[i+1];
        sample_writer=next_sample_pos;
    }
}
void SendSound(void * buffer,unsigned int frames){
    unsigned int i;
    int16_t *samples = (int16_t *)buffer;
    for (i=0;i<frames;i++){
        if(count_waiters_in_waiting_room()<2){
            samples[2*i]=0;
            samples[2*i+1]=0;
        }
        else {
            samples[2*i]=waiting_room_samples[sample_reader];
            samples[2*i +1]=waiting_room_samples[(sample_reader+1)% WAITING_ROOM_SAMPLES_SIZE];
            sample_reader=(sample_reader+2)% WAITING_ROOM_SAMPLES_SIZE;
        }
    }
}
static void DrawMsg(const char *msg,Color c){
    BeginDrawing();
    ClearBackground(BLACK);
    DrawText(msg,((160 * SCALE)-MeasureText(msg,24))/2, ((144 * SCALE)-24)/2,24,c);
    EndDrawing();
}

void audio_init(){
    InitAudioDevice();
    SetAudioStreamBufferSizeDefault(512);
    Audio=LoadAudioStream(44100, 16, 2);
    SetAudioStreamCallback(Audio, SendSound);
    PlayAudioStream(Audio);

}

void web_load_rom(const char *rom_path){
    if (rom_loaded==true){
        gb_free(&gb);
        rom_loaded=false;
    }
    if (!audio_loaded){
        audio_init();
        audio_loaded=true;
    }
    int r=gb_init(&gb,rom_path);
    if (r!=0){
        load_error=true;
        return;
    }
    rom_loaded=true;
    load_error=false;
}

void Window_init(){
    InitWindow(160 * SCALE, 144 * SCALE,"Game Boy");
    SetTargetFPS(60);
    Image Background=GenImageColor(160, 144, BLACK);
    screen=LoadTextureFromImage(Background);
    UnloadImage(Background);
}

EMSCRIPTEN_KEEPALIVE
void save(){
    if (rom_loaded){
        cart_save(&gb.cart);
    }
}
EMSCRIPTEN_KEEPALIVE
void load(const char *rom_path){
    web_load_rom(rom_path);
}
EMSCRIPTEN_KEEPALIVE
void web_set_keyboard(uint8_t state){
    int i;
    for (i=0;i<8;i++){
        gb_set_button(&gb,i,(state >> i) & 1);
    }
}
EMSCRIPTEN_KEEPALIVE
void set_volume(float volume){
    current_volume=volume;
    if (audio_loaded){
    SetMasterVolume(volume);
    }
}
EMSCRIPTEN_KEEPALIVE
void change_color(int color){
    switch (color) {
        case 0:selected_palette=palette_Green;break;
        case 1:selected_palette=palette_White;break;
        case 2:selected_palette=palette_Blue;break;
        case 3:selected_palette=palette_Black;break;
    
    }
}

EMSCRIPTEN_KEEPALIVE
void load_save(){
    bool was_loaded=rom_loaded;
    if (rom_loaded){
        gb_free(&gb);
        rom_loaded=false;
    }
    rename("/import.sav", "/rom.sav");
    if (was_loaded){
        web_load_rom("/rom.gb");
    }
}
void play_frame(){
    if (load_error){
        DrawMsg("Cant load this file",RED);
        return;
    }
    if (!rom_loaded){
        DrawMsg("Load a rom to start",GRAY);
        return;
    }
    if (!(count_waiters_in_waiting_room()>4096)){
        gb_run_frame(&gb);
        waiting_room_write(gb.apu.samples, gb.apu.sample_fill);
        gb.apu.sample_fill=0;
    }
    int x,y;
    for (y=0;y<144;y++){
        for (x=0;x<160;x++){
            pixels[y * 160 + x] = selected_palette[gb.framebuffer[y][x]];
        }
    }
    UpdateTexture(screen, pixels);
    BeginDrawing();
    ClearBackground(BLACK);
    DrawTextureEx(screen, (Vector2){0, 0}, 0.0f, SCALE, WHITE);
    EndDrawing();
}

int main() {
    Window_init();

    emscripten_set_main_loop(play_frame, 0, 1);

    UnloadTexture(screen);
    UnloadAudioStream(Audio);
    CloseAudioDevice();
    CloseWindow();
    gb_free(&gb);
    return 0;
}