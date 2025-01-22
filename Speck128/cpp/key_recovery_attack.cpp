#include "util.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <assert.h>
#include <vector>
#include <tuple>
#include <numeric>
#include <algorithm>
#include <chrono>
#include <thread>
#include <string>
using namespace std;

// Set the number of threads
#define ATTACK_THREAD_NUM 10

bool debug_pre_filter_right_structure = false;

static bool compare(const tuple<word, double>& v1, const tuple<word, double>& v2) {
    return get<1>(v1) > get<1>(v2);
}

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

struct attack_stage_params {
    block in_diff;
    block dis_diff;
    vector<uint32_t> NBs;
    double c;
    uint32_t max_structure_consumption;
    uint32_t kg_bits;
    uint32_t kg_low_bits;
    uint32_t structure_size;
    uint32_t top_k;
    InputExtracter extracter;
    double *response_table;
};

struct attack_params {
    uint32_t n;
    uint32_t attack_nr;
    uint32_t pre_nr;
    word rk_mask;
    uint32_t max_hd_metric;
    attack_stage_params params[5];
};

struct attack_record {
    uint32_t key_surviving_time = 0;
    uint32_t attack_success_time = 0;
    uint32_t tmp_structure_consumption;
    vector<uint32_t> structure_consumption;
    double average_structure_consumption;
    vector<double> attack_time;
    double average_attack_time;
};

class ThreadTask {
private:
    // Recover related subkey bits using one ciphertext structure
    void attack_with_one_structure(const attack_stage_params& params, const block c0[], const block c1[], const vector<word>& surviving_pre_kgs, vector<word>& surviving_kgs) {
        vector<tuple<word, double>> surviving_kgs_scores;
        word kg_space = 1ull << params.kg_bits;
        word kg;
        block t0, t1, x0, x1;
        double score;
        double *kg_scores = new double[kg_space];
        for (word kg_low : surviving_pre_kgs) {
            for (word kg_high = 0; kg_high < kg_space; kg_high++)
                kg_scores[kg_high] = 0;
            for (uint32_t i = 0; i < params.structure_size; i++) {
                x0 = c0[i];
                x1 = c1[i];
                for (word kg_high = 0; kg_high < kg_space; kg_high++) {
                    kg = (kg_high << params.kg_low_bits) | kg_low;
                    dec_one_round(x0, kg, t0);
                    dec_one_round(x1, kg, t1);
                    kg_scores[kg_high] += params.response_table[params.extracter.extract_distinguisher_input(t0, t1)];
                }
            }
            for (word kg_high = 0; kg_high < kg_space; kg_high++) {
                kg_scores[kg_high] /= params.structure_size;
                if (kg_scores[kg_high] > params.c) {
                    surviving_kgs_scores.push_back(tuple<word, double>((kg_high << params.kg_low_bits) | kg_low, kg_scores[kg_high]));
                }
            }
        }
        delete[] kg_scores;
        // No key guess survives for this ciphertext structure
        if (surviving_kgs_scores.empty()) {
            return;
        }

        // Return the best top_k key guesses for next stage
        sort(surviving_kgs_scores.begin(), surviving_kgs_scores.end(), compare);

        for (uint32_t i = 0; (i < surviving_kgs_scores.size()) && (i < params.top_k); i++) {
            surviving_kgs.push_back(get<0>(surviving_kgs_scores[i]));
        }
    }

    void attack_one_stage(const attack_stage_params& params, const uint32_t& attack_nr, block structure_first_p0[], block p0[], block p1[], block c0[], block c1[], const word user_rk[], const vector<word>& surviving_pre_kgs, vector<word>& surviving_kgs) {
        surviving_kgs.clear();
        if (surviving_pre_kgs.empty())
            return;
        uint32_t num_used_structures = 0;
        while (num_used_structures < params.max_structure_consumption) {
            // Generate one plaintext structure using neutral bits
            p0[0] = structure_first_p0[num_used_structures];
            generate_one_plaintext_structure(params.in_diff, p0, p1, params.NBs);
            num_used_structures++;

            // -----------------------Debug-----------------------
            // Early filter all wrong plaintext structures
            if (debug_pre_filter_right_structure) {
                bool valid_flag = true;
                for (uint32_t i = 0; i < params.structure_size; i++) {
                    encrypt(p0[i], user_rk, attack_nr - 10, c0[i]);
                    encrypt(p1[i], user_rk, attack_nr - 10, c1[i]);
                    if ((c0[i].first ^ c1[i].first) != params.dis_diff.first || (c0[i].second ^ c1[i].second) != params.dis_diff.second) {
                        valid_flag = false;
                        break;
                    }
                }
                if (!valid_flag)
                    continue;
            }
            // -----------------------Debug-----------------------

            // Collect ciphertext structures
            collect_ciphertext_structure(params.structure_size, attack_nr, p0, p1, c0, c1, user_rk);

            // Try to recover subkey bits on this ciphertext structure
            attack_with_one_structure(params, c0, c1, surviving_pre_kgs, surviving_kgs);
            if (!surviving_kgs.empty()) {
                // If there is any key guess surviving, this attack stage ends
                break;
            }
        }
        record.tmp_structure_consumption += num_used_structures;
    }
public:
    uint32_t attack_num;
    const attack_params* total_params;
    attack_record record;
    RandomGenerator* thread_random_generator = nullptr;
    FILE *thread_output_file;
    word debug_tk1, debug_tk2, debug_tk3, debug_tk4, debug_tk5;
    uint32_t stage_identifier;
    // The whole attack experiment
    void attack_5stages() {
        uint32_t max_structure_size = 0;
        for (int i = 0; i < 5; i++) {
            if (total_params->params[i].structure_size > max_structure_size) {
                max_structure_size = total_params->params[i].structure_size;
            }
        }
        block *p0 = new block[max_structure_size], *p1 = new block[max_structure_size], *c0 = new block[max_structure_size], *c1 = new block[max_structure_size];
        block* structure_first_p0[5];
        for (int i = 0; i < 5; i++) {
            structure_first_p0[i] = new block[total_params->params[i].max_structure_consumption];
        }
        word mk[M], rk[MAX_NR];
        word tk, bk, dk;
        uint32_t hd;
        vector<word> initial_kgs;
        initial_kgs.push_back(0);
        vector<word> surviving_kgs_1;
        vector<word> surviving_kgs_2;
        vector<word> surviving_kgs_3;
        vector<word> surviving_kgs_4;
        vector<word> surviving_kgs_5;
        uint32_t print_flush_interval = attack_num / 5;
        if (print_flush_interval == 0) print_flush_interval = 1;
        for (int attack_index = 0; attack_index < attack_num; attack_index++) {
            if (attack_index % print_flush_interval == 0)
                fflush(thread_output_file);
            fprintf(thread_output_file, "Attack #%d begins.\n", attack_index);
            surviving_kgs_1.clear();
            surviving_kgs_2.clear();
            surviving_kgs_3.clear();
            surviving_kgs_4.clear();
            surviving_kgs_5.clear();
            record.tmp_structure_consumption = 0;
            // Generate user key
            generate_one_user_key(mk, thread_random_generator);
            expand_key(mk, rk, total_params->attack_nr);
            // Generate random plaintexts used in each attack stage
            for (int i = 0; i < 5; i++) {
                for (int j = 0; j < total_params->params[i].max_structure_consumption; j++) {
                    structure_first_p0[i][j] = generate_one_plaintext(thread_random_generator);
                }
            }
            tk = rk[total_params->attack_nr - 1];
            debug_tk1 = tk & ((1ull << 14) - 1);
            debug_tk2 = tk & ((1ull << 26) - 1);
            debug_tk3 = tk & ((1ull << 40) - 1);
            debug_tk4 = tk & ((1ull << 55) - 1);
            debug_tk5 = tk & total_params->rk_mask;

            auto start_time = chrono::system_clock::now();

            // Stage1
            fprintf(thread_output_file, "[Debug] Begin stage1.\n");
            stage_identifier = 1;
            attack_one_stage(total_params->params[0], total_params->attack_nr, structure_first_p0[0], p0, p1, c0, c1, rk, initial_kgs, surviving_kgs_1);

            // Stage2
            fprintf(thread_output_file, "[Debug] Begin stage2.\n");
            stage_identifier = 2;
            attack_one_stage(total_params->params[1], total_params->attack_nr, structure_first_p0[1], p0, p1, c0, c1, rk, surviving_kgs_1, surviving_kgs_2);

            // Stage3
            fprintf(thread_output_file, "[Debug] Begin stage3.\n");
            stage_identifier = 3;
            attack_one_stage(total_params->params[2], total_params->attack_nr, structure_first_p0[2], p0, p1, c0, c1, rk, surviving_kgs_2, surviving_kgs_3);

            // Stage4
            fprintf(thread_output_file, "[Debug] Begin stage4.\n");
            stage_identifier = 4;
            attack_one_stage(total_params->params[3], total_params->attack_nr, structure_first_p0[3], p0, p1, c0, c1, rk, surviving_kgs_3, surviving_kgs_4);

            // Stage5
            fprintf(thread_output_file, "[Debug] Begin stage5.\n");
            stage_identifier = 5;
            attack_one_stage(total_params->params[4], total_params->attack_nr, structure_first_p0[4], p0, p1, c0, c1, rk, surviving_kgs_4, surviving_kgs_5);

            auto end_time = chrono::system_clock::now();
            auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time);
            record.attack_time.push_back(double(duration.count()) / 1000.0);
            record.structure_consumption.push_back(record.tmp_structure_consumption);
            // This attack ends
            fprintf(thread_output_file, "Attack #%d ends. Time cost: %f s.\n", attack_index, record.attack_time[attack_index]);
            fprintf(thread_output_file, "Plaintext structure consumption: %d.\n", record.tmp_structure_consumption);
            if (surviving_kgs_5.empty()) {
                // No key guess survives
                fprintf(thread_output_file, "No key guess survives. Attack fails!\n");
            } else {
                // There is at least one key guess surviving
                // Return the key guess with the highest kg_score as the right key
                record.key_surviving_time++;
                bk = surviving_kgs_5[0];
                dk = (tk ^ bk) & total_params->rk_mask;
                hd = cal_hw(dk, WORD_SIZE);
                fprintf(thread_output_file, "dk is 0x%llx, hamming distance is %d.\n", dk, hd);
                if (hd <= total_params->max_hd_metric) {
                    record.attack_success_time++;
                    fprintf(thread_output_file, "Attack succeeds!\n");
                } else {
                    fprintf(thread_output_file, "Attack fails!\n");
                }
            }
        }
        // Output attack results
        record.average_attack_time = accumulate(begin(record.attack_time), end(record.attack_time), 0.0) / record.attack_time.size();
        record.average_structure_consumption = accumulate(begin(record.structure_consumption), end(record.structure_consumption), 0) / (record.structure_consumption.size() + 0.0);
        fprintf(thread_output_file, "[Attack result summary]\n");
        fprintf(thread_output_file, "Attack number of times: %d.\nKey guess surviving number of times: %d.\nAttack success number of times: %d.\n", attack_num, record.key_surviving_time, record.attack_success_time);
        fprintf(thread_output_file, "Average time cost: %f s.\n", record.average_attack_time);
        fprintf(thread_output_file, "Average plaintext structure consumption: %f.\n", record.average_structure_consumption);
        fflush(thread_output_file);
        delete[] p0; delete[] p1; delete[] c0; delete[] c1;
        for (int i = 0; i < 5; i++)
            delete[] structure_first_p0[i];
    }
};

void load_attack_settings(attack_params& total_params) {
    total_params.rk_mask = (1ull << 63) - 1;
    total_params.max_hd_metric = 2;
    total_params.attack_nr = 14;
    total_params.pre_nr = total_params.attack_nr - 10;

    total_params.params[0].in_diff = {0x8000000001248000ull, 0x0080000000002084ull};
    total_params.params[1].in_diff = {0x1248000800ull, 0x2084008ull};
    total_params.params[2].in_diff = {0x4920002000000ull, 0x8210020000ull};
    total_params.params[3].in_diff = {0x4900010000000002ull, 0x41080100000000ull};
    total_params.params[4].in_diff = {0x1000000000249ull, 0x4108010000000000ull};

    total_params.params[0].NBs = {18,32,38,39,43,45,88,92,96,110};
    total_params.params[1].NBs = {43, 49, 54, 56, 101, 104, 107, 115, 124, 127};
    total_params.params[2].NBs = {1,6,44,47,55,63,68,75,115,124};
    total_params.params[3].NBs = {4, 7, 9, 11, 12, 67, 71, 73, 78, 88};
    total_params.params[4].NBs = {2,10,15,26,31,74,82,93,98,103};

    total_params.params[0].max_structure_consumption = 1 << 14;
    total_params.params[1].max_structure_consumption = 1 << 14;
    total_params.params[2].max_structure_consumption = 1 << 14;
    total_params.params[3].max_structure_consumption = 1 << 14;
    total_params.params[4].max_structure_consumption = 1 << 14;


    total_params.params[0].dis_diff = {0x1ull, 0};
    total_params.params[1].dis_diff = {0x1000ull, 0};
    total_params.params[2].dis_diff = {0x4000000ull, 0};
    total_params.params[3].dis_diff = {0x20000000000ull, 0};
    total_params.params[4].dis_diff = {0x2000000000000ull, 0};

    total_params.params[0].c = 0;
    total_params.params[1].c = 0;
    total_params.params[2].c = 0;
    total_params.params[3].c = 0;
    total_params.params[4].c = 0;

    total_params.params[0].kg_bits = 14;
    total_params.params[1].kg_bits = 12;
    total_params.params[2].kg_bits = 14;
    total_params.params[3].kg_bits = 15;
    total_params.params[4].kg_bits = 8;

    total_params.params[0].kg_low_bits = 0;
    total_params.params[1].kg_low_bits = 14;
    total_params.params[2].kg_low_bits = 26;
    total_params.params[3].kg_low_bits = 40;
    total_params.params[4].kg_low_bits = 55;

    for (int i = 0; i < 5; i++) {
        total_params.params[i].structure_size = 1 << total_params.params[i].NBs.size();
        assert(total_params.params[i].structure_size == 1024);
    }
    
    total_params.params[0].top_k = 3;
    total_params.params[1].top_k = 3;
    total_params.params[2].top_k = 3;
    total_params.params[3].top_k = 3;
    total_params.params[4].top_k = 1;

    total_params.params[0].extracter = get_extracter(ATTACK_9R_DIFF64);
    total_params.params[1].extracter = get_extracter(ATTACK_9R_DIFF76);
    total_params.params[2].extracter = get_extracter(ATTACK_9R_DIFF90);
    total_params.params[3].extracter = get_extracter(ATTACK_9R_DIFF105);
    total_params.params[4].extracter = get_extracter(ATTACK_9R_DIFF113);

    // Load lookup table distinguishers
    // ATTACK_9R_DIFF64
    uint64_t input_space = 1ull << 18;
    uint64_t *lookup_table = new uint64_t[input_space];
    double *response_table = new double[input_space];
    FILE *input_file = fopen("lookup_table/9r_table_18_14_9R_DIFF64", "rb");
    fread((void *)lookup_table, sizeof(uint64_t), input_space, input_file);
    fclose(input_file);
    build_response_table(18, lookup_table, response_table);
    delete[] lookup_table;
    total_params.params[0].response_table = response_table;
    test_distinguishing_acc(1<<20, 9, total_params.params[0].dis_diff, total_params.params[0].response_table, total_params.params[0].extracter);
    // ATTACK_9R_DIFF76
    input_space = 1ull << 20;
    lookup_table = new uint64_t[input_space];
    response_table = new double[input_space];
    input_file = fopen("lookup_table/9r_table_20_12_9R_DIFF76", "rb");
    fread((void *)lookup_table, sizeof(uint64_t), input_space, input_file);
    fclose(input_file);
    build_response_table(20, lookup_table, response_table);
    delete[] lookup_table;
    total_params.params[1].response_table = response_table;
    test_distinguishing_acc(1<<20, 9, total_params.params[1].dis_diff, total_params.params[1].response_table, total_params.params[1].extracter);
    // ATTACK_9R_DIFF90
    input_space = 1ull << 19;
    lookup_table = new uint64_t[input_space];
    response_table = new double[input_space];
    input_file = fopen("lookup_table/9r_table_19_13_9R_DIFF90", "rb");
    fread((void *)lookup_table, sizeof(uint64_t), input_space, input_file);
    fclose(input_file);
    build_response_table(19, lookup_table, response_table);
    delete[] lookup_table;
    total_params.params[2].response_table = response_table;
    test_distinguishing_acc(1<<20, 9, total_params.params[2].dis_diff, total_params.params[2].response_table, total_params.params[2].extracter);
    // ATTACK_9R_DIFF105
    input_space = 1ull << 20;
    lookup_table = new uint64_t[input_space];
    response_table = new double[input_space];
    input_file = fopen("lookup_table/9r_table_20_12_9R_DIFF105", "rb");
    fread((void *)lookup_table, sizeof(uint64_t), input_space, input_file);
    fclose(input_file);
    build_response_table(20, lookup_table, response_table);
    delete[] lookup_table;
    total_params.params[3].response_table = response_table;
    test_distinguishing_acc(1<<20, 9, total_params.params[3].dis_diff, total_params.params[3].response_table, total_params.params[3].extracter);
    // ATTACK_9R_DIFF113
    input_space = 1ull << 24;
    lookup_table = new uint64_t[input_space];
    response_table = new double[input_space];
    input_file = fopen("lookup_table/9r_table_24_8_9R_DIFF113", "rb");
    fread((void *)lookup_table, sizeof(uint64_t), input_space, input_file);
    fclose(input_file);
    build_response_table(24, lookup_table, response_table);
    delete[] lookup_table;
    total_params.params[4].response_table = response_table;
    test_distinguishing_acc(1<<20, 9, total_params.params[4].dis_diff, total_params.params[4].response_table, total_params.params[4].extracter);
}

int main() {
    bool check_res = check_testvector();
    printf("check testvector res is %d.\n", check_res);
    // Set random seed
    random_generator.set_rand_seed(time(NULL));

    // Set total attack number of times
    uint32_t n = 10;
    printf("Total attack numbr of times: %d.\n", n);

    attack_params total_params;
    uint32_t n_per_thread = (n + ATTACK_THREAD_NUM - 1) / ATTACK_THREAD_NUM;
    load_attack_settings(total_params);
    // Thread pool and task pool
    thread* thread_pool[ATTACK_THREAD_NUM];
    ThreadTask task_pool[ATTACK_THREAD_NUM];
    for (int i = 0; i < ATTACK_THREAD_NUM; i++) {
        if (n_per_thread > n)
            task_pool[i].attack_num = n;
        else
            task_pool[i].attack_num = n_per_thread;
        n -= task_pool[i].attack_num;
        task_pool[i].total_params = &total_params;
        #ifdef SPECK128_128
        string thread_output_path = "./attack_record/128_thread_output_" + to_string(i) + ".txt";
        #endif
        #ifdef SPECK128_192
        string thread_output_path = "./attack_record/192_thread_output_" + to_string(i) + ".txt";
        #endif
        #ifdef SPECK128_256
        string thread_output_path = "./attack_record/256_thread_output_" + to_string(i) + ".txt";
        #endif
        task_pool[i].thread_output_file = fopen(thread_output_path.c_str(), "w");
    }
    // Generate a random generator for each thread
    word mk[M];
    for (int i = 0; i < ATTACK_THREAD_NUM; i++) {
        task_pool[i].thread_random_generator = new RandomGenerator(random_generator);
        for (int attack_index = 0; attack_index < task_pool[i].attack_num; attack_index++) {
            generate_one_user_key(mk);
            for (int j = 0; j < 5; j++) {
                for (int k = 0; k < total_params.params[j].max_structure_consumption; k++) {
                    generate_one_plaintext();
                }
            }
        }
    }
    // Attack begins
    for (int i = 0; i < ATTACK_THREAD_NUM; i++) {
        thread_pool[i] = new thread(&ThreadTask::attack_5stages, &(task_pool[i]));
    }
    for (int i = 0; i < ATTACK_THREAD_NUM; i++) {
        thread_pool[i]->join();
    }
    // Output attack summary
    uint32_t total_kg_surviving_time = 0, total_success_time = 0;
    double total_aver_running_time = 0, total_structure_num = 0;
    for (int i = 0; i < ATTACK_THREAD_NUM; i++) {
        total_kg_surviving_time += task_pool[i].record.key_surviving_time;
        total_success_time += task_pool[i].record.attack_success_time;
        total_aver_running_time += task_pool[i].record.average_attack_time;
        total_structure_num += task_pool[i].record.average_structure_consumption;
    }
    printf("[Attack result summary]\n");
    printf("Key guess surviving number of times: %d.\nAttack success number of times: %d.\n", total_kg_surviving_time, total_success_time);
    printf("Average time cost: %f s.\n", total_aver_running_time / ATTACK_THREAD_NUM);
    printf("Average data complexity: 2^%f.\n", log2(total_structure_num / ATTACK_THREAD_NUM) + 11);
    for (int i = 0; i < ATTACK_THREAD_NUM; i++) {
        delete thread_pool[i];
        delete task_pool[i].thread_random_generator;
        fclose(task_pool[i].thread_output_file);
    }
    for (int i = 0; i < 5; i++) {
        delete[] total_params.params[i].response_table;
    }
}