from os import urandom, environ, path, makedirs
environ['TF_CPP_MIN_LOG_LEVEL'] = '3'
environ["TF_FORCE_GPU_ALLOW_GROWTH"] = "true"
from keras.models import load_model
from cipher.abstract_cipher import AbstractCipher
from cipher.simon import Simon
from make_data import make_train_data
import numpy as np
from net import train_distinguisher, train_distinguisher_with_former_net

def make_diffusion_data(X, bit_id):
    data_width = len(X[0])
    n = len(X)
    random_masks = np.frombuffer(urandom(n), dtype=np.uint8) & 1
    masked_X = X.copy()
    masked_X[:, data_width - 1 - bit_id] = masked_X[:, data_width - 1 - bit_id] ^ random_masks
    return masked_X

def test_bit_sensitivity(n, model_path, cipher: AbstractCipher, diff, saved_folder=None, num_words=3):
    data_width = cipher.get_word_size() * num_words
    bst = np.zeros(data_width + 1)
    net = load_model(model_path)
    X_eval, Y_eval = make_train_data(n, cipher, diff, 1, data_format="dx_x_dy")
    loss, acc = net.evaluate(X_eval, Y_eval, batch_size=10000, verbose=0)
    bst[data_width] = acc
    print("initial acc is {}.".format(acc))

    for bit_id in range(data_width):
        masked_X = make_diffusion_data(X_eval, bit_id)
        loss, bst[bit_id] = net.evaluate(masked_X, Y_eval, batch_size=10000, verbose=0)
        print("bit pos is {}, acc decrease is {}.".format(bit_id, bst[data_width] - bst[bit_id]))

    if saved_folder is not None:
        if not path.exists(saved_folder):
            makedirs(saved_folder)
        np.save(saved_folder + "{}r_bst_res.npy".format(cipher.get_n_rounds()), bst)

if __name__ == "__main__":
    # -----------------------------Simon32/64------------------------------
    # for num_rounds in [9,10,11]:
    #     cipher = Simon(n_rounds=num_rounds)
    #     model_path = "./saved_models/Simon32/{}r_distinguisher_dx_x_dy.h5".format(num_rounds)
    #     saved_folder = "./bst_res/Simon32/"
    #     test_bit_sensitivity(2**24, model_path, cipher, [0, 0x40], saved_folder)
    # ----------------------------------------------------------------------

    # -----------------------------simon64/128------------------------------
    # for num_rounds in [12, 13]:
    #     cipher = Simon(n_rounds=num_rounds, word_size=32, const_seq=3)
    #     model_path = "./saved_models/Simon64/{}r_distinguisher_dx_x_dy.h5".format(num_rounds)
    #     saved_folder = "./bst_res/Simon64/"
    #     test_bit_sensitivity(2**24, model_path, cipher, [0, 0x1], saved_folder)
    # ----------------------------------------------------------------------

    # -----------------------------simon128/128------------------------------
    num_rounds = 19
    cipher = Simon(n_rounds=num_rounds, word_size=64, const_seq=2, m=2)
    model_path = "./saved_models/Simon128/{}r_distinguisher_dx_x_dy.h5".format(num_rounds)
    saved_folder = "./bst_res/Simon128/"
    test_bit_sensitivity(2**24, model_path, cipher, [0, 0x1], saved_folder)
    # -----------------------------------------------------------------------