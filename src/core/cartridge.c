#include "cartridge.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int cart_load(Cartridge *c,const char *rom_path){
    char *p=strrchr(rom_path,'.');
    int len;
    if (p==NULL){len =(int)(strlen(rom_path));}
    else{len =(int)(p-rom_path);}
    snprintf(c->save_path, 512,"%.*s.sav",len,rom_path);


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
    switch (c->mbc_type){
        case 0x00:{
            c->mbc=MBC_NONE;
            break;
        }
        case 0x01:
        case 0x02:
        case 0x03:{
            c->mbc=MBC1;
            c->bank_low=1;
            c->bank_high=0;
            c->mode=0;
            c->is_ram_activated=false;
            break;
        }
        case 0x0F:
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:{
            c->mbc=MBC3;
            c->rom_bank=1;
            c->ram_bank=0;
            c->is_ram_activated=false;
            break;
        }
        default:{
            printf("unsupported MBC type \n");
            cart_free(c);
            return 1;
        }
        
    }
    switch (c->rom[0x149]){
        case 0:{
            c->ram_size=0;

            break;
        }
        case 1:{
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
    if (c->ram_size>0){
        c->ram=calloc(c->ram_size,sizeof(uint8_t));
        if (c->ram==NULL){
            free(c->rom);
            c->rom=NULL;
            return 1;
        }
        FILE * f=fopen(c->save_path,"rb");
        if (f!=NULL){
            fread(c->ram,c->ram_size, sizeof(uint8_t),f);
            fclose(f);
        }

    }
    return 0;
}


uint8_t cart_read(Cartridge *c,uint16_t addr){
    if (c->mbc==MBC_NONE){
        if (addr>=0x0000 && addr<=0x3FFF){
            return c->rom[addr];
        }
        else if (addr>=0x4000 && addr<=0x7FFF){
            return c->rom[addr];
        }
        else if (addr>=0xA000 && addr<=0xBFFF){
            return 0xFF;
        }
    }
    else if (c->mbc==MBC1){
        if (addr>=0x0000 && addr<=0x3FFF){
            if (c->mode==0){
                return c->rom[addr];
            }
            else {
                return c->rom[((c->bank_high << 5)*0x4000)%c->rom_size +addr];
            }
        }
        else if (addr>=0x4000 && addr<=0x7FFF){
            return c->rom[((c->bank_high << 5 | (c->bank_low))*0x4000)% c->rom_size +(addr-0x4000)] ;
        }
        else if (addr>=0xA000 && addr<=0xBFFF){
            if ((!c->is_ram_activated) || c->ram==NULL){
                return 0xFF;
            }
            else {
                if (c->mode==0){
                    return c->ram[(addr-0xA000)];
                }
                else{
                    return c->ram[((c->bank_high)*0x2000)%c->ram_size +(addr-0xA000)];
                }
            }
        }
    }
    else if (c->mbc==MBC3){
        if (addr>=0x0000 && addr<=0x3FFF){
            return c->rom[addr];
        }
        else if (addr>=0x4000 && addr<=0x7FFF){
            return c->rom[((c->rom_bank*0x4000)%c->rom_size)+(addr-(0x4000))];
        }
        else if (addr>=0xA000 && addr<=0xBFFF){
            if ((!c->is_ram_activated) || c->ram==NULL){
                return 0xFF;
            }
            else {
                return c->ram[((c->ram_bank*0x2000)%c->ram_size)+(addr-0xA000)];
            }
        }
    }
    else{
        return 0xFF;
    }
    return 0xFF;
}

void cart_write(Cartridge *c,uint16_t addr,uint8_t value){
    if (c->mbc==MBC_NONE && addr <= 0x7FFF){return;}
    if (c->mbc==MBC1){
        if (addr>=0x0000 && addr<=0x1FFF){
            if ((value & 0b00001111)==0xA){
                c->is_ram_activated=true;
            }
            else {
                c->is_ram_activated=false;
            }
        }
        else if (addr>=0x2000 && addr<=0x3FFF){
            c->bank_low=value & 0b00011111;
            if (c->bank_low==0){c->bank_low=1;}
        }
        else if (addr>=0x4000 && addr<=0x5FFF){
            c->bank_high=value & 0b00000011;
        }
        else if (addr>=0x6000 && addr<=0x7FFF){
            c->mode=value & 0b00000001;
        }
        else if (addr>=0xA000 && addr<=0xBFFF){
            uint8_t ram_bank;
            if (c->mode==0){ram_bank=0;}
            else{ram_bank=c->bank_high;}
            if (c->is_ram_activated && c->ram!=NULL){
                c->ram[((ram_bank*0x2000)%c->ram_size)+(addr-0xA000)]=value;
            }
        }
    }
    if (c->mbc==MBC3){
        if (addr>=0x0000 && addr<=0x1FFF){
            if ((value & 0b00001111)==0xA){
                c->is_ram_activated=true;
            }
            else {
                c->is_ram_activated=false;
            }
        }
        else if (addr>=0x2000 && addr<=0x3FFF){
            uint8_t rom_bank=value & 0b01111111;
            if (rom_bank==0){
                rom_bank=1;
            }
            c->rom_bank=rom_bank;
        }
        else if (addr>=0x4000 && addr<=0x5FFF){
            if (value<=3){
                uint8_t ram_bank=value;
                c->ram_bank=ram_bank;
            }
        }
        else if (addr>=0x6000 && addr<=0x7FFF){
            //RTC clock todo
        }
        else if (addr>=0xA000 && addr<=0xBFFF){
            if (c->is_ram_activated && c->ram!=NULL){
                c->ram[((c->ram_bank*0x2000)%c->ram_size)+(addr-0xA000)]=value;
            }
        }
    }

    
}

void cart_save(Cartridge *c){
    if ((c->mbc_type==0x03)||(c->mbc_type==0x10)||(c->mbc_type==0x13)||(c->mbc_type==0x0F)){
        //if its a stack MBC types
        //we save in a .sav
        //no need to free since we dont stop the Game Boy
        if (c->ram==NULL){
            return;
        }
        FILE *f=fopen(c->save_path,"wb");
        if (f==NULL){
            return;
        }
        fwrite(c->ram,c->ram_size,sizeof(uint8_t),f);
        fclose(f);

    }
}

void cart_free(Cartridge *c){
    cart_save(c);
    free(c->rom);
    free(c->ram);
    c->rom=NULL;
    c->ram=NULL;
}

void cart_print_header(const Cartridge *c){
    printf("title:%s \n",c->title);
    printf("rom size:%zu Ko \n",c->rom_size/1024);
    printf("mbc type : 0x%02x \n",c->mbc_type);
    printf("ram size : %zu \n",c->ram_size);

}
