import net
import speck as sp


sp.check_testvector()
model_folder = './saved_models/'
data_form = "dl_l_dy_y"
# Training neural distinguishers for 5 to 7 rounds
for num_rounds in [5,6,7]:
    net.train_speck_distinguisher(num_epochs=10, num_rounds=num_rounds, depth=10, diff=(0x0040, 0x0), folder=model_folder, data_form=data_form)
    net_path = model_folder + "{}r_distinguisher_{}.h5".format(num_rounds, data_form)
    print("Testing model: {}...".format(net_path))
    net.eval_distinguisher(10**7, num_rounds, (0x40, 0), net_path, data_form)

# Training neural distinguisher for 8 rounds
num_rounds = 8
data_form = "dl_l_dy_y"
net.train_speck_distinguisher_using_stage_training(num_epochs=10, num_rounds=8, num_pre_rounds=3, diff=(0x40,0), auxiliary_diff=(0x8000, 0x840a), folder=model_folder, data_form=data_form)
net_path = model_folder + "{}r_distinguisher_{}.h5".format(num_rounds, data_form)
net.eval_distinguisher(10**7, num_rounds, (0x40, 0), net_path, data_form)

# Training auxiliary distinguishers AD
data_form = "dl_l_dy_y"
# 5r
IBs = [0, 1, 2, 8, 9, 10, 11, 16, 17, 18, 19, 23, 24, 25, 26, 27, 28, 48, 49, 50, 51, 56, 57, 58, 59, 60]
num_rounds = 5
net.train_auxiliary_distinguisher(num_epochs=10, num_rounds=num_rounds, depth=10, diff=(0x40, 0), folder=model_folder, IBs=IBs, data_form=data_form)
#6r
IBs = [0, 1, 2, 8, 9, 10, 11, 16, 17, 18, 19, 24, 25, 26, 27, 28, 48, 49, 50, 51, 56, 57, 58, 59, 60]
num_rounds = 6
net.train_auxiliary_distinguisher(num_epochs=10, num_rounds=num_rounds, depth=10, diff=(0x40, 0), folder=model_folder, IBs=IBs, data_form=data_form)
#7r
IBs = [1, 2, 8, 9, 10, 11, 17, 18, 19, 20, 21, 24, 25, 26, 27, 28, 50, 51, 52, 53, 57, 58, 59, 60]
num_rounds = 7
net.train_auxiliary_distinguisher(num_epochs=10, num_rounds=num_rounds, depth=10, diff=(0x40, 0), folder=model_folder, IBs=IBs, data_form=data_form)
#8r
# 8r
IBs = [1, 2, 3, 4, 10, 11, 17, 18, 19, 20, 21, 26, 27, 28, 49, 50, 51, 52, 53, 58, 59, 60]
num_rounds = 8
net.train_auxiliary_distinguisher_using_stage_training(num_epochs=10, num_rounds=8, num_pre_rounds=3, diff=(0x40,0), auxiliary_diff=(0x8000, 0x840a), folder=model_folder, IBs=IBs, data_form=data_form)