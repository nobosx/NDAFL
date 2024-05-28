# Revisiting BST: Back from Deep Learning to Classical Cryptanalysis

This repository contains the supplementary codes of the paper *Revisiting DST: Back from Deep Learning to Classical Cryptanalysis*. It includes the applications of the framework Neural Distinguisher-Aided Feature Location to: Speck32, Speck48, Speck64, Speck96, Speck 128, Simon32, Simon 64, Simon 128 and LEA. The codes are mainly written in C++ and Python.

In the folder `Speck32`:

* Neural distinguishers and auxiliary distinguishers for 5-round to 8-round Speck32 are stored in the folder `saved_models`. These nets can be trained in `train_nets.py`. Their performances on test sets can be evaluated in `eval_nets.py`.
* Bit sensitivity test and bit reduction test are implemented in `bst.py`. The results of bit sensitivity tests are stored in the folder `bst_res`.
* The C++ code of building and evaluating a counter lookup table for Speck32 is in `cpp/build_lookup_table.cpp`. The resulting lookup table will be stored in the folder `cpp/lookup_table`. We only provide our 8-round lookup table $\mathcal{CD^{8rSpeck32}_{2^{18}}}$ for the consideration of the size limit.
* The C++ code of get the wrong key profile is in `cpp/gen_wrong_key_profile.cpp`. The resulting wrong key profile tables are provided in the folder `cpp/wrong_key_profile`.
* Our implementation of 13-round attack on Speck32/64 is provided in `cpp/improved_13_round_attack.cpp`. Before running the attack, a 7-round counter lookup table and its wrong key profile should be built first.
* One can run the `make` command in the `cpp` folder to get all the executable files of the C++ codes. On getting the executable file `build_lookup_table`, one can run the command `./build_lookup_table [nr] [log2(alpha)]` to build a counter lookup table for nr rounds and with the average table entry alpha, e.g. `./build_lookup_table 7 10` to build $\mathcal{CD}^{7rSpeck32}_{2^{10}}$.

The structure in the folder Speck48, Speck64, Speck96 and Speck128 is basically the same. For example, in the folder `Speck48`:

* Neural distinguishers and the neural differential distinguishers as baseline on Speck48 can be trained in `train_nets.py` and evaluated in `eval_nets.py`. These distinguishers are stored in the folder `saved_models` and the file name of a neural differential distinguisher has the suffix `only_diff`. 
* Bit sensitivity test is implemented in `bst.py`. The results are stored in the folder `bst_res`.
* The C++ code of building and evaluating a counter lookup table for Speck48 is in `cpp/build_lookup_table.cpp`. The resulting lookup table will be stored in the folder `cpp/lookup_table`.

