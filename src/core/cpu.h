#ifndef CPU_H
#define CPU_H
#define FLAG_Z 0x80
#define FLAG_N 0x40
#define FLAG_H 0x20
#define FLAG_C 0x10
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    union {
        struct {
            uint8_t c;
            uint8_t b;
        };
        uint16_t bc;
    };
    union {
        struct {
            uint8_t e;
            uint8_t d;
        };
        uint16_t de;
    };
    union {
        struct {
            uint8_t l;
            uint8_t h;
        };
        uint16_t hl;
    };
    union {
        struct {
            uint8_t f;
            uint8_t a;
        };
        uint16_t af;
    };
    uint16_t sp;
    uint16_t pc;
    bool ime;
    uint8_t ime_delay;
    bool halted;
}CPU;

typedef struct GameBoy GameBoy ;

void cpu_init(CPU *);
int cpu_step(GameBoy *);

#endif