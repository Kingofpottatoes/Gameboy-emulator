#include <raylib.h>
#include <stdint.h>
#include <stdio.h>
#include "../core/gb.h"

#define SCALE 4

static Texture2D screen;
static Color pixels[144 * 160];
//COLORS
static const Color palette[4] = {
    {155, 188, 15, 255},
    {139, 172, 15, 255},
    {48, 98, 48, 255},
    {15, 56, 15, 255}
};




void Window_init(){
    InitWindow(160 * SCALE, 144 * SCALE,"Game Boy");
    SetTargetFPS(60);
    Image Background=GenImageColor(160, 144, BLACK);
    screen=LoadTextureFromImage(Background);
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
        gb_run_frame(&gb);
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
    CloseWindow();
    gb_free(&gb);
    return 0;
}