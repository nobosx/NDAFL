import net
import speck as sp

sp.check_testvector()
# Evaluating neural distinguishers for 5r to 8r Speck32/64
# data_form = "l_r"
data_form = "dl_l_dy_y"
for num_rounds in [5,6,7,8]:
    print("Evaluating {}r neural distinguishers, data form is {}...".format(num_rounds, data_form))
    net_path = "./saved_models/{}r_distinguisher_{}.h5".format(num_rounds, data_form)
    net.eval_distinguisher(n=10**7, num_rounds=num_rounds, diff=(0x40,0), net_path=net_path, data_form=data_form)