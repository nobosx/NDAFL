import speck as sp
import time
import numpy as np
from keras.models import load_model

def extract_table_input_val(X, num_rounds):
    y0_prime = X[0] ^ X[1]
    y1_prime = X[2] ^ X[3]
    dy_prime = y0_prime ^ y1_prime
    res = X[0] ^ X[2]
    if num_rounds == 5:
        res = ((res & 0x1f00) << 13) | ((res & 0xf) << 17)
        res = res | ((dy_prime & 0x7e00) << 2) | ((dy_prime & 0x3c) << 5) | ((y0_prime & 0x3c00) >> 7) | ((y0_prime & 0x1c) >> 2)
    elif num_rounds == 6:
        res = ((res & 0x1f00) << 12) | ((res & 0xf) << 16)
        res = res | ((dy_prime & 0x7c00) << 1) | ((dy_prime & 0x3c) << 5) | ((y0_prime & 0x3c00) >> 7) | ((y0_prime & 0x1c) >> 2)
    elif num_rounds == 7:
        res = ((res & 0x1e00) << 11) | ((res & 0x003c) << 14)
        res = res | ((dy_prime & 0x7c00) << 1) | ((dy_prime & 0x00f8) << 3) | ((y0_prime & 0x3c00) >> 8) | ((y0_prime & 0x0018) >> 3)
    elif num_rounds == 8:
        res = ((res & 0x1c00) << 9) | ((res & 0x003e) << 13)
        res = res | ((dy_prime & 0x7000) >> 1) | ((dy_prime & 0x00f8) << 3) | ((y0_prime & 0x3000) >> 8) | ((y0_prime & 0x0078) >> 3)
    return res

def eval_neural_distinguisher(X, Y, net_path):
    net = load_model(net_path, compile=False)
    start = time.time()
    Z = net.predict(X, batch_size=10000, verbose=1)
    Z = Z.flatten() > 0.5
    end = time.time()
    tpr = np.sum((Z == Y) & (Y == 1)) / np.sum(Y == 1)
    tnr = np.sum((Z == Y) & (Y == 0)) / np.sum(Y == 0)
    acc = (tpr + tnr) / 2
    print("Testing neural distinguisher:", net_path)
    print("Data size is: {}".format(len(X)))
    print("Test time: {} s".format(end - start))
    print("Acc = {}, tpr = {}, tnr = {}".format(acc, tpr, tnr))
    return Z

def eval_lookup_table_distinguisher(X, Y, table_path, num_rounds, alpha):
    lt = np.fromfile(table_path, dtype=np.uint64)
    assert np.sum(lt) == len(lt) * (1 << alpha)
    metric = 1 << alpha
    start = time.time()
    input_val = extract_table_input_val(X, num_rounds)
    Z = lt[input_val] > metric
    end = time.time()
    tpr = np.sum((Z == Y) & (Y == 1)) / np.sum(Y == 1)
    tnr = np.sum((Z == Y) & (Y == 0)) / np.sum(Y == 0)
    acc = (tpr + tnr) / 2
    print("Testing lookup table distinguisher:", table_path)
    print("Data size is: {}".format(len(input_val)))
    print("Test time: {} s".format(end - start))
    print("Acc = {}, tpr = {}, tnr = {}".format(acc, tpr, tnr))
    return Z

if __name__ == "__main__":
    sp.check_testvector()
    n = 10**7
    # Test 5r distinguisher
    num_rounds = 5
    X, Y = sp.make_train_data(n, num_rounds, (0x40, 0), "l_r", bits_form=False)
    print("Test {}r distinguishers.".format(num_rounds))
    # neural distinguisher
    net_path = "./saved_models/{}r_distinguisher_l_r.h5".format(num_rounds)
    Z1 = eval_neural_distinguisher(sp.convert_to_binary(X), Y, net_path)
    # lookup table distnguisher
    table_path = "./cpp/lookup_table/5r_table_26_6_5R"
    Z2 = eval_lookup_table_distinguisher([x.astype(np.uint64) for x in X], Y, table_path, 5, 6)
    match_rate = np.sum(Z1 == Z2) / len(Z1)
    print("Matching rate is", match_rate)

    # Test 6r distinguisher
    num_rounds = 6
    X, Y = sp.make_train_data(n, num_rounds, (0x40, 0), "l_r", bits_form=False)
    print("Test {}r distinguishers.".format(num_rounds))
    # neural distinguisher
    net_path = "./saved_models/{}r_distinguisher_l_r.h5".format(num_rounds)
    Z1 = eval_neural_distinguisher(sp.convert_to_binary(X), Y, net_path)
    # lookup table distnguisher
    table_path = "./cpp/lookup_table/6r_table_25_6_6R"
    Z2 = eval_lookup_table_distinguisher([x.astype(np.uint64) for x in X], Y, table_path, 6, 6)
    match_rate = np.sum(Z1 == Z2) / len(Z1)
    print("Matching rate is", match_rate)

    # Test 7r distinguisher
    num_rounds = 7
    X, Y = sp.make_train_data(n, num_rounds, (0x40, 0), "l_r", bits_form=False)
    print("Test {}r distinguishers.".format(num_rounds))
    # neural distinguisher
    net_path = "./saved_models/{}r_distinguisher_l_r.h5".format(num_rounds)
    Z1 = eval_neural_distinguisher(sp.convert_to_binary(X), Y, net_path)
    # lookup table distnguisher
    table_path = "./cpp/lookup_table/7r_table_24_10_7R"
    Z2 = eval_lookup_table_distinguisher([x.astype(np.uint64) for x in X], Y, table_path, 7, 10)
    match_rate = np.sum(Z1 == Z2) / len(Z1)
    print("Matching rate is", match_rate)

    # Test 8r distinguisher
    num_rounds = 8
    X, Y = sp.make_train_data(n, num_rounds, (0x40, 0), "l_r", bits_form=False)
    print("Test {}r distinguishers.".format(num_rounds))
    # neural distinguisher
    net_path = "./saved_models/{}r_distinguisher_l_r.h5".format(num_rounds)
    Z1 = eval_neural_distinguisher(sp.convert_to_binary(X), Y, net_path)
    # lookup table distnguisher
    table_path = "./cpp/lookup_table/8r_table_22_18_8R"
    Z2 = eval_lookup_table_distinguisher([x.astype(np.uint64) for x in X], Y, table_path, 8, 18)
    match_rate = np.sum(Z1 == Z2) / len(Z1)
    print("Matching rate is", match_rate)