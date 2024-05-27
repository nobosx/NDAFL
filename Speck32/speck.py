import numpy as np
from os import urandom


def WORD_SIZE():
    return(16)


def ALPHA():
    return(7)


def BETA():
    return(2)


MASK_VAL = 2 ** WORD_SIZE() - 1


def shuffle_together(l):
    state = np.random.get_state()
    for x in l:
        np.random.set_state(state)
        np.random.shuffle(x)


def rol(x, k):
    return(((x << k) & MASK_VAL) | (x >> (WORD_SIZE() - k)))


def ror(x, k):
    return((x >> k) | ((x << (WORD_SIZE() - k)) & MASK_VAL))


def enc_one_round(p, k):
    c0, c1 = p[0], p[1]
    c0 = ror(c0, ALPHA())
    c0 = (c0 + c1) & MASK_VAL
    c0 = c0 ^ k
    c1 = rol(c1, BETA())
    c1 = c1 ^ c0
    return(c0,c1)


def dec_one_round(c,k):
    c0, c1 = c[0], c[1]
    c1 = c1 ^ c0
    c1 = ror(c1, BETA())
    c0 = c0 ^ k
    c0 = (c0 - c1) & MASK_VAL
    c0 = rol(c0, ALPHA())
    return(c0, c1)


def expand_key(k, t):
    ks = [0 for i in range(t)]
    ks[0] = k[len(k)-1]
    l = list(reversed(k[:len(k)-1]))
    for i in range(t-1):
        l[i%3], ks[i+1] = enc_one_round((l[i%3], ks[i]), i)
    return(ks)


def encrypt(p, ks):
    x, y = p[0], p[1]
    for k in ks:
        x,y = enc_one_round((x,y), k)
    return(x, y)


def decrypt(c, ks):
    x, y = c[0], c[1]
    for k in reversed(ks):
        x, y = dec_one_round((x,y), k)
    return(x,y)


def check_testvector():
    key = (0x1918, 0x1110, 0x0908, 0x0100)
    pt = (0x6574, 0x694c)
    ks = expand_key(key, 22)
    ct = encrypt(pt, ks)
    if (ct == (0xa868, 0x42f2)):
        print("Testvector verified.")
        return(True)
    else:
        print("Testvector not verified.")
        return(False)


def convert_to_binary(arr):
    n = len(arr)
    X = np.zeros((n * WORD_SIZE(), len(arr[0])), dtype=np.uint8)
    for i in range(n * WORD_SIZE()):
        index = i // WORD_SIZE()
        offset = WORD_SIZE() - (i % WORD_SIZE()) - 1
        X[i] = (arr[index] >> offset) & 1
    X = X.transpose()
    return(X)


def make_train_data(n, nr, diff=(0x0040, 0), data_form="l_r"):
    Y = np.frombuffer(urandom(n), dtype=np.uint8); Y = Y & 1
    keys = np.frombuffer(urandom(8*n), dtype=np.uint16).reshape(4, -1)
    plain0l = np.frombuffer(urandom(2*n), dtype=np.uint16)
    plain0r = np.frombuffer(urandom(2*n), dtype=np.uint16)
    plain1l = plain0l ^ diff[0]; plain1r = plain0r ^ diff[1]
    num_rand_samples = np.sum(Y==0)
    plain1l[Y==0] = np.frombuffer(urandom(2*num_rand_samples), dtype=np.uint16)
    plain1r[Y==0] = np.frombuffer(urandom(2*num_rand_samples), dtype=np.uint16)
    ks = expand_key(keys, nr)
    ctdata0l, ctdata0r = encrypt((plain0l, plain0r), ks)
    ctdata1l, ctdata1r = encrypt((plain1l, plain1r), ks)
    if data_form == "l_r":
        X = convert_to_binary([ctdata0l, ctdata0r, ctdata1l, ctdata1r])
    elif data_form == "dl_dy_y":
        ctdata0y = ror(ctdata0l ^ ctdata0r, BETA())
        ctdata1y = ror(ctdata1l ^ ctdata1r, BETA())
        X = convert_to_binary([ctdata0l^ctdata1l, ctdata0y^ctdata1y, ctdata0y])
    elif data_form == "dl_l_dy_y":
        ctdata0y = ror(ctdata0l ^ ctdata0r, BETA())
        ctdata1y = ror(ctdata1l ^ ctdata1r, BETA())
        X = convert_to_binary([ctdata0l^ctdata1l, ctdata0l, ctdata0y^ctdata1y, ctdata0y])
    return (X,Y)

def make_target_diff_samples(n=10**7, nr=7, diff_type=1, diff=(0x40, 0), data_form="l_r", return_keys=False, common_key=False, bits_form=True):
    p0l = np.frombuffer(urandom(2 * n), dtype=np.uint16)
    p0r = np.frombuffer(urandom(2 * n), dtype=np.uint16)
    if diff_type == 1:
        p1l, p1r = p0l ^ diff[0], p0r ^ diff[1]
    else:
        p1l = np.frombuffer(urandom(2 * n), dtype=np.uint16)
        p1r = np.frombuffer(urandom(2 * n), dtype=np.uint16)
    if common_key:
        keys = np.frombuffer(urandom(8), dtype=np.uint16).reshape(4, -1)
    else:
        keys = np.frombuffer(urandom(8 * n), dtype=np.uint16).reshape(4, -1)
    ks = expand_key(keys, nr)
    c0l, c0r = encrypt((p0l, p0r), ks)
    c1l, c1r = encrypt((p1l, p1r), ks)
    if data_form == "l_r":
        X = (c0l, c0r, c1l, c1r)
    elif data_form == "dl_dy_y":
        c0y = ror(c0l ^ c0r, BETA())
        c1y = ror(c1l ^ c1r, BETA())
        X = (c0l ^ c1l, c0y ^ c1y, c0y)
    elif data_form == "dl_l_dy_y":
        c0y = ror(c0l ^ c0r, BETA())
        c1y = ror(c1l ^ c1r, BETA())
        X = (c0l ^ c1l, c0l, c0y ^ c1y, c0y)
    if bits_form:
        X = convert_to_binary(X)
    if return_keys == 0:
        return X
    else:
        return X, ks