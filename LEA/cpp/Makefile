all : build_lookup_table

build_lookup_table : build_lookup_table.cpp lea.cpp rand_gen.cpp util.cpp
	g++ -O2 -std=c++11 -o $@ $^