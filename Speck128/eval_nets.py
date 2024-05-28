import net

diff_settings = {
    "ID1" : ((0x80, 0), 71, [9]),
    "ID2" : ((0x80, 0x8000000000000000), (63, 71), [9, 10])
}

for key, value in diff_settings.items():
    diff, diff_index, nrs = value
    for num_rounds in nrs:
        # Evaluate the ND
        print("Num rounds is {}, diff ID is {}, diff index is {}.".format(num_rounds, key, diff_index))
        print("Test original net:")
        model_path = "./saved_models/{}_{}_distinguisher.h5".format(diff_index, num_rounds)
        net.eval_distinguisher(n=2**24, net_path=model_path, num_rounds=num_rounds, diff=diff)
        # Evaluate the NDD
        print("Test net with only diff:")
        model_path = "./saved_models/{}_{}_distinguisher_only_diff.h5".format(diff_index, num_rounds)
        net.eval_distinguisher_only_diff(n=2**24, net_path=model_path, num_rounds=num_rounds, diff=diff)