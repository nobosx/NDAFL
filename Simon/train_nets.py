from net import train_distinguisher, train_distinguisher_with_former_net
from cipher.simon import Simon

# -----------------------------Simon32/64------------------------------
# num_rounds = 9
# simon = Simon(n_rounds=num_rounds)
# simon.verify_test_vectors()
# saved_folder = "./saved_models/Simon32/"
# train_distinguisher(simon, [0, 0x40], n_epochs=10, kernel_size=7, lr_high=0.0027, lr_low=0.0002, cconv=True, calc_back=1, data_format="dx_x_dy", folder=saved_folder)

# num_rounds = 10
# simon = Simon(n_rounds=num_rounds)
# saved_folder = "./saved_models/Simon32/"
# former_net_path = "./saved_models/Simon32/{}r_distinguisher_dx_x_dy.h5".format(num_rounds - 1)
# train_distinguisher_with_former_net(simon, [0, 0x40], former_net_path, n_epochs=10, lr_high=0.0027, lr_low=0.0002, calc_back=1, data_format="dx_x_dy", folder=saved_folder)

# num_rounds = 11
# simon = Simon(n_rounds=num_rounds)
# saved_folder = "./saved_models/Simon32/"
# former_net_path = "./saved_models/Simon32/{}r_distinguisher_dx_x_dy.h5".format(num_rounds - 1)
# train_distinguisher_with_former_net(simon, [0, 0x40], former_net_path, n_epochs=10, lr_high=0.0027, lr_low=0.0002, calc_back=1, data_format="dx_x_dy", folder=saved_folder)
# ----------------------------------------------------------------------

# -----------------------------simon64/128------------------------------
num_rounds = 12
simon = Simon(n_rounds=num_rounds, word_size=32, const_seq=3)
saved_folder = "./saved_models/Simon64/"
train_distinguisher(simon, [0, 0x1], n_epochs=10, kernel_size=7, lr_high=0.0027, lr_low=0.0002, cconv=True, calc_back=1, data_format="dx_x_dy", folder=saved_folder)

num_rounds = 13
simon = Simon(n_rounds=num_rounds, word_size=32, const_seq=3)
saved_folder = "./saved_models/Simon64/"
former_net_path = "./saved_models/Simon64/{}r_distinguisher_dx_x_dy.h5".format(num_rounds - 1)
train_distinguisher_with_former_net(simon, [0, 0x1], former_net_path, n_epochs=10, lr_high=0.0027, lr_low=0.0002, calc_back=1, data_format="dx_x_dy", folder=saved_folder)
# ----------------------------------------------------------------------

# -----------------------------simon128/128------------------------------
num_rounds = 19
simon = Simon(n_rounds=num_rounds, word_size=64, const_seq=2, m=2)
saved_folder = "./saved_models/Simon128/"
train_distinguisher(simon, [0, 0x1], n_epochs=10, kernel_size=7, lr_high=0.0027, lr_low=0.0002, cconv=True, calc_back=1, data_format="dx_x_dy", folder=saved_folder)
# -----------------------------------------------------------------------