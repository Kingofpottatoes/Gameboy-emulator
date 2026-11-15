#include "cpu.h"
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
    while (1){
        cpu_step(&gb);
    }
}