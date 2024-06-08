#include "simon.h"
#include "util.h"
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <string>
using namespace std;

// define dis_settings
#define SIMON32_9R 1
#define SIMON32_10R 2
#define SIMON32_11R 3
#define SIMON64_12R 4
#define SIMON64_13R 5
#define SIMON128_19R 6

void build_counter_table(const uint32_t& num_rounds, const block& diff, uint64_t lookup_table[], const uint32_t& input_bits, const uint64_t& average_num, const uint32_t& dis_setting) {
    uint64_t (*extract_bits_from_block)(const block&, const block&);
    switch (dis_setting) {
    case SIMON32_9R:
        extract_bits_from_block = extract_bits_from_block_simon32_9r;
        break;
    case SIMON32_10R:
        extract_bits_from_block = extract_bits_from_block_simon32_10r;
        break;
    case SIMON32_11R:
        extract_bits_from_block = extract_bits_from_block_simon32_11r;
        break;
    case SIMON64_12R:
        extract_bits_from_block = extract_bits_from_block_simon64_12r;
        break;
    case SIMON64_13R:
        extract_bits_from_block = extract_bits_from_block_simon64_13r;
        break;
    case SIMON128_19R:
        extract_bits_from_block = extract_bits_from_block_simon128_19r;
        break;
    default:
        printf("undefined distinguishing setting!!!\n");
        exit(0);
    }

    uint64_t input_space = 1 << input_bits;
    block *c0 = new block[input_space], *c1 = new block[input_space];
    for (uint64_t i = 0; i < input_space; i++) lookup_table[i] = 0;
    for (uint64_t i = 0; i < average_num; i++) {
        make_encryption_data(input_space, num_rounds, diff, c0, c1, true);
        for (uint64_t j = 0; j < input_space; j++) ++lookup_table[(*extract_bits_from_block)(c0[j], c1[j])];
    }
    delete[] c0; delete[] c1;
}

void test_distinguishing_acc(const uint32_t& n, const uint32_t& num_rounds, const block& diff, const uint64_t lookup_table[], const uint32_t& input_bits, const uint32_t& dis_setting) {
    uint64_t (*extract_bits_from_block)(const block&, const block&);
    switch (dis_setting) {
    case SIMON32_9R:
        extract_bits_from_block = extract_bits_from_block_simon32_9r;
        break;
    case SIMON32_10R:
        extract_bits_from_block = extract_bits_from_block_simon32_10r;
        break;
    case SIMON32_11R:
        extract_bits_from_block = extract_bits_from_block_simon32_11r;
        break;
    case SIMON64_12R:
        extract_bits_from_block = extract_bits_from_block_simon64_12r;
        break;
    case SIMON64_13R:
        extract_bits_from_block = extract_bits_from_block_simon64_13r;
        break;
    case SIMON128_19R:
        extract_bits_from_block = extract_bits_from_block_simon128_19r;
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
    make_test_set(n, num_rounds, diff, c0, c1, Y, true);
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
    if (!check_testvector()) return 0;
    random_generator.set_rand_seed(time(0));

    // -----------------------------Simon32/64------------------------------
    #ifdef SIMON32_64

    block diff = {0, 0x40u};
    string table_folder = "./lookup_table/Simon32/";

    // 9R
    // uint32_t num_rounds = 9;
    // uint32_t input_bits = 25;
    // uint32_t average_num_in_bits = 9;
    // uint32_t dis_setting = SIMON32_9R;
    // string dis_tag = "SIMON32_9R";

    // 10R
    // uint32_t num_rounds = 10;
    // uint32_t input_bits = 23;
    // uint32_t average_num_in_bits = 11;
    // uint32_t dis_setting = SIMON32_10R;
    // string dis_tag = "SIMON32_10R";

    // 11R
    uint32_t num_rounds = 11;
    uint32_t input_bits = 19;
    uint32_t average_num_in_bits = 15;
    uint32_t dis_setting = SIMON32_11R;
    string dis_tag = "SIMON32_11R";

    #endif
    // ----------------------------------------------------------------------

    // -----------------------------simon64/128------------------------------
    #ifdef SIMON64_128

    block diff = {0, 0x1u};
    string table_folder = "./lookup_table/Simon64/";

    // 12R
    uint32_t num_rounds = 12;
    uint32_t input_bits = 25;
    uint32_t average_num_in_bits = 9;
    uint32_t dis_setting = SIMON64_12R;
    string dis_tag = "SIMON64_12R";

    // 13R
    // uint32_t num_rounds = 13;
    // uint32_t input_bits = 18;
    // uint32_t average_num_in_bits = 12;
    // uint32_t dis_setting = SIMON64_13R;
    // string dis_tag = "SIMON64_13R";

    #endif
    // ----------------------------------------------------------------------

    // -----------------------------simon128/128------------------------------
    #ifdef SIMON128_128

    block diff = {0, 0x1ull};
    string table_folder = "./lookup_table/Simon128/";

    // 19R
    uint32_t num_rounds = 19;
    uint32_t input_bits = 19;
    uint32_t average_num_in_bits = 11;
    uint32_t dis_setting = SIMON128_19R;
    string dis_tag = "SIMON128_19R";

    #endif
    // -----------------------------------------------------------------------

    uint64_t average_num = 1 << average_num_in_bits;
    string table_path = table_folder + to_string(num_rounds) + "r_table_" + to_string(input_bits) + "_" + to_string(average_num_in_bits) + "_" + dis_tag;
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