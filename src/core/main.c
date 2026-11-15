#include "gb.h"
#include <stdio.h>


int main(int argc, char *argv[]){
    if(argc<2){
        printf("usage: gb <rom> \n");
        return 1;
    }
    GameBoy gb;
    int result=gb_init(&gb,argv[1]);
    if (result!=0){return 1;}
    cart_print_header(&gb.cart);
    printf("%u \n",read8(&gb,0x0134));
    write8(&gb,0xC000,42);
    printf("0x%02X \n",read8(&gb,0xC000));
    gb_free(&gb);
}