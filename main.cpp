#include <stdint.h>
#include <iostream>
#include <vector>
#include <bitset>
#include <string>
#include <ctime>
#include <omp.h>
#include <cmath>
#include <fstream>
#include <algorithm>
#include "htr_cube.h"
#include "prevent_moves_3d.h"

#define MAX_DEPTH 13

std::vector<uint32_t> htr_states_non_unique;
uint32_t htr_states[96];
// if (lm == 0 || prevent_moves[lm][m])
//                             0 1 2 3 4 5 6 7 8 9 10
/*bool prevent_moves[11][11] = {{1,1,1,1,1,1,1,1,1,1,1}, // 0
                              {0,0,0,1,1,0,1,1,1,1,1}, // 1
                              {0,0,0,1,1,0,1,1,1,1,1}, // 2
                              {0,1,1,0,0,1,0,1,1,1,1}, // 3
                              {0,1,1,0,0,1,0,1,1,1,1}, // 4
                              {0,0,0,1,1,0,1,1,1,1,1}, // 5
                              {0,1,1,0,0,1,0,1,1,1,1}, // 6
                              {0,1,1,1,1,1,1,0,1,1,1}, // 7
                              {0,1,1,1,1,1,1,1,0,1,1}, // 8
                              {0,1,1,1,1,1,1,1,1,0,1}, // 9
                              {0,1,1,1,1,1,1,1,1,1,0}}; // 10*/

/*
 * 8 cases
 * 1111 +
 * 1100 +
 * 1010 +
 * 1001 +
 * 0110 +
 * 0101 +
 * 0011 +
 * 0000
 */
uint32_t lookup_parity[8] = {
        0b00000000100100100100000000000000,
        0b00000000100100000000000000000000,
        0b00000000100000100000000000000000,
        0b00000000100000000100000000000000,
        0b00000000000100100000000000000000,
        0b00000000000100000100000000000000,
        0b00000000000000100100000000000000,
        0b00000000000000000000000000000000};


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
    //if(cb.c != 0b00000000000001010011100101110111) return false;
    if(cb.e != 0) return false;
    if((cb.c & 0b00000000001001001001001001001001) != 0b00000000000001000001000001000001) return false;
    uint8_t ct = cb.c & 0b00000000100100100100000000000000;
    uint8_t is_htr = 0;
    uint8_t is_cpar = 0;

#pragma omp simd
    for (uint8_t i = 0; i < 8; i++) {
        is_cpar &= ct != lookup_parity[i];
    }
    if(is_cpar)
        return false;

#pragma omp simd
    for (uint8_t i = 0; i < 96; i++) {
        is_htr |= cb.c == htr_states[i];
    }
    return is_htr;
}

// lm0 is the last move lm1 is the move before that
bool search(Cube cb, uint8_t d, uint8_t sd, uint8_t lm0, uint8_t lm1, uint8_t* moves) {
    cb = move(cb, lm0);
    if (d == 1) {
        Cube cb2 = cb;
        if (prevent_moves[lm1][lm0][1]) {
            cb = move(cb,1);
            if (is_htr(cb)) {
                moves[sd-d] = 1;
                return true;
            }
        }
        if(prevent_moves[lm1][lm0][3]) {
            cb = move(cb2,3);
            if (is_htr(cb)) {
                moves[sd-d] = 3;
                return true;
            }
        }
        return false;
    }
    for (int m = 1; m <= 10; m++) {
        if (prevent_moves[lm1][lm0][m]) {
            bool b = search(cb, d - 1, sd, m, lm0, moves);
            if (b){
                moves[sd-d] = m;
                return true;
            }
        }
    }
    return false;
}

bool call_search(Cube cb, uint8_t d, uint8_t* moves) {
    return search(cb, d, d, 0, 0, moves);
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

void gen_domino_states(std::vector<Cube>& states) {
    std::string cp = "01234567";
    uint8_t eos[128];
    int non_parity_count = 0;
    for(uint8_t eo; eo < 255; eo++) {
        uint8_t parity = eo;
        for(uint8_t b = 1; b < 4; b = b << 1) {
            parity = parity ^ (parity >> b);
        }
        if(!(parity & 1)) {
            eos[non_parity_count] = eo;
            non_parity_count++;
        }
    }

    std::vector<uint32_t> cps;
    do {
        Cube cb = parse_cube("00000000", cp.c_str());
        cps.push_back(cb.c);
    } while (std::next_permutation(cp.begin(), cp.end()));

    for(uint8_t e: eos) {
        for(uint32_t c: cps) {
            states.push_back({e,c});
        }
    }
}

uint32_t transform(uint8_t c, uint8_t t) {
    switch (t){
        case 1:
            c = swap_bits_32bit(c,1*3,3*3,7);
            return c;
        case 2:
            c = swap_bits_32bit(c,3*3,1*3,7);
            c = swap_bits_32bit(c,3*3,7*3,7);
            return c;
        case 3:
            c = swap_bits_32bit(c,3*3,7*3,7);
            c = swap_bits_32bit(c,3*3,1*3,7);
            return c;
        case 4:
            c = swap_bits_32bit(c,2*3,0*3,7);
            c = swap_bits_32bit(c,2*3,4*3,7);
            return c;
        case 5:
            c = swap_bits_32bit(c,2*3,4*3,7);
            c = swap_bits_32bit(c,2*3,0*3,7);
            return c;
        default:
            return c;
    }
}

int main(int argc, char** argv) {

    clock_t t0 = std::clock();
    double wt0;
    init_htr_states();
    int depth = 4;
    Cube cb;
    bool findall = false;
    bool normal = true;
    if(argc >= 4) {
        depth = std::stoi(argv[3]);
        cb = parse_cube(argv[1], argv[2]);
        Cube db0 = move(cb, 9);
        db0 = move(db0, 4);
        db0 = move(db0, 8);
        db0 = move(db0, 1);
        db0 = move(db0, 7);
        db0 = move(db0, 2);
        std::cout << "Cube " << parse_cube_inv(db0) << " in htr: " << (bool) is_htr(db0) << std::endl;
        // F2 R' D2 L U2 L'
        if(argc == 5) findall = true;
    }
    else if (argc == 2) {
        depth = std::stoi(argv[1]);
        normal = false;
    }
    else {
        normal = false;
    }

    if(!normal) {
        std::cout << "Generating all HTR cases" << std::endl;
        std::vector<Cube> states;
        gen_domino_states(states);
        std::clock_t t1 = std::clock();
        uint8_t num_threads = 12;
        std::vector<uint8_t*> solutions;
        solutions.resize(states.size());

#pragma omp parallel for num_threads(12)
        for(uint8_t i = 0; i < num_threads; i++) {
            uint32_t chunk_size = states.size() / num_threads;
            uint32_t start_index = i * chunk_size;
            uint32_t end_index = (i == num_threads - 1) ? states.size() - 1 : start_index + chunk_size;
            uint32_t progress_chunks = chunk_size/10000;
            uint32_t percent = 0;
            uint32_t prog = 0;
            wt0 = omp_get_wtime();

            for(uint32_t j = start_index; j <= end_index; j++) {
                prog++;
                if(i == num_threads - 1 && prog >= progress_chunks) {
                    prog = 0;
                    std::cout << (float) percent/100 << "% @" << (float)(omp_get_wtime() - wt0) << "s" << std::endl;;
                    percent++;
                }
                solutions[j] = new uint8_t[depth];
                if(is_htr(states[j])) {
                    solutions[j][0] = 255;
                    continue;
                }
                for(int d = 1; d <= depth; d++) {
                    if(!call_search(states[j], d, solutions[j])) {
                        solutions[j][0] = 255;
                    } else {
                        break;
                    }
                }
            }
        }

        std::fstream file("cases.txt", std::fstream::out);
        for(uint32_t i = 0; i < states.size(); i++) {
            if(solutions[i][0] != 255) {
                file << "Case: " << parse_cube_inv(states[i]) << " Solution:" << convert_to_wca_notation(solutions[i], depth) << std::endl;
            }
            free(solutions[i]);
        }
        std::cout << "Done!" << std::endl;
        file.close();
    }
    else {
        // standard test: 00100010 02173654
        t0 = std::clock();
        for(int d = 1; d <= depth; d++) {
            std::cout << "Searching depth " << d << std::endl;
            uint8_t moves[MAX_DEPTH];
            if(call_search(cb, d, moves)) {
                std::cout << "Found HTR in " << d << " moves" << std::endl;
                std::cout << convert_to_wca_notation(moves, d) << std::endl;
                if(!findall) break;
            }
        }
        std::cout << "Done in " << (float)(std::clock() - t0)/CLOCKS_PER_SEC << std::endl;
    }
    return 0;
}
