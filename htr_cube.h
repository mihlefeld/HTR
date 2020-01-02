//
// Created by Malte on 02/01/2020.
//

#ifndef HTR_HTR_CUBE_H
#define HTR_HTR_CUBE_H

#endif //HTR_HTR_CUBE_H

struct Cube{
    uint8_t e;
    uint32_t c;
};

inline uint8_t swap_bits_8bit(uint8_t n, uint8_t b1, uint8_t b2, uint8_t mask) {
    uint8_t h = ((n >> b1) & mask) ^ ((n >> b2) & mask);
    return n ^ ((h << b1) | (h << b2));
}

inline uint32_t swap_bits_32bit(uint32_t n, uint32_t b1, uint32_t b2, uint32_t mask) {
    uint32_t h = ((n >> b1) & mask) ^ ((n >> b2) & mask);
    return n ^ ((h << b1) | (h << b2));
}

inline uint8_t cycle_bits_8bit(uint8_t n, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t mask) {
    n = swap_bits_8bit(n,b1,b2,mask);
    n = swap_bits_8bit(n,b1,b3,mask);
    n = swap_bits_8bit(n,b1,b4,mask);
    return n;
}

inline uint32_t cycle_bits_32bit(uint32_t n, uint32_t b1, uint32_t b2, uint32_t b3, uint32_t b4, uint32_t mask) {
    n = swap_bits_32bit(n,b1,b2,mask);
    n = swap_bits_32bit(n,b1,b3,mask);
    n = swap_bits_32bit(n,b1,b4,mask);
    return n;
}

inline uint8_t toggle(uint8_t n, uint8_t b) {
    return n ^ (1 << b);
}

Cube move(Cube cb, uint8_t m) {
    switch(m) {
        case 1:
            // L
            // EO: cycle 7-2-5-3
            // L2 U2 L B2 L' U2 F2 L' B2 L
            cb.e = cycle_bits_8bit(cb.e,7,2,5,3,1);
            cb.e = toggle(cb.e,7);
            cb.e = toggle(cb.e,2);
            cb.e = toggle(cb.e,5);
            cb.e = toggle(cb.e,3),
                    // CP: cycle 7-2-1-6
                    cb.c = cycle_bits_32bit(cb.c,7*3,2*3,1*3,6*3,7);
            return cb;
        case 2:
            // L'
            // EO: cycle 7-3-5-2
            cb.e = cycle_bits_8bit(cb.e,7,3,5,2,1);
            cb.e = toggle(cb.e,7);
            cb.e = toggle(cb.e,2);
            cb.e = toggle(cb.e,5);
            cb.e = toggle(cb.e,3),
                    // CP: cycle 7-6-1-2
                    cb.c = cycle_bits_32bit(cb.c,7*3,6*3,1*3,2*3,7);
            return cb;
        case 3:
            // R
            // EO: cycle 6-1-4-0
            cb.e = cycle_bits_8bit(cb.e,6,1,4,0,1);
            cb.e = toggle(cb.e,6);
            cb.e = toggle(cb.e,1);
            cb.e = toggle(cb.e,4);
            cb.e = toggle(cb.e,0);
            // CP: cycle 4-5-0-3
            cb.c = cycle_bits_32bit(cb.c,4*3,5*3,0*3,3*3,7);
            break;
        case 4:
            // R'
            // EO: cycle 6-0-4-1
            cb.e = cycle_bits_8bit(cb.e,6,0,4,1,1);
            cb.e = toggle(cb.e,6);
            cb.e = toggle(cb.e,1);
            cb.e = toggle(cb.e,4);
            cb.e = toggle(cb.e,0),
                    // CP: cycle 4-3-0-5
                    cb.c = cycle_bits_32bit(cb.c,4*3,3*3,0*3,5*3,7);
            return cb;
        case 5:
            // L2
            // EO: swap bits 7 and 5, swap bits 3 and 2
            cb.e = swap_bits_8bit(cb.e,7,5,1);
            cb.e = swap_bits_8bit(cb.e,3,2,1);
            // CP: swap bytes 7 and 1, swap bytes 6 and 2
            cb.c = swap_bits_32bit(cb.c,7*3,1*3,7);
            cb.c = swap_bits_32bit(cb.c,6*3,2*3,7);
            return cb;
        case 6:
            // R2
            // EO: swap bits 6 and 4, swap bits 1 and 0
            cb.e = swap_bits_8bit(cb.e,6,4,1);
            cb.e = swap_bits_8bit(cb.e,1,0,1);
            // CP: swap bytes 4 and 0, swap bytes 5 and 3
            cb.c = swap_bits_32bit(cb.c,4*3,0*3,7);
            cb.c = swap_bits_32bit(cb.c,5*3,3*3,7);
            return cb;
        case 7:
            // U2
            // EO: swap bits 7 and 6
            cb.e = swap_bits_8bit(cb.e,7,6,1);
            // CP: swap bytes 7 and 5, swap bytes 6 and 4
            cb.c = swap_bits_32bit(cb.c,4*3,6*3,7);
            cb.c = swap_bits_32bit(cb.c,7*3,5*3,7);
            return cb;
        case 8:
            // D2
            // EO: swap bits 4 and 5
            cb.e = swap_bits_8bit(cb.e,4,5,1);
            // CP: swap bytes 1 and 3, swap bytes 0 and 2
            cb.c = swap_bits_32bit(cb.c,0*3,2*3,7);
            cb.c = swap_bits_32bit(cb.c,1*3,3*3,7);
            return cb;
        case 9:
            // F2
            // EO: swap bits 0 and 2
            cb.e = swap_bits_8bit(cb.e,0,2,1);
            // CP: swap bytes 7 and 3, swap bytes 4 and 2
            cb.c = swap_bits_32bit(cb.c,7*3,3*3,7);
            cb.c = swap_bits_32bit(cb.c,4*3,2*3,7);
            return cb;
        case 10:
            // B2
            // EO: swap bits 1 and 3
            cb.e = swap_bits_8bit(cb.e,1,3,1);
            // CP: swap bytes 6 and 0, swap bytes 5 and 1
            cb.c = swap_bits_32bit(cb.c,6*3,0*3,7);
            cb.c = swap_bits_32bit(cb.c,5*3,1*3,7);
            return cb;
        default:
            return cb;
    }
    return cb;
}

