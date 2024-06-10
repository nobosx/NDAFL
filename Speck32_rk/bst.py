from os import urandom, environ
environ['TF_CPP_MIN_LOG_LEVEL'] = '3'
environ["TF_FORCE_GPU_ALLOW_GROWTH"] = "true"

from keras.models import load_model
import numpy as np
import speck_rk as sp

word_size = sp.WORD_SIZE()

def make_diffusion_data(X, bit_id, num_words):
    global word_size
    data_width = word_size * num_words
    n = len(X)
    random_masks = np.frombuffer(urandom(n), dtype=np.uint8) & 1
    masked_X = X.copy()
    masked_X[:, data_width - 1 - bit_id] = masked_X[:, data_width - 1 - bit_id] ^ random_masks
    return masked_X

def test_bit_sensitivity(n, model_path, num_rounds, diff, mk_diff, rk_diff_trail, saved_folder=None, num_words=4):
    global word_size
    data_width = word_size * num_words
    bst = np.zeros(data_width + 1)
    net = load_model(model_path)
    X_eval, Y_eval = sp.make_train_data(n, num_rounds, diff, mk_diff, rk_diff_trail)
    loss, acc = net.evaluate(X_eval, Y_eval, batch_size=10000, verbose=0)
    bst[data_width] = acc
    print("initial acc is {}.".format(acc))

    for bit_id in range(data_width):
        masked_X = make_diffusion_data(X_eval, bit_id, num_words)
        loss, bst[bit_id] = net.evaluate(masked_X, Y_eval, batch_size=10000, verbose=0)
        print("bit pos is {}, acc decrease is {}.".format(bit_id, bst[data_width] - bst[bit_id]))
    
    if saved_folder is not None:
        np.save(saved_folder + "{}r_bst_res.npy".format(num_rounds), bst)

if __name__ == "__main__":
    diff = (0x0000, 0x0000)

    # ID2
    saved_folder = "./bst_res/ID2/"
    mk_diff = (0x0040, 0x0000, 0x0000, 0x0000)
    rk_diff_trail = [0x0000, 0x0000, 0x0000, 0x8000, 0x8002, 0x8008, 0x812a, 0x8480, 0x9382, 0xcf8a]
    for num_rounds in [9, 10]:
        print("bst for {}r ND of ID2 rk differential trail...".format(num_rounds))
        model_path = "./saved_models/ID2/{}r_distinguisher.h5".format(num_rounds)
        test_bit_sensitivity(2**24, model_path, num_rounds, diff, mk_diff, rk_diff_trail, saved_folder)

    # ID3
    saved_folder = "./bst_res/ID3/"
    mk_diff = (0x0040, 0x0000, 0x0000, 0x0000)
    rk_diff_trail = [0x0000, 0x0000, 0x0000, 0x8000, 0x8002, 0x8008, 0x812a, 0x8480, 0x9082, 0xc28a]
    for num_rounds in [9, 10]:
        print("bst for {}r ND of ID3 rk differential trail...".format(num_rounds))
        model_path = "./saved_models/ID3/{}r_distinguisher.h5".format(num_rounds)
        test_bit_sensitivity(2**24, model_path, num_rounds, diff, mk_diff, rk_diff_trail, saved_folder)