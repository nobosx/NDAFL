import net

diff = (0x0000, 0x0000)

# ID2
saved_folder = "./saved_models/ID2/"
mk_diff = (0x0040, 0x0000, 0x0000, 0x0000)
rk_diff_trail = [0x0000, 0x0000, 0x0000, 0x8000, 0x8002, 0x8008, 0x812a, 0x8480, 0x9382, 0xcf8a]

num_rounds = 9
net.train_speck_distinguisher(num_epochs=10, num_rounds=num_rounds, depth=5, diff=diff, mk_diff=mk_diff, rk_diff_trail=rk_diff_trail, saved_folder=saved_folder)

num_rounds = 10
former_net_path = saved_folder + "{}r_distinguisher.h5".format(num_rounds - 1)
net.train_speck_distinguisher_with_former_net(num_rounds=10, num_rounds=num_rounds, depth=5, diff=diff, mk_diff=mk_diff, rk_diff_trail=rk_diff_trail, former_net_path=former_net_path, saved_folder=saved_folder)

# ID3
saved_folder = "./saved_models/ID3/"
mk_diff = (0x0040, 0x0000, 0x0000, 0x0000)
rk_diff_trail = [0x0000, 0x0000, 0x0000, 0x8000, 0x8002, 0x8008, 0x812a, 0x8480, 0x9082, 0xc28a]

num_rounds = 9
net.train_speck_distinguisher(num_epochs=10, num_rounds=num_rounds, depth=5, diff=diff, mk_diff=mk_diff, rk_diff_trail=rk_diff_trail, saved_folder=saved_folder)

num_rounds = 10
former_net_path = saved_folder + "{}r_distinguisher.h5".format(num_rounds - 1)
net.train_speck_distinguisher_with_former_net(num_rounds=10, num_rounds=num_rounds, depth=5, diff=diff, mk_diff=mk_diff, rk_diff_trail=rk_diff_trail, former_net_path=former_net_path, saved_folder=saved_folder)