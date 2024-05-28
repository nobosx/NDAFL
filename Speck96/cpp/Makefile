all : main

main : build_lookup_table.cpp speck.cpp rand_gen.cpp util.cpp speck.h rand_gen.h util.h
	g++ -O2 -std=c++11 -o $@ $^

# test_diff_linear_cor : test_diff_linear_cor.cpp speck.cpp rand_gen.cpp util.cpp speck.h rand_gen.h util.h
# 	g++ -O2 -std=c++11 -o $@ $^