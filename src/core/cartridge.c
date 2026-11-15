#include "cartridge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int cart_load(Cartridge *c,const char *rom_path){

    FILE * file=fopen(rom_path,"rb");
    
    if (file==NULL){return 1;}

    fseek(file,0,SEEK_END);
    long end=ftell(file);

    if (end==-1){
        fclose(file);
        return 1;
    }

    rewind(file);
    c->rom=malloc(end);

    if (c->rom==NULL){
        fclose(file);
        return 1;
    }

    size_t size_read=fread(c->rom,1,end,file);
    fclose(file);   
    if (size_read!=(size_t) end){
        free(c->rom);
        c->rom = NULL;
        return 1;
    }
    if (end< 0x150){
        free(c->rom);
        c->rom = NULL;
        return 1;
    }
    c->rom_size=end;
    memcpy(c->title,c->rom+0x134,16);
    c->title[16]='\0';
    c->mbc_type=c->rom[0x147];
    switch (c->rom[0x149]){
        case 0:{
            c->ram_size=0;
            break;
        }
        case 2:{
            c->ram_size=8*1024;
            break;
        }
        case 3:{
            c->ram_size=32*1024;
            break;
        }
        case 4:{
            c->ram_size=128*1024;
            break;
        }
        case 5:{
            c->ram_size=64*1024;
            break;
        }
        default:{
            free(c->rom);
            c->rom = NULL;
            return 1;
        }
    }
    return 0;
}

void cart_free(Cartridge *c){
    free(c->rom);
    c->rom=NULL;
}
void cart_print_header(const Cartridge *c){
    printf("title:%s \n",c->title);
    printf("rom size:%zu Ko \n",c->rom_size/1024);
    printf("mbc type : 0x%02x \n",c->mbc_type);
    printf("ram size : %zu \n",c->ram_size);

}
