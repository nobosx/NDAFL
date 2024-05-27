import speck as sp

import numpy as np
from keras.models import load_model
from os import urandom
import copy
import gc


def make_target_bit_diffusion_data(X, id0, id1, test_type=0):
    n = X.shape[0]
    data_width = X.shape[1]
    masks = np.frombuffer(urandom(n), dtype=np.uint8) & 0x1
    masked_X = copy.deepcopy(X)
    if test_type == 0:
        masked_X[:, data_width - 1 - id0] = X[:, data_width - 1 - id0] ^ masks
    else:
        masked_X[:, data_width - 1 - id0] = X[:, data_width - 1 - id0] ^ masks
        masked_X[:, data_width - 1 - id1] = X[:, data_width - 1 - id1] ^ masks
    return masked_X

def test_bits_sensitivity(n=10**7, nr=7, net_path='./', diff=(0x0040, 0x0), folder='./bst_res/', data_form="l_r"):
    data_width = 3 * sp.WORD_SIZE() if data_form == "dl_dy_y" else 4 * sp.WORD_SIZE()
    acc = np.zeros(data_width+1)
    X, Y = sp.make_train_data(n=n, nr=nr, diff=diff, data_form=data_form)
    assert X.shape[1] == data_width
    net = load_model(net_path)
    loss, acc[data_width] = net.evaluate(X, Y, batch_size=10000, verbose=0)
    print('The initial acc is ', acc[data_width])

    for i in range(data_width):
        
        masked_X = make_target_bit_diffusion_data(X, id0=i, id1=0, test_type=0)
        loss, acc[i] = net.evaluate(masked_X, Y, batch_size=10000, verbose=0)
        print('cur bit position is ', i) 
        print('the decrease of the acc is ', acc[data_width] - acc[i])
        gc.collect()
    
    np.save(folder + '{}r_distinguisher_bit_sensitivity_{}.npy'.format(nr, data_form), acc)

def test_xor_pair_sensitivity(n=10**7, nr=7, net_path='./', diff=(0x0040, 0x0), error_bar=0.0001):
    block_size = 2 * sp.WORD_SIZE()
    X, Y = sp.make_train_data(n=n, nr=nr, diff=diff, data_form="l_r")
    net = load_model(net_path)
    loss, acc = net.evaluate(X, Y, batch_size=10000, verbose=0)
    print('The initial acc is ', acc)

    # Test C[i] xor C'[i]
    for i in range(block_size):
        masked_X = make_target_bit_diffusion_data(X, id0=i, id1=i+block_size, test_type=1)
        loss, tmp_acc = net.evaluate(masked_X, Y, batch_size=10000, verbose=0)
        if acc - tmp_acc < error_bar:
            print('X[{}] and X[{}+{}] can be combined into (X[{}] xor X[{}+{}]).'.format(i, i, block_size, i, i, block_size))
        gc.collect()

    # Test l[i] xor r[i]
    for i in range(sp.WORD_SIZE()):
        masked_X = make_target_bit_diffusion_data(X, id0=i, id1=i+sp.WORD_SIZE(), test_type=1)
        loss, tmp_acc = net.evaluate(masked_X, Y, batch_size=10000, verbose=0)
        if acc - tmp_acc < error_bar:
            print('L[{}] and R[{}] can be combined into (L[{}] xor R[{}]).'.format(i, i, i, i))
        gc.collect()

# Bit sensitivity test
data_form = "l_r"
num_rounds = 8
net_path = './saved_models/{}r_distinguisher_{}.h5'.format(num_rounds, data_form)
print("Test bst: {}.".format(net_path))
test_bits_sensitivity(n=10**7, nr=num_rounds, net_path=net_path, diff=(0x0040, 0x0), folder="./bst_res/", data_form=data_form)

# Bit reduction test
num_rounds = 7
net_path = './saved_models/{}r_distinguisher_l_r.h5'.format(num_rounds)
test_xor_pair_sensitivity(n=10**7, nr=num_rounds, net_path=net_path, diff=(0x40, 0))
