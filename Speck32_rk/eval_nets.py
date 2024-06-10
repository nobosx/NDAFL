import net

diff = (0x0000, 0x0000)

# ID2
mk_diff = (0x0040, 0x0000, 0x0000, 0x0000)
rk_diff_trail = [0x0000, 0x0000, 0x0000, 0x8000, 0x8002, 0x8008, 0x812a, 0x8480, 0x9382, 0xcf8a]
for num_rounds in [9, 10]:
    print("Test {}r RK-ND of ID2 rk differential trail...".format(num_rounds))
    model_path = "./saved_models/ID2/{}r_distinguisher.h5".format(num_rounds)
    net.eval_distinguisher(10**7, model_path, num_rounds, diff, mk_diff, rk_diff_trail)

# ID3
mk_diff = (0x0040, 0x0000, 0x0000, 0x0000)
rk_diff_trail = [0x0000, 0x0000, 0x0000, 0x8000, 0x8002, 0x8008, 0x812a, 0x8480, 0x9082, 0xc28a]
for num_rounds in [9, 10]:
    print("Test {}r RK-ND of ID3 rk differential trail...".format(num_rounds))
    model_path = "./saved_models/ID3/{}r_distinguisher.h5".format(num_rounds)
    net.eval_distinguisher(10**7, model_path, num_rounds, diff, mk_diff, rk_diff_trail)