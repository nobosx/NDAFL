import net
import speck as sp

sp.check_testvector()
# evaluate neural distinguishers for 5r to 8r Speck32/64
data_form = "dl_l_dy_y"
for num_rounds in [5,6,7,8]:
    print("Evaluating {}r neural distinguishers, data form is {}...".format(num_rounds, data_form))
    net_path = "./saved_models/{}r_distinguisher_{}.h5".format(num_rounds, data_form)
    net.eval_distinguisher(n=10**7, num_rounds=num_rounds, diff=(0x40,0), net_path=net_path, data_form=data_form)

# evaluate auxiliary distinguishers for 5r to 8r Speck32/64
data_form = "dl_l_dy_y"
IBs = {
    5 : [0, 1, 2, 8, 9, 10, 11, 16, 17, 18, 19, 23, 24, 25, 26, 27, 28, 48, 49, 50, 51, 56, 57, 58, 59, 60],
    6 : [0, 1, 2, 8, 9, 10, 11, 16, 17, 18, 19, 24, 25, 26, 27, 28, 48, 49, 50, 51, 56, 57, 58, 59, 60],
    7 : [1, 2, 8, 9, 10, 11, 17, 18, 19, 20, 21, 24, 25, 26, 27, 28, 50, 51, 52, 53, 57, 58, 59, 60],
    8 : [1, 2, 3, 4, 10, 11, 17, 18, 19, 20, 21, 26, 27, 28, 49, 50, 51, 52, 53, 58, 59, 60]
}
for num_rounds in [5,6,7,8]:
    print("Evaluating {}r auxiliary distinguishers, data form is {}...".format(num_rounds, data_form))
    net_path = "./saved_models/auxiliary_nets/{}r_auxiliary_distinguisher_{}.h5".format(num_rounds, data_form)
    net.eval_auxiliary_distinguisher(n=10**7, num_rounds=num_rounds, diff=(0x40,0), net_path=net_path, IBs=IBs[num_rounds], data_form=data_form)
    