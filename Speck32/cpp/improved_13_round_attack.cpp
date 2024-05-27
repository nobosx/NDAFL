#include "speck.h"
#include "util.h"
#include <vector>
#include <string>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <assert.h>
#include <fstream>
#include <set>
#include <math.h>
using namespace std;

ofstream log_file;

struct attack_para_two_rounds {
    uint32_t n;
    uint32_t attack_nr;
    uint32_t n_iter1, n_iter2;
    uint32_t n_cand1, n_cand2;
    block diff1;
    block diff2;
    vector<neutral_bit> NBs;
    uint32_t switch_bit;
    uint32_t paired_bit;
    vector<linear_constraint> plaintext_conditions;
    uint32_t num_used_structures;
    uint32_t structure_repeat_num;
    uint32_t structure_size;
    double *lookup_table1, *lookup_table2;
    vector<uint32_t> selected_bits1, selected_bits2;
    double *mu1, *mu2;
    double *sigma1, *sigma2;
    double c1, c2;
    vector<word> diff_kg1;
    vector<word> diff_kg2;
};

struct kg_score {
    word kg;
    double score;
};

int partition(kg_score p[], int l, int r) {
    double key = p[l].score;
    while (l < r) {
        while (p[r].score >= key && l < r) {
            --r;
        }
        if (l < r) {
            swap(p[l], p[r]);
            ++l;
        }
        while (p[l].score <= key && l < r) {
            ++l;
        }
        if (l < r) {
            swap(p[l], p[r]);
            --r;
        }
    }
    return l;
}

void get_min_k(kg_score p[], int l, int r, int k) {
    if (l >= r) return;
    int pivot = partition(p, l, r);
    int len = pivot - l + 1;
    if (len < k) get_min_k(p, pivot + 1, r, k - len);
    else if (k < len) get_min_k(p, l, pivot - 1, k);
}

void bayesian_key_search_8r(const block c0[], const block c1[], const uint32_t& structure_size, const uint32_t& n_iter, const uint32_t& n_cand, kg_score kg_scores[], const double lookup_table[], const double mu[], const double sigma[]) {
    uint32_t kg_space = 1 << 14;
    kg_score kg_pr[1 << 14];
    kg_score *p = kg_scores, *q;
    block t0, t1;
    word tmp_kg;
    double score;
    word mask_val = 0x3fff;
    // Initialize key guess batch
    for (int i = 0; i < n_cand; i++) p[i].kg = RAND_WORD;
    // for (int i = 0; i < n_cand; i++) p[i].kg = i;
    for (int i = 0; i < n_cand; i++) {
        tmp_kg = p[i].kg;
        score = 0;
        for (int j = 0; j < structure_size; j++) {
            dec_one_round(c0[j], tmp_kg, t0);
            dec_one_round(c1[j], tmp_kg, t1);
            score += lookup_table[extract_bits_from_block_8r(t0, t1)];
        }
        p[i].score = score;
    }
    q = &(kg_scores[n_cand]);
    // Begin bayesian iteration
    for (int iter_index = 1; iter_index < n_iter; iter_index++) {
        // Recommand new batch of key guess
        for (uint32_t x = 0; x < kg_space; x++) {
            kg_pr[x].kg = x;
            score = 0;
            for (int i = 0; i < n_cand; i++) score += pow((p[i].score - mu[(p[i].kg ^ x) & mask_val]) / sigma[(p[i].kg ^ x) & mask_val], 2);
            kg_pr[x].score = score;
        }
        get_min_k(kg_pr, 0, kg_space - 1, n_cand);
        for (int i = 0; i < n_cand; i++) q[i].kg = kg_pr[i].kg | (word(RAND_BYTE & 0x3) << 14);
        p = q;
        q = q + n_cand;
        for (int i = 0; i < n_cand; i++) {
            tmp_kg = p[i].kg;
            score = 0;
            for (int j = 0; j < structure_size; j++) {
                dec_one_round(c0[j], tmp_kg, t0);
                dec_one_round(c1[j], tmp_kg, t1);
                score += lookup_table[extract_bits_from_block_8r(t0, t1)];
            }
            p[i].score = score;
        }
    }
}

void bayesian_key_search_7r(const block c0[], const block c1[], const uint32_t& structure_size, const uint32_t& n_iter, const uint32_t& n_cand, kg_score kg_scores[], const double lookup_table[], const double mu[], const double sigma[]) {
    uint32_t kg_space = 1 << 14;
    kg_score kg_pr[1 << 14];
    kg_score *p = kg_scores, *q;
    block t0, t1;
    word tmp_kg;
    double score;
    word mask_val = 0x3fff;
    for (int i = 0; i < n_cand; i++) p[i].kg = RAND_WORD & mask_val;
    for (int i = 0; i < n_cand; i++) {
        tmp_kg = p[i].kg;
        score = 0;
        for (int j = 0; j < structure_size; j++) {
            dec_one_round(c0[j], tmp_kg, t0);
            dec_one_round(c1[j], tmp_kg, t1);
            score += lookup_table[extract_bits_from_block_7r(t0, t1)];
        }
        p[i].score = score;
    }
    q = &(kg_scores[n_cand]);
    for (int iter_index = 1; iter_index < n_iter; iter_index++) {
        for (uint32_t x = 0; x < kg_space; x++) {
            kg_pr[x].kg = x;
            score = 0;
            for (int i = 0; i < n_cand; i++) score += pow((p[i].score - mu[p[i].kg ^ x]) / sigma[p[i].kg ^ x], 2);
            kg_pr[x].score = score;
        }
        get_min_k(kg_pr, 0, kg_space - 1, n_cand);
        for (int i = 0; i < n_cand; i++) q[i].kg = kg_pr[i].kg;
        p = q;
        q = q + n_cand;
        for (int i = 0; i < n_cand; i++) {
            tmp_kg = p[i].kg;
            score = 0;
            for (int j = 0; j < structure_size; j++) {
                dec_one_round(c0[j], tmp_kg, t0);
                dec_one_round(c1[j], tmp_kg, t1);
                score += lookup_table[extract_bits_from_block_7r(t0, t1)];
            }
            p[i].score = score;
        }
    }
}

void verify_search(const block c0[], const block c1[], const uint32_t& structure_size, const word& sk1, const word& sk2, const double& init_score, double lookup_table[], word& bk1, word& bk2, const vector<word>& diff_kg1, const vector<word>& diff_kg2) {
    word k1, k2, best_k1, best_k2;
    block t0, t1;
    block *a0 = new block[structure_size], *a1 = new block[structure_size];
    double score, best_score = 0;
    for (int i = 0; i < structure_size; i++) {
        dec_one_round(c0[i], sk1, a0[i]);
        dec_one_round(c1[i], sk1, a1[i]);
        dec_one_round(a0[i], sk2, t0);
        dec_one_round(a1[i], sk2, t1);
        best_score += lookup_table[extract_bits_from_block_7r(t0, t1)];
    }
    best_k1 = sk1, best_k2 = sk2;
    assert(abs(init_score - best_score) < 1e-5);
    do {
        bk1 = best_k1, bk2 = best_k2;
        for(auto d1 : diff_kg1) {
            k1 = bk1 ^ d1;
            for (int i = 0; i < structure_size; i++) {
                dec_one_round(c0[i], k1, a0[i]);
                dec_one_round(c1[i], k1, a1[i]);
            }
            for (auto d2 : diff_kg2) {
                k2 = bk2 ^ d2;
                score = 0;
                for (int i = 0; i < structure_size; i++) {
                    dec_one_round(a0[i], k2, t0);
                    dec_one_round(a1[i], k2, t1);
                    score += lookup_table[extract_bits_from_block_7r(t0, t1)];
                }
                if (score > best_score) {
                    best_score = score;
                    best_k1 = k1;
                    best_k2 = k2;
                }
            }
        }
    } while((best_k1 != bk1) || (best_k2 != bk2));
    delete[] a0, delete[] a1;
}

bool naive_guess_two_rounds(cipher_structure data[], const attack_para_two_rounds& para, word& bk1, word&bk2) {
    uint32_t total_kg_num1 = para.n_cand1 * para.n_iter1, total_kg_num2 = para.n_cand2 * para.n_iter2;
    kg_score *kg_scores1 = new kg_score[total_kg_num1], *kg_scores2 = new kg_score[total_kg_num2];
    word sk1, sk2;

    // Simply visit each ciphertext structure for structure_repeat_num times
    for (int repeat_index = 0; repeat_index < para.structure_repeat_num; repeat_index++) {
        for (int i = 0; i < para.num_used_structures; i++) {
            // Guess kg_12
            bayesian_key_search_8r(data[i].c0, data[i].c1, para.structure_size, para.n_iter1, para.n_cand1, kg_scores1, para.lookup_table1, para.mu1, para.sigma1);
            for (int j = 0; j < total_kg_num1; j++) {
                if (kg_scores1[j].score > para.c1) {
                    sk1 = kg_scores1[j].kg;
                    // Decrypt for one round
                    for (int z = 0; z < para.structure_size; z++) {
                        dec_one_round(data[i].c0[z], sk1, data[i].p0[z]);
                        dec_one_round(data[i].c1[z], sk1, data[i].p1[z]);
                    }
                    // Guess kg_11
                    bayesian_key_search_7r(data[i].p0, data[i].p1, para.structure_size, para.n_iter2, para.n_cand2, kg_scores2, para.lookup_table2, para.mu2, para.sigma2);
                    for (int z = 0; z < total_kg_num2; z++) {
                        if (kg_scores2[z].score > para.c2) {
                            sk2 = kg_scores2[z].kg;
                            verify_search(data[i].c0, data[i].c1, para.structure_size, sk1, sk2, kg_scores2[z].score, para.lookup_table2, bk1, bk2, para.diff_kg1, para.diff_kg2);
                            delete[] kg_scores1, delete[] kg_scores2;
                            return true;
                        }
                    }
                }
            }
        }
    }
    // No key geuss survives
    delete[] kg_scores1, delete[] kg_scores2;
    return false;
}

void attack_13r(const attack_para_two_rounds& para) {
    cipher_structure *data = new cipher_structure[para.num_used_structures];
    for (int i = 0; i < para.num_used_structures; i++) data[i].init(para.structure_size);
    word ks[50];
    word mk[M];
    word tk1, tk2, tmp_k0, tk_k0_bits, bk1, bk2;
    bool whether_surviving_kg;
    vector<linear_constraint> k0_conditions;
    for (int i = 0; i < para.plaintext_conditions.size(); i++) {
        k0_conditions.push_back({{0,0}, para.plaintext_conditions[i].num_bits, 0});
        for (int j = 0; j < para.plaintext_conditions[i].num_bits; j++) {
            if (para.plaintext_conditions[i].xor_bit_pos[j] < 16) k0_conditions[i].xor_bit_pos[j] = para.plaintext_conditions[i].xor_bit_pos[j];
            else k0_conditions[i].xor_bit_pos[j] = para.plaintext_conditions[i].xor_bit_pos[j] - 16;
        }
    }
    uint32_t num_conditions = para.plaintext_conditions.size();
    uint32_t k0_guess_space = 1 << num_conditions;

    clock_t start, end;
    time_t start_time, over_time;
    double total_running_time = 0;
    uint32_t attack_success_time = 0, kg_surviving_time = 0;

    time(&start_time);

    for (int attack_index = 0; attack_index < para.n; attack_index++) {
        printf("\n#Attack: %d.\n", attack_index);
        start = clock();
        whether_surviving_kg = false;
        // Generate weak key space
        do {
            for (int i = 0; i < M; i++) mk[i] = RAND_WORD;
            expand_key(mk, ks, para.attack_nr);
        } while (((ks[2] >> 12) & 1) == ((ks[2] >> 11) & 1));
        tk1 = ks[para.attack_nr - 1];
        tk2 = ks[para.attack_nr - 2];

        tk_k0_bits = 0;
        for (int i = 0; i < k0_conditions.size(); i++) {
            uint32_t tmp = 0;
            for (int j = 0; j < k0_conditions[i].num_bits; j++) tmp ^= (ks[0] >> k0_conditions[i].xor_bit_pos[j]) & 1;
            tk_k0_bits |= tmp << i;
        }

        // Guess k0
        for (uint32_t k0_guess = 0; k0_guess < k0_guess_space; k0_guess++) {
            for (int i = 0; i < num_conditions; i++) k0_conditions[i].xor_value = (k0_guess >> i) & 1;
            tmp_k0 = gen_rand_uint32_with_linear_constraint(k0_conditions) & 0xffff;
            printf("k0 guess is: %d\n", k0_guess);

            // Construct plaintext structures
            for (int i = 0, j = para.num_used_structures >> 1; i < (para.num_used_structures >> 1); i++, j++) gen_extended_plaintext_structure(data[i].p0, data[i].p1, data[j].p0, data[j].p1, para.diff1, para.diff2, para.NBs, para.switch_bit, para.paired_bit, para.plaintext_conditions);
            // Collect ciphertext structures
            for (int i = 0; i < para.num_used_structures; i++) collect_ciper_structure(data[i].p0, data[i].p1, tmp_k0, ks, para.structure_size, para.attack_nr, data[i].c0, data[i].c1);

            // Key guessing phase
            whether_surviving_kg = naive_guess_two_rounds(data, para, bk1, bk2);
            if (whether_surviving_kg) break;
        }
        end = clock();
        printf("Time consumption: %f s\n", (end - start + 0.0) / CLOCKS_PER_SEC);
        printf("Plaintext conspumption: 2^(%f).\n", log2(para.num_used_structures * para.structure_size * k0_guess_space));
        total_running_time += (end - start + 0.0) / CLOCKS_PER_SEC;
        if (whether_surviving_kg) {
            printf("There is a key guess surviving, the difference between the key guess and the ture key is: (0x%x, 0x%x).\n", tk1 ^ bk1, (tk2 ^ bk2) & 0x3fff);
            kg_surviving_time++;
            uint32_t kg_hd = cal_hw(tk1 ^ bk1, 16) + cal_hw(tk2 ^ bk2, 14);
            if (kg_hd <= 2) {
                attack_success_time++;
                printf("Attack success.\n");
            } else {
                printf("Attack failure.\n");
            }
        } else {
            printf("No key guess survived.\n");
            printf("Attack failure.\n");
        }
    }
    time(&over_time);
    printf("\nAll attacks is over:\n");
    printf("Program begins at: %s", ctime(&start_time));
    printf("Program ends at: %s", ctime(&over_time));
    printf("Average running time: %f.\n", total_running_time / para.n);
    printf("Attack sueccess time: %d.\n", attack_success_time);
    printf("Average plaintext consumption: 2^(%f).\n", log2(para.num_used_structures * para.structure_size * k0_guess_space));
    delete[] data;
}

void load_attack_parameter(attack_para_two_rounds& para, const uint32_t& n) {
    para.attack_nr = 13;
    para.n_iter1 = 5; para.n_cand1 = 32;
    para.n_iter2 = 5; para.n_cand2 = 32;
    para.diff1 = {0x8020, 0x4101};
    para.diff2 = {0x8060, 0x4101};
    para.NBs = {{1,{20}}, {1,{13}}, {2,{12,19}}, {2,{14,21}}, {2,{6,29}}, {1,{30}},{3,{0,8,31}},{2,{5,28}},{2,{15,24}},{3,{4,27,29}}};
    para.switch_bit = 21;
    para.paired_bit = 22;
    para.plaintext_conditions = {{{28,5}, 2, 1}, {{1}, 1, 0}, {{27,4}, 2, 1}};
    para.n = n;
    para.num_used_structures = 1 << 14;
    para.structure_size = 1 << 12;
    para.structure_repeat_num = 4;
    para.c1 = 17;
    para.c2 = -300;

    // Load 8-round lookup table
    string table_path("attack_lookup_table/8r_table_22_18_8R");
    ifstream fin(table_path, ios::in | ios::binary);
    uint64_t input_space = pow(2, para.selected_bits1.size());
    uint64_t *lookup_table = new uint64_t[input_space];
    fin.read((char *)lookup_table, input_space * sizeof(uint64_t));
    fin.close();
    para.lookup_table1 = new double[input_space];
    trans_lookup_table_output(lookup_table, para.lookup_table1, para.selected_bits1);
    delete[] lookup_table;

    // Load 7-round lookup table
    table_path = "attack_lookup_table/7r_table_24_10_7R";
    fin.open(table_path, ios::in | ios::binary);
    input_space = pow(2, para.selected_bits2.size());
    lookup_table = new uint64_t[input_space];
    fin.read((char *)lookup_table, input_space * sizeof(uint64_t));
    fin.close();
    para.lookup_table2 = new double[input_space];
    trans_lookup_table_output(lookup_table, para.lookup_table2, para.selected_bits2);
    delete[] lookup_table;


    // Load 8r wrong key profile
    uint32_t kg_space = 1 << 16, structure_size = 1 << 12;
    para.mu1 = new double[kg_space];
    para.sigma1 = new double [kg_space];
    string mu_path("wrong_key_profile/8r_mean"), sigma_path("wrong_key_profile/8r_std_var_12");
    fin.open(mu_path, ios::in | ios::binary);
    fin.read((char *)para.mu1, kg_space * sizeof(double));
    fin.close();
    fin.open(sigma_path, ios::in | ios::binary);
    fin.read((char *)para.sigma1, kg_space * sizeof(double));
    fin.close();
    for (uint32_t i = 0; i < kg_space; i++) {
        para.mu1[i] *= structure_size;
    }

    // Load 7r wrong key profile
    kg_space = 1 << 14;
    para.mu2 = new double[kg_space];
    para.sigma2 = new double[kg_space];
    mu_path = "wrong_key_profile/7r_mean", sigma_path = "wrong_key_profile/7r_std_var_12";
    fin.open(mu_path, ios::in | ios::binary);
    fin.read((char *)para.mu2, kg_space * sizeof(double));
    fin.close();
    fin.open(sigma_path, ios::in | ios::binary);
    fin.read((char *)para.sigma2, kg_space * sizeof(double));
    fin.close();
    for (uint32_t i = 0; i < kg_space; i++) {
        para.mu2[i] *= structure_size;
    }

    // Used in the verify search
    para.diff_kg1.clear();
    para.diff_kg2.clear();
    for (int i = 0; i < (1 << 16); i++)
        if (cal_hw(i, 16) <= 2) para.diff_kg1.push_back(i);
    for (int i = 0; i < (1 << 14); i++)
        if (cal_hw(i, 14) <= 2) para.diff_kg2.push_back(i);
}

int main() {
    printf("Test check vector for Speck32/64...\n");
    check_testvector();
    // Set random seed
    random_generator.set_rand_seed(time(NULL));
    attack_para_two_rounds para;
    // Set total attack number
    uint32_t attack_num = 10;
    printf("Load attack parameters...\n");
    load_attack_parameter(para, attack_num);
    
    // Execute 13-round attack on Speck32/64
    attack_13r(para);

    delete[] para.lookup_table1, delete[] para.lookup_table2, delete[] para.mu1, delete[] para.sigma1, delete[] para.mu2, delete[] para.sigma2;
}