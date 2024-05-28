#include "speck.h"
#include "util.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include <string>

#define ID1_9R 1
#define ID2_9R 2
#define ID2_10R 3

void build_counter_table(const uint32_t& num_rounds, const block& diff, uint64_t lookup_table[], const uint32_t& input_bits, const uint64_t& average_num, const uint32_t& dis_setting) {
    uint64_t (*extract_bits_from_block)(const block&, const block&);
    switch (dis_setting) {
    case ID1_9R:
        extract_bits_from_block = extract_bits_from_block_9r_ID1;
        break;
    case ID2_9R:
        extract_bits_from_block = extract_bits_from_block_9r_ID2;
        break;
    case ID2_10R:
        extract_bits_from_block = extract_bits_from_block_10r_ID2;
        break;
    default:
        printf("undefined distinguishing setting!!!\n");
        exit(0);
    }

    uint64_t input_space = 1 << input_bits;
    block *c0 = new block[input_space], *c1 = new block[input_space];
    for (uint64_t i = 0; i < input_space; i++) lookup_table[i] = 0;
    for (uint64_t i = 0; i < average_num; i++) {
        make_encryption_data(input_space, num_rounds, diff, c0, c1);
        for (uint64_t j = 0; j < input_space; j++) ++lookup_table[(*extract_bits_from_block)(c0[j], c1[j])];
    }
    delete[] c0; delete[] c1;
}

void test_distinguishing_acc(const uint32_t& n, const uint32_t& num_rounds, const block& diff, const uint64_t lookup_table[], const uint32_t& input_bits, const uint32_t& dis_setting) {
    uint64_t (*extract_bits_from_block)(const block&, const block&);
    switch (dis_setting) {
    case ID1_9R:
        extract_bits_from_block = extract_bits_from_block_9r_ID1;
        break;
    case ID2_9R:
        extract_bits_from_block = extract_bits_from_block_9r_ID2;
        break;
    case ID2_10R:
        extract_bits_from_block = extract_bits_from_block_10r_ID2;
        break;
    default:
        printf("undefined distinguishing setting!!!\n");
        exit(0);
    }

    uint64_t input_space = 1 << input_bits;
    uint64_t sample_num = 0;
    uint64_t average_num;
    for (uint64_t i = 0; i < input_space; i++) sample_num += lookup_table[i];
    average_num = sample_num / input_space;
    printf("average num in log2: %f.\n", log2(average_num));
    block *c0 = new block[n], *c1 = new block[n];
    bool *Y = new bool[n];
    make_test_set(n, num_rounds, diff, c0, c1, Y);
    uint32_t num_p = 0, num_n = 0, num_true_p = 0, num_true_n = 0;
    bool prediction;
    for (uint32_t i = 0; i < n; i++) {
        if (lookup_table[(*extract_bits_from_block)(c0[i], c1[i])] < average_num) prediction = false;
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

int main() {
    bool check_res = check_testvector();
    printf("check testvector res is %d.\n", check_res);
    random_generator.set_rand_seed(time(nullptr));

    // ID1_9R
    block diff = {0x80ull, 0};
    uint32_t num_rounds = 9;
    uint32_t input_bits = 25;
    uint32_t average_num_in_bits = 9;
    uint32_t dis_setting = ID1_9R;
    string dis_tag = "ID1_9R";

    // ID2_9R
    // block diff = {0x80ull, 0x8000000000000000ull};
    // uint32_t num_rounds = 9;
    // uint32_t input_bits = 27;
    // uint32_t average_num_in_bits = 7;
    // uint32_t dis_setting = ID2_9R;
    // string dis_tag = "ID2_9R";

    // ID2_10R
    // block diff = {0x80ull, 0x8000000000000000ull};
    // uint32_t num_rounds = 10;
    // uint32_t input_bits = 26;
    // uint32_t average_num_in_bits = 6;
    // uint32_t dis_setting = ID2_10R;
    // string dis_tag = "ID2_10R";

    uint64_t average_num = 1 << average_num_in_bits;
    string table_path = "./lookup_table/" + to_string(num_rounds) + "r_table_" + to_string(input_bits) + "_" + to_string(average_num_in_bits) + "_" + dis_tag;
    uint64_t input_space = 1 << input_bits;
    uint64_t *lookup_table = new uint64_t[input_space];

    // Build a counter lookup table
    printf("Building lookup table...\n");
    clock_t start = clock();
    build_counter_table(num_rounds, diff, lookup_table, input_bits, average_num, dis_setting);
    clock_t end = clock();
    printf("Building time: %f s.\n", (end - start + 0.0) / CLOCKS_PER_SEC);

    // If the table already exists, just load it using the following code
    // FILE *input_file = fopen(table_path.c_str(), "rb");
    // fread((void *)lookup_table, sizeof(uint64_t), input_space, input_file);
    // fclose(input_file);

    // Test distinguisher accuracy
    test_distinguishing_acc(1<<24, num_rounds, diff, lookup_table, input_bits, dis_setting);

    // Save the lookup table
    printf("Saved lookup file path: %s\n", table_path.c_str());
    FILE *output_file = fopen(table_path.c_str(), "wb");
    fwrite((const void *)lookup_table, sizeof(uint64_t), input_space, output_file);
    fclose(output_file);
    delete[] lookup_table;
    return 0;
}