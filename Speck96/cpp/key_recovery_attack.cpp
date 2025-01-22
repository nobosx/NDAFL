#include "util.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <time.h>
#include <vector>
#include <tuple>
#include <numeric>
#include <algorithm>
#include <thread>
#include <chrono>
#include <string>
using namespace std;

bool debug_filter_valid_structure = false;

// Set the number of threads
#define ATTACK_THREAD_NUM 10

uint32_t attack_nr;
uint32_t pre_nr;
block in_diff;
block dis_diff;
word rk1_mask;
word rk2_mask;
uint32_t structure_size;
uint32_t max_structure_consumption;
vector<uint32_t> NBs;
double th1;
double th2;
double th3;
double th4;
uint32_t kg1_high_bits1;
uint32_t kg1_low_bits1;
uint32_t kg1_high_offset_bits1;
uint32_t kg1_high_bits2;
uint32_t kg1_low_bits2;
uint32_t kg1_high_offset_bits2;
uint32_t kg1_low_offset_bits2;
uint32_t kg1_bits3;
uint32_t kg1_high_offset_bits3;
uint32_t kg2_bits3;
uint32_t kg2_high_offset_bits3;
uint32_t kg1_bits4;
uint32_t kg2_bits4;
uint32_t kg1_high_offset_bits4;
uint32_t top_k1;
uint32_t top_k2;
uint32_t top_k3;
InputExtracter extracter1;
InputExtracter extracter2;
InputExtracter extracter3;
InputExtracter extracter4;
double *response_table1;
double *response_table2;
double *response_table3;
double *response_table4;
uint32_t max_hw_metric;

void test_distinguishing_acc(const uint32_t& n, const uint32_t& num_rounds, const block& diff, const double response_table[], const InputExtracter& extracter) {
    block *c0 = new block[n], *c1 = new block[n];
    bool *Y = new bool[n];
    make_test_set(n, num_rounds, diff, c0, c1, Y);
    uint32_t num_p = 0, num_n = 0, num_true_p = 0, num_true_n = 0;
    bool prediction;
    for (uint32_t i = 0; i < n; i++) {
        if (response_table[extracter.extract_distinguisher_input(c0[i], c1[i])] < 0.0) prediction = false;
        else prediction = true;
        if (Y[i]) {
            num_p++;
            if (prediction) num_true_p++;
        } else {
            num_n++;
            if (!prediction) num_true_n++;
        }
    }
    double acc = (num_true_p + num_true_n + 0.0) / n, tpr = (num_true_p + 0.0) / num_p, tnr = (num_true_n + 0.0) / num_n;
    printf("Acc = %f, tpr = %f, tnr = %f\n", acc, tpr, tnr);
    delete[] c0; delete[] c1, delete[] Y;
}

static bool compare_one_kg(const tuple<word, double>& v1, const tuple<word, double>& v2) {
    return get<1>(v1) > get<1>(v2);
}

static bool compare_two_kgs(const tuple<word, word, double>& v1, const tuple<word, word, double>& v2) {
    return get<2>(v1) > get<2>(v2);
}

void dec_one_round_with_guessing_borrow_bit(const block& c, const word& key_guess, block& p, const uint32_t& borrow_bit_pos) {
    word mask_val = (1ull << borrow_bit_pos) - 1;
    word gamma = c.first ^ key_guess;
    word beta = ror(c.first ^ c.second, BETA);
    word borrow_bit = (beta >> (borrow_bit_pos - 1)) & 1;
    word alpha_high = (gamma >> borrow_bit_pos) - (beta >> borrow_bit_pos) - borrow_bit;
    word alpha_low = (gamma - beta) & mask_val;
    p.first = rol((alpha_high << borrow_bit_pos) | alpha_low, ALPHA);
    p.second = beta;
}

// Stage1: recover k_{-1}[12~11,4~0]
void attack_stage_1(const block c0[], const block c1[], vector<tuple<word, double>>& surviving_kg1_1_scores) {
    word kg_high_space = 1ull << kg1_high_bits1;
    word kg_low_space = 1ull << kg1_low_bits1;
    word kg1;
    double score;
    block t0, t1;
    for (word kg1_low = 0; kg1_low < kg_low_space; kg1_low++) {
        for (word kg1_high = 0; kg1_high < kg_high_space; kg1_high++) {
            kg1 = (kg1_high << kg1_high_offset_bits1) | kg1_low;
            score = 0;
            for (uint32_t i = 0; i < structure_size; i++) {
                dec_one_round_with_guessing_borrow_bit(c0[i], kg1, t0, kg1_high_offset_bits1);
                dec_one_round_with_guessing_borrow_bit(c1[i], kg1, t1, kg1_high_offset_bits1);
                score += response_table1[extracter1.extract_distinguisher_input(t0, t1)];
            }
            score /= structure_size;
            if (score > th1) {
                surviving_kg1_1_scores.push_back(tuple<word, double>(kg1, score));
            }
        }
    }
}

// Stage2: recover k_{-1}[20~13,10~5]
void attack_stage_2(const block c0[], const block c1[], const vector<word>& surviving_kg1_1, vector<tuple<word, double>>& surviving_kg1_2_scores) {
    word kg_high_space = 1ull << kg1_high_bits2;
    word kg_low_space = 1ull << kg1_low_bits2;
    word kg1;
    double score;
    block t0, t1;
    for (word kg1_1 : surviving_kg1_1) {
        for (word kg1_low = 0; kg1_low < kg_low_space; kg1_low++) {
            for (word kg1_high = 0; kg1_high < kg_high_space; kg1_high++) {
                kg1 = (kg1_high << kg1_high_offset_bits2) | (kg1_low << kg1_low_offset_bits2) | kg1_1;
                score = 0;
                for (uint32_t i = 0; i < structure_size; i++) {
                    dec_one_round(c0[i], kg1, t0);
                    dec_one_round(c1[i], kg1, t1);
                    score += response_table2[extracter2.extract_distinguisher_input(t0, t1)];
                }
                score /= structure_size;
                if (score > th2) {
                    surviving_kg1_2_scores.push_back(tuple<word, double>(kg1, score));
                }
            }
        }
    }  
}

// Stage3: recover k_{-1}[23~21]||k_{-2}[20~8]
void attack_stage_3(const block c0[], const block c1[], const vector<word>& surviving_kg1_2, vector<tuple<word, word, double>>& surviving_kgs_3_scores) {
    word kg1_space = 1ull << kg1_bits3, kg2_space = 1ull << kg2_bits3;
    double score;
    block t0, t1;
    word kg1, kg2;
    for (word kg1_2 : surviving_kg1_2) {
        for (word kg1_high = 0; kg1_high < kg1_space; kg1_high++) {
            kg1 = (kg1_high << kg1_high_offset_bits3) | kg1_2;
            for (word kg2_high = 0; kg2_high < kg2_space; kg2_high++) {
                kg2 = kg2_high << kg2_high_offset_bits3;
                score = 0;
                for (uint32_t i = 0; i < structure_size; i++) {
                    dec_one_round(c0[i], kg1, t0);
                    dec_one_round(c1[i], kg1, t1);
                    dec_one_round_with_guessing_borrow_bit(t0, kg2, t0, kg2_high_offset_bits3);
                    dec_one_round_with_guessing_borrow_bit(t1, kg2, t1, kg2_high_offset_bits3);
                    score += response_table3[extracter3.extract_distinguisher_input(t0, t1)];
                }
                score /= structure_size;
                if (score > th3) {
                    surviving_kgs_3_scores.push_back(tuple<word,word,double>(kg1, kg2, score));
                }
            }
        }
    }
}

// Stage4: recover k_{-1}[47~40]||k_{-2}[7~0]
void attack_stage_4(const block c0[], const block c1[], const vector<tuple<word,word>>& surviving_kgs_3, vector<tuple<word, word, double>>& surviving_kgs_and_scores) {
    word kg1_space = 1ull << kg1_bits4, kg2_space = 1ull << kg2_bits4;
    double score;
    block t0, t1;
    word kg1, kg2;

    for (auto x : surviving_kgs_3) {
        for (word kg1_high = 0; kg1_high < kg1_space; kg1_high++) {
            kg1 = (kg1_high << kg1_high_offset_bits4) | get<0>(x);
            for (word kg2_low = 0; kg2_low < kg2_space; kg2_low++) {
                kg2 = get<1>(x) | kg2_low;
                score = 0;
                for (uint32_t i = 0; i < structure_size; i++) {
                    dec_one_round_with_guessing_borrow_bit(c0[i], kg1, t0, kg1_high_offset_bits4);
                    dec_one_round_with_guessing_borrow_bit(c1[i], kg1, t1, kg1_high_offset_bits4);
                    dec_one_round(t0, kg2, t0);
                    dec_one_round(t1, kg2, t1);
                    score += response_table4[extracter4.extract_distinguisher_input(t0, t1)];
                }
                score /= structure_size;
                if (score > th4)
                    surviving_kgs_and_scores.push_back(tuple<word,word,double>(kg1, kg2, score));
            }
        }
    }
}

class ThreadTask {
private:
    void attack_with_one_structure(const block c0[], const block c1[], vector<tuple<word, word, double>>& surviving_kgs_and_scores) {
        // Stage 1
        // printf("[Debug] Begin stage1.\n");
        vector<tuple<word, double>> surviving_kg1_1_scores;
        vector<word> surviving_kg1_1;
        attack_stage_1(c0, c1, surviving_kg1_1_scores);
        if (surviving_kg1_1_scores.empty()) {
            return;
        }

        // Leave best top_k1 key guesses for next stage
        sort(surviving_kg1_1_scores.begin(), surviving_kg1_1_scores.end(), compare_one_kg);
        for (int i = 0; (i < surviving_kg1_1_scores.size()) && (i < top_k1); i++) {
            surviving_kg1_1.push_back(get<0>(surviving_kg1_1_scores[i]));
        }

        // Stage 2
        fprintf(thread_output_file, "[Debug] Begin stage2.\n");
        vector<tuple<word, double>> surviving_kg1_2_scores;
        vector<word> surviving_kg1_2;
        attack_stage_2(c0, c1, surviving_kg1_1, surviving_kg1_2_scores);
        if (surviving_kg1_2_scores.empty()) {
            return;
        }

        // Leave best top_k2 key guesses for next stage
        sort(surviving_kg1_2_scores.begin(), surviving_kg1_2_scores.end(), compare_one_kg);
        for (int i = 0; (i < surviving_kg1_2_scores.size()) && (i < top_k2); i++) {
            surviving_kg1_2.push_back(get<0>(surviving_kg1_2_scores[i]));
        }

        // Stage 3
        fprintf(thread_output_file, "[Debug] Begin stage3.\n");
        vector<tuple<word, word, double>> surviving_kgs_3_scores;
        vector<tuple<word, word>> surviving_kgs_3;
        attack_stage_3(c0, c1, surviving_kg1_2, surviving_kgs_3_scores);
        if (surviving_kgs_3_scores.empty()) {
            return;
        }

        // Leave best top_k3 key guesses for next stage
        sort(surviving_kgs_3_scores.begin(), surviving_kgs_3_scores.end(), compare_two_kgs);
        for (int i = 0; (i < surviving_kgs_3_scores.size()) && (i < top_k3); i++) {
            surviving_kgs_3.push_back(tuple<word,word>(get<0>(surviving_kgs_3_scores[i]), get<1>(surviving_kgs_3_scores[i])));
        }

        // Stage 4
        fprintf(thread_output_file, "[Debug] Begin stage4.\n");
        attack_stage_4(c0, c1, surviving_kgs_3, surviving_kgs_and_scores);
    }
public:
    uint32_t attack_num;
    RandomGenerator *thread_random_generator = nullptr;
    FILE *thread_output_file;
    uint64_t structure_consumption_sum = 0;
    uint64_t attack_time_ms_sum = 0;
    uint64_t key_surviving_time = 0;
    uint64_t attack_success_time = 0;
    // Attack using one CPU thread
    void attack_4stages() {
        auto start_time = chrono::system_clock::now(), end_time = chrono::system_clock::now();
        word mk[M], rk[MAX_NR];
        word tk1, tk2;
        uint32_t num_used_structures;
        block *p0 = new block[structure_size], *p1 = new block[structure_size], *c0 = new block[structure_size], *c1 = new block[structure_size];
        block *structure_first_p0 = new block[max_structure_consumption];
        vector<tuple<word, word, double>> surviving_kgs_and_scores;
        uint32_t print_flush_interval = attack_num / 5;
        if (print_flush_interval == 0)
            print_flush_interval = 1;
        
        for (int attack_index = 0; attack_index < attack_num; attack_index++) {
            if (attack_index % print_flush_interval == 0)
                fflush(thread_output_file);
            fprintf(thread_output_file, "Attack index: %d.\n", attack_index);
            // Generate user key
            generate_one_user_key(mk, thread_random_generator);
            expand_key(mk, rk, attack_nr);
            tk1 = rk[attack_nr - 1] & rk1_mask;
            tk2 = rk[attack_nr - 2] & rk2_mask;
            // Generate plaintexts
            for (uint32_t i = 0; i < max_structure_consumption; i++) {
                structure_first_p0[i] = generate_one_plaintext(thread_random_generator);
            }
            num_used_structures = 0;
            surviving_kgs_and_scores.clear();
            start_time = chrono::system_clock::now();
            while (num_used_structures < max_structure_consumption) {
                // Generate one plaintext structure using neutral bits
                p0[0] = structure_first_p0[num_used_structures];
                num_used_structures++;
                generate_one_plaintext_structure(in_diff, p0, p1, NBs);

                // -----------------------Debug-----------------------
                // Early filter all wrong plaintext structures
                if (debug_filter_valid_structure) {
                    encrypt(p0[0], rk, pre_nr, c0[0]);
                    encrypt(p1[0], rk, pre_nr, c1[0]);
                    if (((c0[0].first ^ c1[0].first) != dis_diff.first) || ((c0[0].second ^ c1[0].second) != dis_diff.second)) {
                        continue;
                    }
                    uint32_t valid_num = 0;
                    for (uint32_t i = 0; i < structure_size; i++) {
                        encrypt(p0[i], rk, pre_nr, c0[i]);
                        encrypt(p1[i], rk, pre_nr, c1[i]);
                        if (((c0[i].first ^ c1[i].first) == dis_diff.first) && ((c0[i].second ^ c1[i].second) == dis_diff.second)) {
                            valid_num++;
                        }
                    }
                    if (valid_num != structure_size)
                        continue;
                }
                // -----------------------Debug-----------------------

                // Collect ciphertext structures
                for (uint32_t i = 0; i < structure_size; i++) {
                    encrypt(p0[i], rk, attack_nr, c0[i]);
                    encrypt(p1[i], rk, attack_nr, c1[i]);
                }
                // Try to recover subkeys using one ciphertext structure
                attack_with_one_structure(c0, c1, surviving_kgs_and_scores);
                if (!surviving_kgs_and_scores.empty())
                    break;
            }
            end_time = chrono::system_clock::now();
            auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time);
            // This attack ends
            structure_consumption_sum += num_used_structures;
            attack_time_ms_sum += duration.count();
            fprintf(thread_output_file, "Attack #%d ends. Time cost: %f s.\n", attack_index, double(duration.count()) / 1000.0);
            fprintf(thread_output_file, "Plaintext structure consumption: %d.\n", num_used_structures);
            if (surviving_kgs_and_scores.empty()) {
                // No key guess survives
                fprintf(thread_output_file, "No key guess survives. Attack fails!\n");
            } else {
                // There is at least one key guess surviving
                // Return the key guess with the highest kg_score as the right key
                key_surviving_time++;
                fprintf(thread_output_file, "%d key guesses survives.\n", surviving_kgs_and_scores.size());
                word fk1, fk2, dk1, dk2;
                uint32_t hd;
                double max_score = -1 * 1e5;
                for (auto x : surviving_kgs_and_scores) {
                    if (get<2>(x) > max_score) {
                        fk1 = get<0>(x);
                        fk2 = get<1>(x);
                        max_score = get<2>(x);
                    }
                }
                dk1 = (fk1 ^ tk1) & rk1_mask;
                dk2 = (fk2 ^ tk2) & rk2_mask;
                hd = cal_hw(dk1, WORD_SIZE) + cal_hw(dk2, WORD_SIZE);
                fprintf(thread_output_file, "dk1 is 0x%llx, dk2 is 0x%llx, hamming distance is %u.\n", dk1, dk2, hd);
                if (hd <= max_hw_metric) {
                    attack_success_time++;
                    fprintf(thread_output_file, "Attack succeeds!\n");
                } else {
                    fprintf(thread_output_file, "Attack fails!\n");
                }
            }
        }
        // Output attack results
        fprintf(thread_output_file, "[Attack result summary]\n");
        fprintf(thread_output_file, "Attack number of times: %d.\nKey guess surviving number of times: %d.\nAttack success number of times: %d.\n", attack_num, key_surviving_time, attack_success_time);
        fprintf(thread_output_file, "Average time cost: %f s.\n", (double)attack_time_ms_sum / 1000.0 / attack_num);
        fprintf(thread_output_file, "Average plaintext structure consumption: %f.\n", (double)structure_consumption_sum / attack_num);
        fflush(thread_output_file);
        delete[] p0; delete[] p1; delete[] c0; delete[] c1; delete[] structure_first_p0;
    }
};

void load_attack_settings() {
    // 14r attack setting
    attack_nr = 1 + 4 + 8 + 1;
    pre_nr = 1 + 4;
    in_diff = {0x800a080808ull, 0x800124a0848ull};
    dis_diff = {0x80ull, 0ull};
    max_structure_consumption = 1 << 23;
    NBs = {32,33,34,35,36,87,88,89,90,91};

    structure_size = 1 << NBs.size();
    rk1_mask = 0xff0000ffffffull;
    rk2_mask = (1ull << 21) - 1;
    th1 = 0;
    th2 = 0;
    th3 = 0;
    th4 = 0;
    kg1_high_bits1 = 2;
    kg1_low_bits1 = 5;
    kg1_high_offset_bits1 = 11;
    kg1_high_bits2 = 8;
    kg1_low_bits2 = 6;
    kg1_high_offset_bits2 = 13;
    kg1_low_offset_bits2 = 5;
    kg1_bits3 = 3;
    kg1_high_offset_bits3 = 21;
    kg2_bits3 = 13;
    kg2_high_offset_bits3 = 8;
    kg1_bits4 = 8;
    kg2_bits4 = 8;
    kg1_high_offset_bits4 = 40;
    top_k1 = 10;
    top_k2 = 10;
    top_k3 = 10;
    extracter1 = get_extracter(ATTACK_8R_21_20_AND_13_8);
    extracter2 = get_extracter(ATTACK_8R_29_8);
    extracter3 = get_extracter(ATTACK_7R_29_16);
    extracter4 = get_extracter(ATTACK_7R_29_8);
    max_hw_metric = 0;

    // Load lookup table distinguishers
    // ATTACK_8R_21_20_AND_13_8
    uint64_t input_space = 1ull << 14;
    uint64_t *lookup_table = new uint64_t[input_space];
    FILE *input_file = fopen("./lookup_table/8r_table_14_16_8R_21_20_AND_13_8", "rb");
    fread((void *)lookup_table, sizeof(uint64_t), input_space, input_file);
    response_table1 = new double[input_space];
    build_response_table(14, lookup_table, response_table1);
    fclose(input_file);
    delete[] lookup_table;
    test_distinguishing_acc(1<<20, 8, dis_diff, response_table1, extracter1);
    // ATTACK_8R_29_8
    input_space = 1ull << 23;
    lookup_table = new uint64_t[input_space];
    input_file = fopen("./lookup_table/8r_table_23_8_8R_29_8", "rb");
    fread((void *)lookup_table, sizeof(uint64_t), input_space, input_file);
    response_table2 = new double[input_space];
    build_response_table(23, lookup_table, response_table2);
    fclose(input_file);
    delete[] lookup_table;
    test_distinguishing_acc(1<<20, 8, dis_diff, response_table2, extracter2);
    // ATTACK_7R_29_16
    input_space = 1ull << 16;
    lookup_table = new uint64_t[input_space];
    input_file = fopen("./lookup_table/7r_table_16_14_7R_29_16", "rb");
    fread((void *)lookup_table, sizeof(uint64_t), input_space, input_file);
    response_table3 = new double[input_space];
    build_response_table(16, lookup_table, response_table3);
    fclose(input_file);
    delete[] lookup_table;
    test_distinguishing_acc(1<<20, 7, dis_diff, response_table3, extracter3);
    // ATTACK_7R_29_8
    input_space = 1ull << 26;
    lookup_table = new uint64_t[input_space];
    input_file = fopen("./lookup_table/7r_table_26_6_7R_29_8", "rb");
    fread((void *)lookup_table, sizeof(uint64_t), input_space, input_file);
    response_table4 = new double[input_space];
    build_response_table(26, lookup_table, response_table4);
    fclose(input_file);
    delete[] lookup_table;
    test_distinguishing_acc(1<<20, 7, dis_diff, response_table4, extracter4);
}

int main() {
    bool check_res = check_testvector();
    printf("check testvector res is %d.\n", check_res);
    // Set random seed
    random_generator.set_rand_seed(time(NULL));

    load_attack_settings();
    // Set total attack number of times
    uint32_t n = 10;
    printf("Total Attack numbr of times: %d.\n", n);
    uint32_t n_per_thread = (n + ATTACK_THREAD_NUM - 1) / ATTACK_THREAD_NUM;
    // Thread pool and task pool
    thread* thread_pool[ATTACK_THREAD_NUM];
    ThreadTask task_pool[ATTACK_THREAD_NUM];
    uint32_t tmp_n = n;
    for (int i = 0; i < ATTACK_THREAD_NUM; i++) {
        if (n_per_thread > tmp_n)
            task_pool[i].attack_num = tmp_n;
        else
            task_pool[i].attack_num = n_per_thread;
        tmp_n -= task_pool[i].attack_num;
        string thread_output_path = "./attack_record/14r_attack_thread_output_" + to_string(i) + ".txt";
        task_pool[i].thread_output_file = fopen(thread_output_path.c_str(), "w");
    }
    // Generate a random generator for each thread
    word mk[M];
    block tmp_p;
    for (int i = 0; i < ATTACK_THREAD_NUM; i++) {
        task_pool[i].thread_random_generator = new RandomGenerator(random_generator);
        for (int attack_index = 0; attack_index < task_pool[i].attack_num; attack_index++) {
            generate_one_user_key(mk);
            for (int j = 0; j < max_structure_consumption; j++) {
                tmp_p = generate_one_plaintext();
            }
        }
    }
    // Attack begins
    for (int i = 0; i < ATTACK_THREAD_NUM; i++) {
        thread_pool[i] = new thread(&ThreadTask::attack_4stages, &(task_pool[i]));
    }
    for (int i = 0; i < ATTACK_THREAD_NUM; i++) {
        thread_pool[i]->join();
    }
    // Output attack summary
    printf("[Attack result summary]\n");
    uint32_t key_surviving_num = 0, attack_success_num = 0;
    uint64_t attack_time_ms_sum = 0;
    uint64_t structure_consumption_sum = 0;
    for (int i = 0; i < ATTACK_THREAD_NUM; i++) {
        key_surviving_num += task_pool[i].key_surviving_time;
        attack_success_num += task_pool[i].attack_success_time;
        attack_time_ms_sum += task_pool[i].attack_time_ms_sum;
        structure_consumption_sum += task_pool[i].structure_consumption_sum;
    }
    printf("Key guess surviving number of times: %d.\nAttack success number of times: %d.\n", key_surviving_num, attack_success_num);
    printf("Average time cost: %f s.\n", (double)attack_time_ms_sum / 1000.0 / n);
    printf("Average data complexity: 2^%f.\n", log2((double)structure_consumption_sum / n) + 11);

    for (int i = 0; i < ATTACK_THREAD_NUM; i++) {
        delete thread_pool[i];
        delete task_pool[i].thread_random_generator;
        fclose(task_pool[i].thread_output_file);
    }

    delete[] response_table1;
    delete[] response_table2;
    delete[] response_table3;
    delete[] response_table4;
}