import numpy as np
from os import urandom

def WORD_SIZE():
    return 24

def ALPHA():
    return 8

def BETA():
    return 3

MASK_VAL = 2**WORD_SIZE() - 1

def rol(x, k):
    x = x & MASK_VAL
    return (((x << k) & MASK_VAL) | (x >> (WORD_SIZE() - k)))

def ror(x, k):
    x = x & MASK_VAL
    return ((x >> k) | ((x << (WORD_SIZE() - k)) & MASK_VAL))

def enc_one_round(p, k):
    c0, c1 = p[0], p[1]
    c0 = ror(c0, ALPHA())
    c0 = (c0 + c1) & MASK_VAL
    c0 = c0 ^ k
    c1 = rol(c1, BETA())
    c1 = c1 ^ c0
    return (c0,c1)

def dec_one_round(c,k):
    c0, c1 = c[0], c[1]
    c1 = c1 ^ c0
    c1 = ror(c1, BETA())
    c0 = c0 ^ k
    c0 = (c0 - c1) & MASK_VAL
    c0 = rol(c0, ALPHA())
    return (c0, c1)

def expand_key(k, t):
    ks = [0 for i in range(t)]
    ks[0] = k[len(k) - 1]
    l = list(reversed(k[:len(k) - 1]))
    tmp = len(l)
    for i in range(t - 1):
        l[i % tmp], ks[i + 1] = enc_one_round((l[i % tmp], ks[i]), i)
    return ks

def encrypt(p, ks):
    x, y = p[0], p[1]
    for k in ks:
        x, y = enc_one_round((x,y), k)
    return (x, y)

def decrypt(c, ks):
    x, y = c[0], c[1]
    for k in reversed(ks):
        x, y = dec_one_round((x,y), k)
    return (x,y)

def check_testvector():
    key = (0x121110, 0x0a0908, 0x020100)
    pt = (0x20796c, 0x6c6172)
    ks = expand_key(key, 22)
    ct = encrypt(pt, ks)
    if ct == (0xc049a5, 0x385adc):
        print('Testvector for speck48/72 verified.')
    else:
        print('Testvector for speck48/72 not verified.')
        return False

    key = (0x1a1918, 0x121110, 0x0a0908, 0x020100)
    pt = (0x6d2073, 0x696874)
    ks = expand_key(key, 23)
    ct = encrypt(pt, ks)
    if ct == (0x735e10, 0xb6445d):
        print('Testvector for speck48/96 verified.')
    else:
        print('Testvector for speck48/96 not verified.')
        return False

    n = 10**6
    pt = np.frombuffer(urandom(8 * n), dtype=np.uint32).reshape(2, n) & MASK_VAL
    key = np.frombuffer(urandom(12 * n), dtype=np.uint32).reshape(3, n) & MASK_VAL
    ks = expand_key(key, 22)
    ct = encrypt(pt, ks)
    pt_tmp = decrypt(ct, ks)
    if np.sum(pt[0] == pt_tmp[0]) == n and np.sum(pt[1] == pt_tmp[1]) == n:
        print('Testdecryption verified.')
    else:
        print('Testdecryption not verified.')
        return False

    return True

def convert_to_binary(arr):
    X = np.zeros((len(arr) * WORD_SIZE(), len(arr[0])), dtype=np.uint8)
    for i in range(len(arr) * WORD_SIZE()):
        index = i // WORD_SIZE()
        offset = WORD_SIZE() - (i % WORD_SIZE()) - 1
        X[i] = (arr[index] >> offset) & 1
    X = X.transpose()
    return X

def make_train_data(n, nr, diff, master_key_bit_length=72):
    assert master_key_bit_length % WORD_SIZE() == 0
    m = master_key_bit_length // WORD_SIZE()
    assert m == 3 or m == 4
    Y = np.frombuffer(urandom(n), dtype=np.uint8) & 1
    keys = np.frombuffer(urandom(4 * m * n), dtype=np.uint32).reshape(m, n) & MASK_VAL
    p0l = np.frombuffer(urandom(4 * n), dtype=np.uint32) & MASK_VAL
    p0r = np.frombuffer(urandom(4 * n), dtype=np.uint32) & MASK_VAL
    p1l = p0l ^ diff[0]; p1r = p0r ^ diff[1]
    num_rand_samples = np.sum(Y==0)
    p1l[Y == 0] = np.frombuffer(urandom(4 * num_rand_samples), dtype=np.uint32) & MASK_VAL
    p1r[Y == 0] = np.frombuffer(urandom(4 * num_rand_samples), dtype=np.uint32) & MASK_VAL
    ks = expand_key(keys, nr)
    c0l, c0r = encrypt((p0l, p0r), ks)
    c1l, c1r = encrypt((p1l, p1r), ks)
    y0 = ror(c0l ^ c0r, BETA())
    y1 = ror(c1l ^ c1r, BETA())
    X = convert_to_binary([c0l ^ c1l, c0l, y0 ^ y1, y0])
    return (X, Y)

def make_train_data_only_diff(n, nr, diff, master_key_bit_length=72):
    assert master_key_bit_length % WORD_SIZE() == 0
    m = master_key_bit_length // WORD_SIZE()
    assert m == 3 or m == 4
    Y = np.frombuffer(urandom(n), dtype=np.uint8) & 1
    keys = np.frombuffer(urandom(4 * m * n), dtype=np.uint32).reshape(m, n) & MASK_VAL
    p0l = np.frombuffer(urandom(4 * n), dtype=np.uint32) & MASK_VAL
    p0r = np.frombuffer(urandom(4 * n), dtype=np.uint32) & MASK_VAL
    p1l = p0l ^ diff[0]; p1r = p0r ^ diff[1]
    num_rand_samples = np.sum(Y==0)
    p1l[Y == 0] = np.frombuffer(urandom(4 * num_rand_samples), dtype=np.uint32) & MASK_VAL
    p1r[Y == 0] = np.frombuffer(urandom(4 * num_rand_samples), dtype=np.uint32) & MASK_VAL
    ks = expand_key(keys, nr)
    c0l, c0r = encrypt((p0l, p0r), ks)
    c1l, c1r = encrypt((p1l, p1r), ks)
    y0 = ror(c0l ^ c0r, BETA())
    y1 = ror(c1l ^ c1r, BETA())
    X = convert_to_binary([c0l ^ c1l, y0 ^ y1])
    return (X, Y)

if __name__ == '__main__':
    check_testvector()