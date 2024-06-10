all : build_lookup_table

build_lookup_table : build_lookup_table.cpp speck.cpp util.cpp rand_gen.cpp
	g++ -O2 -std=c++11 -o $@ $^