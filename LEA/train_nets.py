import lea
import numpy as np
import net

num_rounds = 11
data_format = "dx_x"
calc_back = True
diff_unit = 0x80000000
diff = [diff_unit for _ in range(4)]

# Train neural distinguishers

# 9r ND
num_rounds = 9
model_folder = "./saved_models/"
net.train_distinguisher(num_epochs=10, diff=diff, num_rounds=num_rounds, saved_folder=model_folder, depth=10, data_format=data_format, calc_back=calc_back)

# 10r ND
num_rounds = 10
model_folder = "./saved_models/"
net.train_distinguisher(num_epochs=10, diff=diff, num_rounds=num_rounds, saved_folder=model_folder, depth=10, data_format=data_format, calc_back=calc_back)

# 11r ND
num_rounds = 11
model_folder = "./saved_models/"
former_net_path = model_folder + "{}r_distinguisher_{}.h5".format(num_rounds - 1, data_format)
net.train_distinguisher(num_epochs=10, diff=diff, num_rounds=num_rounds, saved_folder=model_folder, depth=10, data_format=data_format, former_net_path=former_net_path, calc_back=calc_back)

# Train neural differential distinguishers

# 9r NDD
num_rounds = 9
model_folder = "./saved_models/"
net.train_distinguisher_with_only_diff(num_epochs=10, diff=diff, num_rounds=num_rounds, saved_folder=model_folder, depth=10, calc_back=calc_back)

# 10r NDD
num_rounds = 10
model_folder = "./saved_models/"
net.train_distinguisher_with_only_diff(num_epochs=10, diff=diff, num_rounds=num_rounds, saved_folder=model_folder, depth=10, calc_back=calc_back)

# 11r NDD
num_rounds = 11
model_folder = "./saved_models/"
former_net_path = model_folder + "{}r_distinguisher_only_diff.h5".format(num_rounds - 1)
net.train_distinguisher_with_only_diff(num_epochs=10, diff=diff, num_rounds=num_rounds, saved_folder=model_folder, depth=10, former_net_path=former_net_path, calc_back=calc_back)