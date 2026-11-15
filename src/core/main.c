#include "cartridge.h"
#include <stdio.h>


int main(int argc, char *argv[]){
    if(argc<2){
        printf("usage: gb <rom> \n");
        return 1;
    }
    Cartridge cart;
    cart_load(&cart, argv[1]);
    cart_print_header(&cart);
}