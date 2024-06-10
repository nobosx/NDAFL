import lea
import numpy as np
import os
from keras.models import load_model
from os import urandom

def make_target_bit_diffusion_data(X, id=0):
    n = X.shape[0]
    data_length = X.shape[1]
    masks = np.frombuffer(urandom(n), dtype=np.uint8) & 1
    masked_X = X.copy()
    masked_X[:, data_length - 1 - id] = masked_X[:, data_length - 1 - id] ^ masks
    return masked_X

def test_bits_sensitivity(n, nr, net_path, diff, folder, data_format, calc_back):
    data_length = 2 * lea.BLOCK_SIZE
    acc = np.zeros(data_length + 1)
    X, Y = lea.make_train_data(n, nr, diff, data_format=data_format, calc_back=calc_back)
    net = load_model(net_path)
    _, acc[data_length] = net.evaluate(X, Y, batch_size=10000, verbose=0)
    print('The initial acc is ', acc[data_length])

    for i in range(data_length):
        masked_X = make_target_bit_diffusion_data(X, i)
        _, acc[i] = net.evaluate(masked_X, Y, batch_size=10000, verbose=0)
        print('cur bit position is ', i)
        print('the decrease of the acc is ', acc[data_length] - acc[i])
    
    if not os.path.exists(folder):
        os.makedirs(folder)
    np.save(folder + '{}r_distinguisher_bit_sensitivity.npy'.format(nr), acc)

if __name__ == '__main__':
    data_format = "dx_x"
    diff_unit = 0x80000000
    calc_back = True
    diff = [diff_unit for _ in range(4)]
    for num_rounds in [9,10,11]:
        model_path = "./saved_models/{}r_distinguisher_{}.h5".format(num_rounds, data_format)
        bst_folder = "./bst_res/"
        test_bits_sensitivity(10**7, num_rounds, model_path, diff, bst_folder, data_format, calc_back)