#include "speck.h"
#include "util.h"
#include "time.h"
#include <vector>
#include <math.h>
#include <string>
#include <fstream>
#include <assert.h>

#define DIS_SETTING_5R 1
#define DIS_SETTING_6R 2
#define DIS_SETTING_7R 3
#define DIS_SETTING_8R 4

void build_counter_table(const uint32_t& num_rounds, const block& diff, uint64_t lookup_table[], const uint64_t& input_bits, const uint64_t& average_num, const uint32_t& dis_setting) {
    uint64_t (*extract_bits_from_block)(const block&, const block&);
    switch (dis_setting)
    {
        case DIS_SETTING_5R:
            extract_bits_from_block = extract_bits_from_block_5r;
            break;
        case DIS_SETTING_6R:
            extract_bits_from_block = extract_bits_from_block_6r;
            break;
        case DIS_SETTING_7R:
            extract_bits_from_block = extract_bits_from_block_7r;
            break;
        case DIS_SETTING_8R:
            extract_bits_from_block = extract_bits_from_block_8r;
            break;
        default:
            printf("undefined distinguishing setting!!!\n");
            exit(0);
    }

    uint64_t input_space = 1 << input_bits;
    block *c0 = new block[input_space], *c1 = new block[input_space];
    for (uint64_t i = 0; i < input_space; i++) lookup_table[i] = 0;
    for (uint64_t i = 0; i < average_num; i++) {
        make_target_diff_samples(input_space, diff, num_rounds, c0, c1, true);
        for (uint64_t j = 0; j < input_space; j++) ++lookup_table[(*extract_bits_from_block)(c0[j], c1[j])];
    }
    delete[] c0; delete[] c1;
}

void test_distinguisher_acc(const uint32_t& n, const uint32_t& num_rounds, const block& diff, uint64_t lookup_table[], const uint64_t& input_bits, const uint32_t& dis_setting) {
    uint64_t (*extract_bits_from_block)(const block&, const block&);
    switch (dis_setting)
    {
        case DIS_SETTING_5R:
            extract_bits_from_block = extract_bits_from_block_5r;
            break;
        case DIS_SETTING_6R:
            extract_bits_from_block = extract_bits_from_block_6r;
            break;
        case DIS_SETTING_7R:
            extract_bits_from_block = extract_bits_from_block_7r;
            break;
        case DIS_SETTING_8R:
            extract_bits_from_block = extract_bits_from_block_8r;
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
    block *c0, *c1;
    bool *Y;
    c0 = new block[n], c1 = new block[n], Y = new bool[n];
    make_test_set(n, diff, num_rounds, c0, c1, Y);
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

// Run the program as ./main [numer_rounds] [average_num_log2]
int main(int argc, char *argv[]) {
    assert(argc == 3);
    bool check_res = check_testvector();
    printf("check testvector res is %d.\n", check_res);
    random_generator.set_rand_seed(time(0));
    block diff = {0x40, 0};
    uint32_t num_rounds, dis_setting;
    uint64_t input_bits, average_num_in_bits;
    string dis_tag;

    int choice = atoi(argv[1]);
    average_num_in_bits = atoi(argv[2]);
    switch (choice)
    {
    case 5:
        // 5R
        num_rounds = 5;
        input_bits = 26;
        dis_setting = DIS_SETTING_5R;
        dis_tag = "5R";
        break;
    case 6:
        // 6R
        num_rounds = 6;
        input_bits = 25;
        dis_setting = DIS_SETTING_6R;
        dis_tag = "6R";
        break;
    case 7:
        // 7R
        num_rounds = 7;
        input_bits = 24;
        dis_setting = DIS_SETTING_7R;
        dis_tag = "7R";
        break;
    case 8:
        // 8R
        num_rounds = 8;
        input_bits = 22;
        dis_setting = DIS_SETTING_8R;
        dis_tag = "8R";
        break;
    default:
        exit(0);
    }

    uint64_t average_num = 1 << average_num_in_bits;
    string table_path = "./lookup_table/" + to_string(num_rounds) + "r_table_" + to_string(input_bits) + "_" + to_string(average_num_in_bits) + "_" + dis_tag;
    uint64_t input_space = 1 << input_bits;
    uint64_t *lookup_table = new uint64_t[input_space];

    // If the table already exists, just load it using the following code
    // printf("Building lookup table %s...\n", table_path.c_str());
    // clock_t start = clock();
    // build_counter_table(num_rounds, diff, lookup_table, input_bits, average_num, dis_setting);
    // clock_t end = clock();
    // printf("Building time: %f s.\n", (end - start + 0.0) / CLOCKS_PER_SEC);

    // Build a counter lookup table
    printf("Loading lookup table %s...\n", table_path.c_str());
    FILE *input_file = fopen(table_path.c_str(), "rb");
    fread((void *)lookup_table, sizeof(uint64_t), input_space, input_file);
    fclose(input_file);

    // Test distinguisher accuracy
    test_distinguisher_acc(1e7, num_rounds, diff, lookup_table, input_bits, dis_setting);

    // Save the lookup table
    FILE *output_file = fopen(table_path.c_str(), "wb");
    fwrite((const void *)lookup_table, sizeof(uint64_t), input_space, output_file);
    fclose(output_file);
    delete[] lookup_table;
    return 0;
}