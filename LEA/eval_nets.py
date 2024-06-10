import net

data_format = "dx_x"
calc_back = True
diff_unit = 0x80000000
diff = [diff_unit for _ in range(4)]

# Test neural distinguishers NDs
for num_rounds in [9,10,11]:
    print("Testing {}r ND...".format(num_rounds))
    model_path = "./saved_models/{}r_distinguisher_{}.h5".format(num_rounds, data_format)
    net.eval_distinguisher(2**24, num_rounds, diff, model_path, data_format, calc_back)

# Test neural differential distinguishers NDDs
for num_rounds in [9,10,11]:
    print("Testing {}r ND...".format(num_rounds))
    model_path = "./saved_models/{}r_distinguisher_only_diff.h5".format(num_rounds)
    net.eval_distinguisher_with_only_diff(2**24, num_rounds, diff, model_path, calc_back)