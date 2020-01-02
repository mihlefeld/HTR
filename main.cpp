#include <stdint.h>
#include <iostream>
#include <vector>
#include <bitset>
#include <string>
#include <ctime>
#include <omp.h>
#include "htr_cube.h"

#define MAX_DEPTH 13

std::vector<uint32_t> htr_states_non_unique;
uint32_t htr_states[96];
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
    return {eo, cp};
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
    htr_states_non_unique.push_back(cb.c);
    if (d == 0) return;
    for (uint8_t m = 5; m <= 10; m++) {
        init_htr_states_non_unique(cb, d - 1, m);
    }
}

void init_htr_states() {
    init_htr_states_non_unique(parse_cube("00000000", "01234567"),4,0);
    int i = 0;
    for(uint32_t cb: htr_states_non_unique) {
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
    if(cb.e != 0) return false;
    if((cb.c & 0b00000000001001001001001001001001) != 0b00000000000001000001000001000001) return false;
    bool is_htr = false;
#pragma omp simd
    for (uint8_t i = 0; i < 96; i++) {
        is_htr |= cb.c == htr_states[i];
    }
    return is_htr;
}

bool search(Cube cb, uint8_t d, uint8_t sd, uint8_t lm, uint8_t* moves) {
    cb = move(cb,lm);
    if (d == 1) {
        Cube cb2 = cb;
        if (lm == 0 || prevent_moves[lm][1]) {
            cb = move(cb,1);
            if (is_htr(cb)) {
                moves[sd-d] = 1;
                return true;
            }
        }
        if(lm == 0 || prevent_moves[lm][3]) {
            cb = move(cb2,3);
            if (is_htr(cb)) {
                moves[sd-d] = 3;
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
                return true;
            }
        }
    }
    return false;
}

bool call_search(Cube cb, uint8_t d, uint8_t* moves) {
    return search(cb, d, d,0, moves);
}

/**
 * 
 * @param int_moves
 * @param count
 * @return
 */
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
    if(argc >= 4) {
        depth = std::stoi(argv[3]);
        cb = parse_cube(argv[1], argv[2]);
    }

    bool findall = false;

    std::cout << (int) cb.c << std::endl;


    // standard test: 00100010 02173654
    std::clock_t t0 = std::clock();
    for(int d = 1; d <= depth; d++) {
        std::cout << "Searching depth " << d << std::endl;
        if(findall && d > 1) {
#pragma omp parallel for
            for(uint8_t i = 1; i <= 10; i++) {
                uint8_t moves[MAX_DEPTH];
                moves[0] = i;

                if(search(cb, d-1, d, i, moves)) {
                    std::cout << "Found HTR in " << d << " moves: " << convert_to_wca_notation(moves, d) << std::endl;
                }
            }
        } else {
            uint8_t moves[MAX_DEPTH];
            if(call_search(cb, d, moves)) {
                std::cout << "Found HTR in " << d << " moves" << std::endl;
                std::cout << convert_to_wca_notation(moves, d) << std::endl;
                break;
            }
        }
    }
    std::cout << "Done in " << (float)(std::clock() - t0)/CLOCKS_PER_SEC << std::endl;
    return 0;
}
