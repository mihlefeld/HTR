#include <stdint.h>
#include <iostream>
#include <vector>
#include <bitset>
#include <string>
#include <ctime>
#include <omp.h>
#include <cmath>
#include <getopt.h>
#include "htr_cube.h"
#include "prevent_moves_3d.h"

std::vector<uint32_t> htr_states_non_unique;
uint32_t htr_states[96];

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


uint8_t parse_eo(const char* eo_str) {
    uint8_t eo = 0;
    for (uint8_t i = 0; i < 8; i++) {
        eo |= eo_str[i] - '0';
        if (i < 7) {
            eo <<= 1;
        }
    }
    return eo;
}

uint32_t parse_cp(const char* cp_str) {
    uint32_t cp = 0;
    for (uint8_t i = 0; i < 8; i++) {
        cp |= cp_str[i] - '0';
        cp <<= 3;
    }
    cp >>= 3;
    return cp;
}

Cube parse_cube(const char* eo_str, const char* cp_str) {
    uint8_t eo = parse_eo(eo_str);
    uint32_t cp = parse_cp(cp_str);
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

bool search_no_sol(Cube cb, uint8_t d, uint8_t sd, uint8_t lm0, uint8_t lm1) {
    cb = move(cb, lm0);
    if (d == 1) {
        Cube cb2 = cb;
        if (prevent_moves[lm1][lm0][1]) {
            cb = move(cb,1);
            if (is_htr(cb)) {
                return true;
            }
        }
        if(prevent_moves[lm1][lm0][3]) {
            cb = move(cb2,3);
            if (is_htr(cb)) {
                return true;
            }
        }
        return false;
    }
    for (int m = 1; m <= 10; m++) {
        if (prevent_moves[lm1][lm0][m]) {
            bool b = search_no_sol(cb, d - 1, sd, m, lm0);
            if (b){
                return true;
            }
        }
    }
    return false;
}

bool call_search_no_sol(Cube cb, uint8_t d) {
    return search_no_sol(cb, d, d, 0, 0);
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
            default:
                break;
        }
    }
    return wca_moves;
}

void co_to_cp_list(uint8_t co, uint32_t* cp_list) {
    uint8_t orbits[2][4] = {{0,2,4,6},
                           {1,3,5,7}};
    uint8_t orig_indices[8];
    uint8_t oi[2] = {0, 0};
    // canonic CO -> CP mapping
    uint32_t cp = 0;
    for(int i = 0; i < 8; i++) {
        uint8_t par = (co >> (7-i) & 1) ^ (i & 1);
        cp += orbits[par][oi[par]];
        orig_indices[cp & 0b111] = 7 - i;
        oi[par] += 1;
        cp <<= 3;
    }
    cp >>= 3;
    cp_list[0] = cp;
    uint32_t ia, ib, ic;
    uint32_t tcp;

    // swao diag UD
    ia = orig_indices[1];
    ib = orig_indices[3];
    cp_list[1] = swap_bits_32bit(cp,ia*3,ib*3,7);

    // swap diag LR
    ia = orig_indices[2];
    ib = orig_indices[4];
    cp_list[2] = swap_bits_32bit(cp,ia*3,ib*3,7);

    // swap diag FB
    ia = orig_indices[0];
    ib = orig_indices[4];
    cp_list[3] = swap_bits_32bit(cp,ia*3,ib*3,7);

    // cycle CAW
    ia = orig_indices[3];
    ib = orig_indices[1];
    ic = orig_indices[7];
    tcp = swap_bits_32bit(cp, ia*3, ib*3, 7);
    cp_list[4] = swap_bits_32bit(tcp,ia*3,ic*3,7);

    // cycle CWA
    tcp = swap_bits_32bit(cp, ia*3, ic*3, 7);
    cp_list[5] = swap_bits_32bit(tcp,ia*3,ib*3,7);
}

void gen_domino_states(std::vector<Cube>& states, int sample_size) {
    std::vector<uint8_t > eos, co_s;
    for(uint32_t eo = 0; eo < 256; eo++) {
        uint8_t fhbc = ((eo >> 7) & 1) + ((eo >> 6) & 1) + ((eo >> 5) & 1) + ((eo >> 4) & 1);
        uint8_t shbc = ((eo >> 3) & 1) + ((eo >> 2) & 1) + ((eo >> 1) & 1) + ((eo >> 0) & 1);
        uint8_t co_fhbc = ((eo >> 0) & 1) + ((eo >> 2) & 1) + ((eo >> 4) & 1) + ((eo >> 6) & 1);
        uint8_t co_shbc = ((eo >> 1) & 1) + ((eo >> 3) & 1) + ((eo >> 5) & 1) + ((eo >> 7) & 1);
        if(fhbc == shbc) {
            eos.push_back(eo);
        }
        if(co_fhbc == co_shbc) {
            co_s.push_back(eo);
        }
    }
    std::vector<uint32_t> cps;
    uint32_t temp_cps[6];
    for(auto co: co_s) {
        co_to_cp_list(co, temp_cps);
        for(auto cp: temp_cps) {
            cps.push_back(cp);
        }
    }

    for(auto cp: cps) {
        for(auto eo: eos) {
            states.push_back({eo, cp});
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

void calc_distribution(uint8_t depth, uint32_t sample_size, int threads) {
    std::vector<Cube> states;
    gen_domino_states(states, sample_size);
    if(sample_size != 0 && sample_size < states.size()) {
        while (states.size() > sample_size) {
            int index = (int) ((float)rand()*states.size()/RAND_MAX);
            states.erase(states.begin()+index);
        }
    }
    double t0 = omp_get_wtime();
    uint8_t num_threads = threads;
    uint32_t counts[depth + 1];
    uint32_t bounds[num_threads+1];
    for (int i = 0; i <= depth; i++) {
        counts[i] = 0;
    }
    bounds[0] = 0;
    for (int i = 1; i < num_threads; i++) {
        bounds[i] = (int) ((float)states.size()*i/num_threads);
    }
    bounds[num_threads] = states.size();

    #pragma omp parallel default(none) shared(states,depth,bounds,num_threads,counts,std::cout,t0) num_threads(num_threads)
    {
        uint8_t priv_counts[num_threads];
        for (int i = 0; i <= depth; i++) {
            priv_counts[i] = 0;
        }
        #pragma omp for
        for (int i = 0; i < num_threads; i++) {
            const double chunk_size = (bounds[i + 1] - bounds[i])/100.0f;
            uint8_t percent_progress = 0;
            for (int j = bounds[i]; j < bounds[i + 1]; j++) {
                if (is_htr(states[j])) {
                    priv_counts[0] += 1;
                    continue;
                }
                for (int d = 1; d <= depth; d++) {
                    if (call_search_no_sol(states[j], d)) {
                        priv_counts[d] += 1;
                        //std::cout << "Cube #" << j << ": " << parse_cube_inv(states[j]) << " sol len: " << d << std::endl;
                        break;
                    }
                }
                percent_progress++;
                if(percent_progress > chunk_size) {
                    #pragma omp critical
                    {
                        std::cout << "Proc " << i << ": " << (double) ((j+1) - bounds[i]) / chunk_size << "% @" << omp_get_wtime() - t0 << std::endl;
                    }
                    percent_progress = 0;
                }
            }
        }
        #pragma omp critical
        {
            for (int i = 0; i <= depth; i++) {
                counts[i] += priv_counts[i];
            }
        }
    }
    for (int i = 0; i <= depth; i++) {
        std::cout << "len " << i << ": " << counts[i] << std::endl;
    }
}

void calc_solution(bool single, Cube cube, int depth) {
    // standard test: 00100010 02173654
    std::cout << "Solving HTR for cube: " << parse_cube_inv(cube) << std::endl;
    for(int d = 1; d <= depth; d++) {
        std::cout << "Searching depth " << d << std::endl;
        uint8_t moves[depth];
        if(call_search(cube, d, moves)) {
            std::cout << "Found HTR in " << d << " moves" << std::endl;
            std::cout << convert_to_wca_notation(moves, d) << std::endl;
            break;
        }
    }
}

void calc_hus(uint8_t eo, uint8_t co, uint8_t depth) {
    std::bitset<8> edges(eo);
    std::bitset<8> corners(co);
    std::cout << "Solving HUS HTR for cube: " << edges << " " << corners << std::endl;
    std::vector<std::string> solutions;
    uint32_t cps[6];
    co_to_cp_list(co, cps);
    double t0 = omp_get_wtime();
    for(auto cp: cps) {
        Cube cube = {eo, cp};
        std::cout << "Cube: " << parse_cube_inv(cube) << std::endl;
        for(int d = 1; d <= depth; d++) {
            uint8_t moves[depth];
            if(call_search(cube, d, moves)) {
                std::cout << "    " << d << " moves @" << (double) omp_get_wtime() - t0 << "s" << std::endl;
                solutions.push_back(convert_to_wca_notation(moves, d) + "(" + std::to_string(d) + ")");
                break;
            }
        }
    }

    for(auto str: solutions) {
        std::cout << str << std::endl;
    }
}

void error_message() {
    std::cout << "Valid options:" << std::endl;
    std::cout << "  -h <EO_CO> :" << std::endl;
    std::cout << "     Use the Human Solver with the given EO and CO, seperated by any non whitespace character." << std::endl;
    std::cout << "  -s <EO_CP> :" << std::endl;
    std::cout << "     Solve HTR for the given EO and CP, seperated by any non whitespace character." << std::endl;
    std::cout << "  -r <seed> :" << std::endl;
    std::cout << "    Initialize a seed for distribution calculation." << std::endl;
    std::cout << "  -d :" << std::endl;
    std::cout << "    Calculate the solution length distribution for all possible DR states." << std::endl;
    std::cout << "  -n :" << std::endl;
    std::cout << "    Specify the sample size for distribution calculation." << std::endl;
    std::cout << "  -t :" << std::endl;
    std::cout << "    Specify the number of threads for distribution calculation." << std::endl;
    exit(-1);
}

int main(int argc, char** argv) {
    init_htr_states();

    double t0;
    int depth = 14;
    int sample_size = 0;
    int seed = 42;
    int threads = 12;
    uint8_t co;
    uint8_t eo;
    Cube cube;
    bool dist = false;
    bool sol = false;
    bool hus = false;

    if(argc < 2) {
        error_message();
    }

    int c = 0;
    while ((c = getopt(argc, argv, "dn:s:h:r:t:")) != -1) {
        switch (c) {
            case 'n': {
                sample_size = std::stoi(optarg);
                break;
            }
            case 'd': {
                dist = true;
                break;
            }
            case 's': {
                std::string arg(optarg);
                std::string eo = arg.substr(0, 8);
                std::string cp = arg.substr(9, 8);
                cube = parse_cube(eo.c_str(), cp.c_str());
                sol = true;
                break;
            }
            case 'h': {
                std::string arg(optarg);
                eo = parse_eo(arg.substr(0, 8).c_str());
                co = parse_eo(arg.substr(9, 8).c_str());
                hus = true;
                break;
            }
            case 'r': {
                seed = std::stoi(optarg);
                srand(seed);
                break;
            }
            case 't': {
                threads = std::stoi(optarg);
                break;
            }
            case '?': {
                error_message();
                break;
            }
            default:
                break;
        }
    }


    t0 = omp_get_wtime();
    if(dist) {
        calc_distribution(depth, sample_size, threads);
    }
    if(sol) {
        calc_solution(false, cube, depth);
    }
    if(hus) {
        calc_hus(eo, co, depth);
    }
    std::cout << "Done in " << omp_get_wtime() - t0 << std::endl;
    return 0;
}
