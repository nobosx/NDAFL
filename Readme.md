# Revisiting BST: Back from Deep Learning to Classical Cryptanalysis

This repository contains the supplementary codes of the paper *Revisiting BST: Back from Deep Learning to Classical Cryptanalysis*. It includes the applications of the framework Neural Distinguisher-Aided Feature Location (NDAFL) to: Speck32, Speck32 under related-key setting, Speck48, Speck64, Speck96, Speck 128, Simon and LEA. The codes are mainly written in C++ and Python.

In the folder `Speck32`:

* Neural distinguishers and auxiliary distinguishers for 5-round to 8-round Speck32 are stored in the folder `saved_models`. These nets can be trained in `train_nets.py`. Their performances on test sets can be evaluated in `eval_nets.py`.
* Bit sensitivity test and bit reduction test are implemented in `bst.py`. The results of bit sensitivity tests are stored in the folder `bst_res`.
* The C++ code of building and evaluating a counter lookup table for Speck32 is in `cpp/build_lookup_table.cpp`. The resulting lookup table will be stored in the folder `cpp/lookup_table`. We only provide our 8-round lookup table $\mathcal{CD^{8rSpeck32}_{2^{18}}}$ for the consideration of the size limit.
* The C++ code of get the wrong key profile is in `cpp/gen_wrong_key_profile.cpp`. The resulting wrong key profile tables are provided in the folder `cpp/wrong_key_profile`.
* Our implementation of 13-round attack on Speck32/64 is provided in `cpp/improved_13_round_attack.cpp`. Before running the attack, a 7-round counter lookup table and its wrong key profile should be built first.
* One can run the `make` command in the `cpp` folder to get all the executable files of the C++ codes. On getting the executable file `build_lookup_table`, one can run the command `./build_lookup_table [nr] [log2(alpha)]` to build a counter lookup table for nr rounds and with the average table entry alpha, e.g. `./build_lookup_table 7 10` to build $\mathcal{CD}^{7rSpeck32}_{2^{10}}$​.
* We have run the 13-round attack for 100 times, and the attacking log is in `./cpp/attack_record.txt`.

The folders `Speck32_rk`, `Speck48`, `Speck64`, `Speck96`, `Speck128` and `LEA`  contain the codes for Speck32 under related-key setting, Speck48, Speck64, Speck96, Speck128 and LEA respectively. The structure of these folders is basically the same. For example, in the folder `Speck48`:

* Neural distinguishers and the neural differential distinguishers as baseline on Speck48 can be trained in `train_nets.py` and evaluated in `eval_nets.py`. These distinguishers are stored in the folder `saved_models` and the file name of a neural differential distinguisher has the suffix `only_diff`. 
* Bit sensitivity test is implemented in `bst.py`. The results are stored in the folder `bst_res`.
* The C++ code of building and evaluating a counter lookup table for Speck48 is in `cpp/build_lookup_table.cpp`. The resulting lookup tables will be stored in the folder `cpp/lookup_table`.

In the folder `Simon`:

* Neural distinguishers of Simon32, Simon64 and Simon128 can be obtained by running `train_nets.py` and evaluated by running `eval_nets.py`. These neural distinguishers are provided in the folder `saved_models`.
* Bit sensitivity test is implemented in `bst.py` and the results are provided in the folder `bst_res`.
* The C++ code of building and evaluating a counter lookup table for Simon is in `cpp/build_lookup_table.cpp`. The resulting lookup tables will be stored in the folder `cpp/lookup_table`. To build lookup tables for different Simon instances, one need to choose the version of Simon instance in `cpp/simon.h` (defined as `SIMON32_64`, `SIMON64_128` and `SIMON128_128` respectively) first. 

## Tested software environment

Python:

* python == 3.8.10
* tensorflow == 2.4.0
* keras == 2.4.3
* h5py == 2.10.0
* numpy == 1.19.5

For the C++ code, we used g++ compiler to get an executable file.

## References

* Our Python codes of the neural network and Speck32 implementations are based on the GitHub repository [https://github.com/agohr/deep_speck.git](https://github.com/agohr/deep_speck.git) which provides the supplementary codes of the paper [1].
* Our Python code of the Simon implementation is based on the GitHub repository [https://github.com/differential-neural/An-Assessment-of-Differential-Neural-Distinguishers.git](https://github.com/differential-neural/An-Assessment-of-Differential-Neural-Distinguishers.git) which provides the supplementary codes of the paper [2].
* Our Python codes of Speck32_rk and its corresponding neural network are based on the repository [https://www.dropbox.com/sh/yleufeiu0wqwcjv/AADUpM15q86Uk1lM8z99fU2ia?dl=0](https://www.dropbox.com/sh/yleufeiu0wqwcjv/AADUpM15q86Uk1lM8z99fU2ia?dl=0) which provides the source codes of the paper [3].

[1] Gohr, A.: Improving attacks on round-reduced speck32/64 using deep learning. In: Boldyreva, A., Micciancio, D. (eds.) Advances in Cryptology – CRYPTO 2019. pp. 150–179. Springer International Publishing, Cham (2019) https://doi.org/10.1007/978-3-030-26951-7_6

[2] Gohr, A., Leander, G., Neumann, P.: An assessment of differential-neural distinguishers. Cryptology ePrint Archive, Paper 2022/1521 (2022) https://eprint.iacr.org/2022/1521

[3] Bao, Z., Lu, J., Yao, Y., Zhang, L.: More insight on deep learning-aided cryptanalysis. In: Guo, J., Steinfeld, R. (eds.) Advances in Cryptology – ASIACRYPT 2023. pp. 436–467. Springer Nature Singapore, Singapore (2023) https://doi.org/10.1007/978-981-99-8727-6_15
