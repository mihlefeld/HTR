#include <stdint.h>
#include <iostream>
#include <vector>
#include <bitset>
#include <string>
#include <ctime>


struct Cube{
    uint8_t e;
    uint32_t c;
};


std::vector<Cube> htr_states_non_unique;
Cube htr_states[96];
// if (lm == 0 || prevent_moves[lm][m])
//                             0 1 2 3 4 5 6 7 8 9 10
bool prevent_moves[11][11] = {{0,0,0,0,0,0,0,0,0,0,0}, // 0
                              {0,0,0,1,1,0,1,1,1,1,1}, // 1
                              {0,0,0,1,1,0,1,1,1,1,1}, // 2
                              {0,1,1,0,0,1,0,1,1,1,1}, // 3
                              {0,1,1,0,0,1,0,1,1,1,1}, // 4
                              {0,0,0,1,1,0,1,1,1,1,1}, // 5
                              {0,1,1,0,0,1,0,1,1,1,1}, // 6
                              {0,1,1,1,1,1,1,0,1,1,1}, // 7
                              {0,1,1,1,1,1,1,1,0,1,1}, // 8
                              {0,1,1,1,1,1,1,1,1,0,1}, // 9
                              {0,1,1,1,1,1,1,1,1,1,0}}; // 10


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

Cube parse_cube(const char* eo_str, const char* cp_str) {
    uint8_t eo = 0;
    uint32_t cp = 0;
    for (uint8_t i = 0; i < 8; i++) {
        eo |= eo_str[i] - '0';
        if(i < 7) {
            eo <<= 1;
        }
        cp |= cp_str[i] - '0';
        cp <<= 3;
    }
    cp >>= 3;
    return {0, cp};
}

std::string parse_cube_inv(Cube cb) {
    std::string ret = "";
    for (int i = 7; i >= 0; i--) {
        ret += (char) (cb.e >> i & 1) + '0';
    }
    ret += " ";
    for (int i = 7; i >= 0; i--) {
        ret += (char) (cb.c >> i*3 & 7) + '0';
    }
    return ret;
}

void init_htr_states_non_unique(Cube cb, uint8_t d, uint8_t lm) {
    cb = move(cb,lm);
    htr_states_non_unique.push_back(cb);
    if (d == 0) return;
    for (uint8_t m = 5; m <= 10; m++) {
        init_htr_states_non_unique(cb, d - 1, m);
    }
}

 bool operator==(Cube a, Cube b) {
    return a.e == b.e && a.c == b.c;
}

void init_htr_states() {
    init_htr_states_non_unique(parse_cube("00000000", "01234567"),4,0);
    int i = 0;
    for(Cube cb: htr_states_non_unique) {
        bool exists = false;
        for(int j = 0; j < i; j++) {
            if(cb == htr_states[j]) {
                exists = true;
            }
        }
        if(!exists) {
            htr_states[i] = cb;
            i++;
        }
    }
}

bool is_htr(Cube cb) {
    for (uint8_t i = 0; i < 96; i++) {
        if(cb == htr_states[i]) return true;
    }
    return false;
}

bool search(Cube cb, uint8_t d, uint8_t sd, uint8_t lm, uint8_t* moves) {
    cb = move(cb,lm);
    if (d == 1) {
        Cube cb2 = cb;
        if (lm == 0 || prevent_moves[lm][1]) {
            cb = move(cb,1);
            if (is_htr(cb)) {
                moves[sd-d] = 1;
                std::cout << 1 << " ";
                return true;
            }
        }
        if(lm == 0 || prevent_moves[lm][3]) {
            cb = move(cb2,3);
            if (is_htr(cb)) {
                moves[sd-d] = 3;
                std::cout << 3 << " ";
                return true;
            }
        }
        return false;
    }
    for (int m = 1; m <= 10; m++) {
        if (lm == 0 || prevent_moves[lm][m]) {
            bool b = search(cb, d - 1, sd, m, moves);
            if (b){
                moves[sd-d] = m;
                std::cout << m << " ";
                if (d==sd) {
                    std::cout << std::endl;
                }
                return true;
            }
        }
    }
    return false;
}

bool search(Cube cb, uint8_t d, uint8_t* moves) {
    if (is_htr(cb)) return true;
    return search(cb, d, d,0, moves);
}

std::string convert_to_wca_notation(uint8_t* int_moves, uint8_t count) {
    std::string wca_moves = "";
    for (int i = 0; i < count; i++) {
        switch(int_moves[i]) {
            case 1:
                wca_moves += "L ";
                break;
            case 2:
                wca_moves += "L' ";
                break;
            case 3:
                wca_moves += "R ";
                break;
            case 4:
                wca_moves += "R' ";
                break;
            case 5:
                wca_moves += "L2 ";
                break;
            case 6:
                wca_moves += "R2 ";
                break;
            case 7:
                wca_moves += "U2 ";
                break;
            case 8:
                wca_moves += "D2 ";
                break;
            case 9:
                wca_moves += "F2 ";
                break;
            case 10:
                wca_moves += "B2 ";
                break;

        }
    }
    return wca_moves;
}

int main(int argc, char** argv) {
    init_htr_states();
    int depth = 10;
    Cube cb;
    if(argc >= 2) {
        depth = std::stoi(argv[1]);
    }
    if(argc == 4) {
        cb = parse_cube(argv[2], argv[3]);
    }
    else {
        char eo[8];
        char cp[8];
        std::cout << "Enter eo: " << std::endl;
        std::cin >> eo;
        std::cout << "Enter cp: " << std::endl;
        std::cin >> cp;
        cb = parse_cube(eo, cp);
    }

    std::cout << "{";
    for (int i = 0; i < 96; i++) {
        Cube cb = htr_states[i];
        std::bitset<24> bits(cb.c);
        if(i == 95) {
            std::cout << bits << "};" << std::endl;
        }
        else {
            std::cout << bits << "," << std::endl;
        }
    }
    std::cout << "}";

    cb = parse_cube("00000000", "01234567");
    //F2 U2 L D2 L2 F2 L U2 R
    cb = move(cb, 4); cb = move(cb, 7); cb = move(cb, 2);
    cb = move(cb, 9); cb = move(cb, 5); cb = move(cb, 8);
    cb = move(cb, 2); cb = move(cb, 7); cb = move(cb, 9);
    std::cout << parse_cube_inv(cb) << std::endl;
    std::clock_t t0 = std::clock();
    for(int d = 1; d <= depth; d++) {
        uint8_t* moves = new uint8_t[d+2];
        std::cout << "Searching depth " << d << std::endl;
        if(search(cb, d, moves)) {
            std::cout << "Found HTR in " << d << " moves" << std::endl;
            std::cout << convert_to_wca_notation(moves, d) << std::endl;
            std::cout << "Done in " << (float)(std::clock() - t0)/CLOCKS_PER_SEC << std::endl;
            break;
        }
    }
    std::cout << 1;
    return 0;
}
