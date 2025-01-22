# General enhancing framework for deep learning-aided cryptanalysis: applications to Speck, Simon and LEA ciphers

This repository contains the supplementary codes of the paper *General enhancing framework for deep learning-aided cryptanalysis: applications to Speck, Simon and LEA ciphers*. It includes the applications of the framework Neural Distinguisher-Aided Feature Location (NDAFL) to: Speck32, Speck48, Speck64, Speck96, Speck 128, Simon32, Simon64, Simon128 and LEA. The codes are mainly written in C++ and Python.

In the folder `Speck32`:

* Neural distinguishers for 5-round to 8-round Speck32 are stored in the folder `saved_models`. `h5` files with the `l_r` suffix are the neural distinguishers with input form $I = C_L || C_R || C'_L || C'_R$, while the `dl_l_dy_y` suffix represents those neural distinguishers with input form $\Delta C_L || C_L || \Delta C_Y || C_Y$. These nets can be trained in `train_nets.py`. Their performances on test sets can be evaluated in `eval_nets.py`.
* Bit sensitivity test is implemented in `bst.py`. The results of bit sensitivity tests are stored in the folder `bst_res`.
* Comparison of accuracy and processing time between neural distinguishers and our lookup table distinguishers is implemented in `compare_distinguishers.py`.
* The C++ code of building and evaluating a counter lookup table for Speck32 is in `cpp/build_lookup_table.cpp`. The resulting lookup table will be stored in the folder `cpp/lookup_table`. We only provide our 8-round lookup table $\mathcal{LT^{22}_{2^{18}}}$ for the consideration of the size limit.
* The C++ code of get the wrong key profile is in `cpp/gen_wrong_key_profile.cpp`. The resulting wrong key profile tables are provided in the folder `cpp/wrong_key_profile`.
* Our implementation of 13-round attack on Speck32/64 is provided in `cpp/improved_13_round_attack.cpp`. Before running the attack, a 7-round counter lookup table and its wrong key profile should be built first.
* One can run the `make` command in the `cpp` folder to get all the executable files of the C++ codes. On getting the executable file `build_lookup_table`, one can run the command `./build_lookup_table [nr] [log2(alpha)]` to build a counter lookup table for nr rounds and with the average table entry alpha, e.g. `./build_lookup_table 7 10` to build 7-round $\mathcal{LT}^{24}_{2^{10}}$​.
* We have run the 13-round attack for 100 times, and the attacking log is in `./cpp/attack_record.txt`.

The folders `Speck48`, `Speck64`, `Speck96`, `Speck128` and `LEA`  contain the codes for Speck48, Speck64, Speck96, Speck128 and LEA respectively. The structure of these folders is basically the same. For example, in the folder `Speck48`:

* Neural distinguishers and the neural differential distinguishers as baseline on Speck48 can be trained in `train_nets.py` and evaluated in `eval_nets.py`. These distinguishers are stored in the folder `saved_models` and the file name of a neural differential distinguisher has the suffix `only_diff`. 
* Bit sensitivity test is implemented in `bst.py`. The results are stored in the folder `bst_res`.
* The C++ code of building and evaluating a counter lookup table for Speck48 is in `cpp/build_lookup_table.cpp`. One can run `./build_lookup_table [choice]` to build a corresponding lookup table distinguisher, where the option `[choice]` is an integer. The resulting lookup tables will be stored in the folder `cpp/lookup_table`.

Specially, in the folder `Speck96/cpp`, C++ code of the 14-round attack on Speck96 is provided in `key_recovery_attack.cpp`. One can change the value of `ATTACK_THREAD_NUM` to set the number of threads used to run the program and change the value of  `n` to set the total number of attack trials. To execute the key recovery attack experiment, one can run the following commands :

```shell
make
./build_lookup_table 5
./build_lookup_table 6
./build_lookup_table 7
./build_lookup_table 8
./key_recovery_attack
```

Similarly, in the folder `Speck128/cpp`, C++ code of the 14-round attack on Speck128 is provided in `key_recovery_attack.cpp`. One can change the value of `ATTACK_THREAD_NUM` to set the number of threads used to run the program and change the value of  `n` to set the total number of attack trials. To execute the key recovery attack experiment, one can run the following commands:

```shell
make
./build_lookup_table 4
./build_lookup_table 5
./build_lookup_table 6
./build_lookup_table 7
./build_lookup_table 8
./key_recovery_attack
```

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
