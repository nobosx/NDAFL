#include "lea.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string>
#include <math.h>
using namespace std;

#define SETTING_9R 1
#define SETTING_10R 2
#define SETTING_11R 3

void build_counter_table(const uint32_t& num_rounds, const block& diff, uint64_t lookup_table[], const uint32_t& input_bits, const uint64_t& average_num, const uint32_t& dis_setting, const bool& calc_back=true) {
    uint64_t (*extract_bits_from_block)(const block&, const block&);
    switch (dis_setting)
    {
    case SETTING_9R:
        extract_bits_from_block = extract_bits_from_block_9r;
        break;
    case SETTING_10R:
        extract_bits_from_block = extract_bits_from_block_10r;
        break;
    case SETTING_11R:
        extract_bits_from_block = extract_bits_from_block_11r;
        break;
    default:
        printf("undefined distinguishing setting!!!\n");
        exit(0);
    }

    uint64_t input_space = 1ull << input_bits;
    block *c0 = new block[average_num], *c1 = new block[average_num];
    for (uint64_t i = 0; i < input_space; i++) lookup_table[i] = 0;
    for (uint64_t i = 0; i < input_space; i++) {
        make_target_diff_samples(average_num, num_rounds, diff, c0, c1, true, calc_back);
        for (uint64_t j = 0; j < average_num; j++) ++lookup_table[(*extract_bits_from_block)(c0[j], c1[j])];
    }
    delete[] c0; delete[] c1;
}

void test_distinguishing_acc(const uint32_t& n, const uint32_t& num_rounds, const block& diff, const uint64_t lookup_table[], const uint32_t& input_bits, const uint32_t& dis_setting, const bool& calc_back=true) {
    uint64_t (*extract_bits_from_block)(const block&, const block&);
    switch (dis_setting)
    {
    case SETTING_9R:
        extract_bits_from_block = extract_bits_from_block_9r;
        break;
    case SETTING_10R:
        extract_bits_from_block = extract_bits_from_block_10r;
        break;
    case SETTING_11R:
        extract_bits_from_block = extract_bits_from_block_11r;
        break;
    default:
        printf("undefined distinguishing setting!!!\n");
        exit(0);
    }

    uint64_t input_space = 1ull << input_bits;
    uint64_t sample_num = 0;
    uint64_t average_num;
    for (uint64_t i = 0; i < input_space; i++) sample_num += lookup_table[i];
    average_num = sample_num / input_space;
    printf("average num in log2: %f.\n", log2(average_num));
    block *c0 = new block[n], *c1 = new block[n];
    uint32_t true_positive_num = 0, true_negative_num = 0;

    // positive
    make_target_diff_samples(n, num_rounds, diff, c0, c1, true, calc_back);
    for (uint32_t i = 0; i < n; i++) {
        if (lookup_table[(*extract_bits_from_block)(c0[i], c1[i])] > average_num) true_positive_num++;
    }

    // negative
    make_target_diff_samples(n, num_rounds, diff, c0, c1, false, calc_back);
    for (uint32_t i = 0; i < n; i++) {
        if (lookup_table[(*extract_bits_from_block)(c0[i], c1[i])] < average_num) true_negative_num++;
    }

    // output
    double tpr = (true_positive_num + 0.0) / n, tnr = (true_negative_num + 0.0) / n;
    double acc = (tpr + tnr) / 2;
    printf("Acc = %f, tpr = %f, tnr = %f\n", acc, tpr, tnr);

    delete[] c0; delete[] c1;
}

int main() {
    // check test vector
    bool res = check_test_vectors(128);
    res = res && check_test_vectors(192);
    res = res && check_test_vectors(256);
    printf("check testvector res: %d\n", res);

    random_generator.set_rand_seed(time(NULL));

    block diff = {0x80000000u, 0x80000000u, 0x80000000u, 0x80000000u};
    bool calc_back = true;

    // 9R
    uint32_t num_rounds = 9;
    uint32_t input_bits = 33;
    uint32_t average_num_in_bits = 5;
    uint32_t dis_setting = SETTING_9R;
    string dis_tag = "9R";

    // 10R
    // uint32_t num_rounds = 10;
    // uint32_t input_bits = 27;
    // uint32_t average_num_in_bits = 9;
    // uint32_t dis_setting = SETTING_10R;
    // string dis_tag = "10R";

    // // 11R
    // uint32_t num_rounds = 11;
    // uint32_t input_bits = 17;
    // uint32_t average_num_in_bits = 15;
    // uint32_t dis_setting = SETTING_11R;
    // string dis_tag = "11R";

    uint64_t average_num = 1ull << average_num_in_bits;
    string table_path = "./lookup_table/" + to_string(num_rounds) + "r_table_" + to_string(input_bits) + "_" + to_string(average_num_in_bits) + "_" + dis_tag;
    uint64_t input_space = 1ull << input_bits;
    uint64_t *lookup_table = new uint64_t[input_space];
    
    // Build a counter lookup table
    printf("Building lookup table...\n");
    clock_t start = clock();
    build_counter_table(num_rounds, diff, lookup_table, input_bits, average_num, dis_setting, calc_back);
    clock_t end = clock();
    printf("Building time: %f s.\n", (end - start + 0.0) / CLOCKS_PER_SEC);

    // If the table already exists, just load it using the following code
    FILE *input_file = fopen(table_path.c_str(), "rb");
    fread((void *)lookup_table, sizeof(uint64_t), input_space, input_file);
    fclose(input_file);
    if (input_file == NULL) printf("Table not found!\n");

    // Test distinguisher accuracy
    test_distinguishing_acc(1<<24, num_rounds, diff, lookup_table, input_bits, dis_setting, calc_back);

    // Save the lookup table
    printf("Saved lookup file path: %s\n", table_path.c_str());
    FILE *output_file = fopen(table_path.c_str(), "wb");
    if (output_file == NULL) {
        printf("Lookup file dir not created yet!\n");
    } else {
        fwrite((const void *)lookup_table, sizeof(uint64_t), input_space, output_file);
        fclose(output_file);
    }

    delete[] lookup_table;
    return 0;
}