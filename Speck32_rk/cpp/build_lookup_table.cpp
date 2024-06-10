#include "speck.h"
#include "util.h"
#include "rand_gen.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <math.h>
using namespace std;

void build_conuter_table(const uint32_t& num_rounds, const block& diff, const word mk_diff[], const word rk_diff_trail[], uint64_t lookup_table[], const uint32_t& input_bits, const uint64_t& average_num, const uint32_t& rk_diff_setting, const uint32_t key_repeat_num=(1<<10)) {
    uint64_t (*extract_bits_from_block)(const block&, const block&);
    switch (rk_diff_setting)
    {
    case ID2_9R:
        extract_bits_from_block = extract_bits_from_block_rk_9r_ID2;
        break;
    case ID2_10R:
        extract_bits_from_block = extract_bits_from_block_rk_10r_ID2;
        break;
    case ID3_9R:
        extract_bits_from_block = extract_bits_from_block_rk_9r_ID3;
        break;
    case ID3_10R:
        extract_bits_from_block = extract_bits_from_block_rk_10r_ID3;
        break;
    default:
        printf("undefined rk setting!!!\n");
        exit(0);
    }

    uint64_t input_space = 1 << input_bits;
    block *c0 = new block[input_space], *c1 = new block[input_space];
    for (uint64_t i = 0; i < input_space; i++) lookup_table[i] = 0;
    for (uint64_t i = 0; i < average_num; i++) {
        make_encryption_data_rk(input_space, num_rounds, diff, mk_diff, rk_diff_trail, c0, c1, key_repeat_num);
        for (uint64_t j = 0; j < input_space; j++) {
            ++lookup_table[(*extract_bits_from_block)(c0[j], c1[j])];
        }
    }
    delete[] c0; delete[] c1;
}

void test_distinguisher_acc(const uint32_t& n, const uint32_t& num_rounds, const block& diff, const word mk_diff[], const word rk_diff_trail[], uint64_t lookup_table[], const uint32_t& input_bits, const uint32_t& rk_diff_setting, const uint32_t& key_repeat_num=(1<<10)) {
    uint64_t (*extract_bits_from_block)(const block&, const block&);
    switch (rk_diff_setting)
    {
    case ID2_9R:
        extract_bits_from_block = extract_bits_from_block_rk_9r_ID2;
        break;
    case ID2_10R:
        extract_bits_from_block = extract_bits_from_block_rk_10r_ID2;
        break;
    case ID3_9R:
        extract_bits_from_block = extract_bits_from_block_rk_9r_ID3;
        break;
    case ID3_10R:
        extract_bits_from_block = extract_bits_from_block_rk_10r_ID3;
        break;
    default:
        printf("undefined rk setting!!!\n");
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
    make_test_set_rk(n, num_rounds, diff, mk_diff, rk_diff_trail, c0, c1, Y, key_repeat_num);
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
    delete[] c0; delete[] c1; delete[] Y;
}

int main() {
    random_generator.set_rand_seed(time(0));
    block diff = {0x0000, 0x0000};

    // ID2_9R
    word mk_diff[] = {0x0040, 0x0000, 0x0000, 0x0000};
    word rk_diff_trail[] = {0x0000, 0x0000, 0x0000, 0x8000, 0x8002, 0x8008, 0x812a, 0x8480, 0x9382, 0xcf8a};
    uint32_t input_bits = 27;
    uint32_t rk_diff_setting = ID2_9R;
    uint32_t average_num_in_bits = 7;
    uint32_t num_rounds = 9;
    string dis_tag = "_ID2";

    // ID2_10R
    // word mk_diff[] = {0x0040, 0x0000, 0x0000, 0x0000};
    // word rk_diff_trail[] = {0x0000, 0x0000, 0x0000, 0x8000, 0x8002, 0x8008, 0x812a, 0x8480, 0x9382, 0xcf8a};
    // uint32_t input_bits = 21;
    // uint32_t rk_diff_setting = ID2_10R;
    // uint32_t average_num_in_bits = 13;
    // uint32_t num_rounds = 10;
    // string dis_tag = "_ID2";

    // ID3_9R
    // word mk_diff[] = {0x0040, 0x0000, 0x0000, 0x0000};
    // word rk_diff_trail[] = {0x0000, 0x0000, 0x0000, 0x8000, 0x8002, 0x8008, 0x812a, 0x8480, 0x9082, 0xc28a};
    // uint32_t num_rounds = 9;
    // uint32_t input_bits = 29;
    // uint32_t rk_diff_setting = ID3_9R;
    // uint32_t average_num_in_bits = 7;
    // string dis_tag = "_ID3";

    // ID3_10R
    // word mk_diff[] = {0x0040, 0x0000, 0x0000, 0x0000};
    // word rk_diff_trail[] = {0x0000, 0x0000, 0x0000, 0x8000, 0x8002, 0x8008, 0x812a, 0x8480, 0x9082, 0xc28a};
    // uint32_t num_rounds = 10;
    // uint32_t input_bits = 21;
    // uint32_t rk_diff_setting = ID3_10R;
    // uint32_t average_num_in_bits = 13;
    // string dis_tag = "_ID3";
    
    uint64_t average_num = 1 << average_num_in_bits;
    string table_path = "./lookup_table/" + to_string(num_rounds) + "r_table_" + to_string(input_bits) + "_" + to_string(average_num_in_bits) + dis_tag;
    uint32_t key_repeat_num = (num_rounds == 10) ? (1<<14) : (1<<10);

    // Build a counter lookup table
    uint64_t input_space = 1 << input_bits;
    uint64_t *lookup_table = new uint64_t[input_space];
    printf("Building lookup table...\n");
    clock_t start = clock();
    build_conuter_table(num_rounds, diff, mk_diff, rk_diff_trail, lookup_table, input_bits, average_num, rk_diff_setting, key_repeat_num);
    clock_t end = clock();
    printf("Building time: %f s.\n", (end - start + 0.0) / CLOCKS_PER_SEC);

    // If the table already exists, just load it using the following code
    // FILE *input_file = fopen(table_path.c_str(), "rb");
    // fread((void *)lookup_table, sizeof(uint64_t), input_space, input_file);
    // fclose(input_file);

    // Test distinguisher accuracy
    test_distinguisher_acc(1<<24, num_rounds, diff, mk_diff, rk_diff_trail, lookup_table, input_bits, rk_diff_setting, key_repeat_num);

    // Save the lookup table
    printf("Saved lookup file path: %s\n", table_path.c_str());
    FILE *output_file = fopen(table_path.c_str(), "wb");
    fwrite((const void *)lookup_table, sizeof(uint64_t), input_space, output_file);
    fclose(output_file);
    delete[] lookup_table;
}