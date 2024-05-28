from os import urandom, environ
import os
from keras.models import load_model
import numpy as np
import speck as sp

word_size = sp.WORD_SIZE()

def make_diffusion_data(X, bit_id, num_words):
    global word_size
    data_width = num_words * word_size
    n = len(X)
    random_masks = np.frombuffer(urandom(n), dtype=np.uint8) & 1
    masked_X = X.copy()
    masked_X[:, data_width - 1 - bit_id] = masked_X[:, data_width - 1 - bit_id] ^ random_masks
    return masked_X

def test_bit_sensitivity(n, model_path, num_rounds, diff, saved_folder=None, num_words=4):
    global word_size
    data_width = word_size * num_words
    bst = np.zeros(data_width + 1)
    net = load_model(model_path)
    X_eval, Y_eval = sp.make_train_data(n, num_rounds, diff)
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
    num_rounds = 7
    diff_settings = {
        "ID1" : ((0x80, 0), 31),
        "ID2" : ((0x80, 0x800000), (23, 31)),
        "ID3" : ((0, 0x800000), 23)
    }
    for key, value in diff_settings.items():
        diff, diff_index = value
        print("Bit sensitivity test for diff {}, diff index is {}, num rounds is {}.".format(key, diff_index, num_rounds))
        model_path = "./saved_models/{}_{}_distinguisher.h5".format(diff_index, num_rounds)
        saved_folder = "./bst_res/{}_".format(diff_index)
        test_bit_sensitivity(2**22, model_path, num_rounds, diff, saved_folder)