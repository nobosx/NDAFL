import numpy as np
from os import urandom


def WORD_SIZE():
    return(16)

def ALPHA():
    return(7)

def BETA():
    return(2)

MASK_VAL = 2 ** WORD_SIZE() - 1

def rol(x,k):
    return(((x << k) & MASK_VAL) | (x >> (WORD_SIZE() - k)))

def ror(x,k):
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
    key = (0x1918,0x1110,0x0908,0x0100)
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
    word_num = len(arr)
    n = len(arr[0])
    X = np.zeros((word_num * WORD_SIZE(), n), dtype=np.uint8)
    for i in range(word_num * WORD_SIZE()):
        index = i // WORD_SIZE()
        offset = WORD_SIZE() - (i % WORD_SIZE()) - 1
        X[i] = (arr[index] >> offset) & 1
    X = X.transpose()
    return(X)


def make_train_data(n, nr, diff, mk_diff, rk_diff_trail, only_diff=False):
    keys = np.frombuffer(urandom(8 * n), dtype=np.uint16).reshape(4, -1)
    keys_diff = np.array([keys[0] ^ mk_diff[0], keys[1] ^ mk_diff[1], keys[2] ^ mk_diff[2], keys[3] ^ mk_diff[3]],dtype=np.uint16)
    
    ks = expand_key(keys, nr)
    ks_diff = expand_key(keys_diff, nr)

    # filter keys that conform to the round key differential
    check_index = np.full(n, True, dtype=bool)
    for i in range(nr):
        check_index = check_index & ((ks[i] ^ ks_diff[i]) == rk_diff_trail[i])
    new_ks = [ks[i][check_index] for i in range(nr)]
    new_ks_diff = [ks_diff[i][check_index] for i in range(nr)]
    
    
    rep_num = (n + len(new_ks[0]) - 1) // len(new_ks[0])
    new_ks = np.repeat(new_ks, rep_num, axis=1)
    new_ks_diff = np.repeat(new_ks_diff, rep_num, axis=1)
    
    dataset_size = len(new_ks[0])
    
    Y = np.frombuffer(urandom(dataset_size), dtype=np.uint8)
    Y = Y & 1
    num_rand_samples = np.sum(Y == 0)
    
    plain0l = np.frombuffer(urandom(2 * dataset_size), dtype=np.uint16)
    plain0r = np.frombuffer(urandom(2 * dataset_size), dtype=np.uint16)
    plain1l = plain0l ^ diff[0]
    plain1r = plain0r ^ diff[1]
            
    plain1l[Y == 0] = np.frombuffer(urandom(2 * num_rand_samples), dtype=np.uint16)
    plain1r[Y == 0] = np.frombuffer(urandom(2 * num_rand_samples), dtype=np.uint16)
        
    ctdata0l, ctdata0r = encrypt((plain0l, plain0r), new_ks)
    ctdata1l, ctdata1r = encrypt((plain1l, plain1r), new_ks_diff)

    R0 = ror(ctdata0l ^ ctdata0r, BETA())
    R1 = ror(ctdata1l ^ ctdata1r, BETA())

    if only_diff:
        # only use the differential information
        X_result = [ctdata0l ^ ctdata1l, R0 ^ R1]
    else:
        # dl_l_dy_y
        X_result = [ctdata0l ^ ctdata1l, ctdata0l, R0 ^ R1, R0]
   
    X= convert_to_binary(X_result)
    return (X, Y)

check_testvector()