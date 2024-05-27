all : build_lookup_table improved_13_round_attack gen_wrong_key_profile

build_lookup_table : speck.cpp util.cpp rand_gen.cpp build_lookup_table.cpp
	g++ -O2 -std=c++11 -o $@ $^

improved_13_round_attack : speck.cpp util.cpp rand_gen.cpp improved_13_round_attack.cpp
	g++ -O2 -std=c++11 -o $@ $^

gen_wrong_key_profile : speck.cpp util.cpp rand_gen.cpp gen_wrong_key_profile.cpp
	g++ -O2 -std=c++11 -o $@ $^