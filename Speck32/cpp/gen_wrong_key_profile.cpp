#include "speck.h"
#include "util.h"
#include <math.h>
#include <string>
#include <fstream>
#include <vector>
#include <time.h>
using namespace std;

void evaluate_wrong_key_profile(const uint32_t& n, const block& dis_diff, const uint32_t& dis_nr, const double lookup_table[], const word& kg_delta, double& mu, double& sigma) {
    double *dis_response = new double[n];
    block p0, p1, c0, c1;
    word ks[50];
    word mk[M];
    for (uint32_t try_index = 0; try_index < n; try_index++) {
        for (int i = 0; i < M; i++) mk[i] = RAND_WORD;
        expand_key(mk, ks, dis_nr+1);
        p0 = {RAND_WORD, RAND_WORD};
        p1.first = p0.first ^ dis_diff.first;
        p1.second = p0.second ^ dis_diff.second;
        encrypt(p0, ks, dis_nr+1, c0);
        encrypt(p1, ks, dis_nr+1, c1);
        dec_one_round(c0, ks[dis_nr] ^ kg_delta, p0);
        dec_one_round(c1, ks[dis_nr] ^ kg_delta, p1);
        if (dis_nr == 8) dis_response[try_index] = lookup_table[extract_bits_from_block_8r(p0, p1)];
        else dis_response[try_index] = lookup_table[extract_bits_from_block_7r(p0, p1)];
    }
    cal_mean_std(dis_response, n, mu, sigma);
    delete[] dis_response;
}

int main() {
    // Set random seed
    random_generator.set_rand_seed(time(NULL));
    block dis_diff = {0x40, 0};
    uint32_t dis_nr = 7;
    string table_path("lookup_table/7r_table_24_10_7R");
    vector<uint32_t> selected_bits = {44,43,42,41,37,36,35,34,28,27,26,25,24,21,20,19,18,17,11,10,9,8,2,1}; // IBs for 7r ND

    // uint32_t dis_nr = 8;
    // string table_path("lookup_table/8_round_table_22_18_backup");
    // vector<uint32_t> selected_bits = {44,43,42,37,36,35,34,33,28,27,26,21,20,19,18,17,11,10,4,3,2,1}; // IBs for 8r ND

    clock_t start, end;
    uint32_t structure_size = 1 << 12, kg_space = 1 << 16, n = 1 << 18;
    
    ifstream fin(table_path, ios::in | ios::binary);
    uint64_t input_space = pow(2, selected_bits.size());

    uint64_t *lookup_table = new uint64_t[input_space];
    fin.read((char *)lookup_table, input_space * sizeof(uint64_t));
    fin.close();
    double *transed_lookup_table = new double[input_space];
    trans_lookup_table_output(lookup_table, transed_lookup_table, selected_bits);

    double *mu = new double[kg_space], *sigma = new double[kg_space];
    start = clock();
    for (uint32_t kg_delta = 0; kg_delta < kg_space; kg_delta++) {
        // printf("\rkg_delta: %d", kg_delta);
        // fflush(stdout);
        evaluate_wrong_key_profile(n, dis_diff, dis_nr, transed_lookup_table, kg_delta, mu[kg_delta], sigma[kg_delta]);
        sigma[kg_delta] /= sqrt(structure_size);
    }
    end = clock();
    printf("Running time: %f s\n", (end - start + 0.0) / CLOCKS_PER_SEC);
    
    // Save wrong key profile
    string saved_folder = "./wrong_key_profile/";
    
    // Write mean
    ofstream fout(saved_folder + "7r_mean", ios::out | ios::binary);
    // ofstream fout(saved_folder + "8r_mean", ios::out | ios::binary);
    fout.write((const char *)mu, kg_space * sizeof(double));
    fout.close();

    // Write std var
    fout.open(saved_folder + "7r_std_var_12", ios::out | ios::binary);
    // fout.open(saved_folder + "8r_std_var_12", ios::out | ios::binary);
    fout.write((const char *)sigma, kg_space * sizeof(double));
    fout.close();
    delete[] mu, delete[] sigma, delete[] lookup_table; delete[] transed_lookup_table;
}