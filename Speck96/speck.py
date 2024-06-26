import numpy as np
from os import urandom


def WORD_SIZE():
    return(48)


def ALPHA():
    return(8)


def BETA():
    return(3)


MASK_VAL = 2 ** WORD_SIZE() - 1


def shuffle_together(l):
    state = np.random.get_state()
    for x in l:
        np.random.set_state(state)
        np.random.shuffle(x)


def rol(x, k):
    x = x & 0x0000ffffffffffff      # set 48~63 bits to 0
    return(((x << k) & MASK_VAL) | (x >> (WORD_SIZE() - k)))


def ror(x, k):
    x = x & 0x0000ffffffffffff      # set 48~63 bits to 0
    return((x >> k) | ((x << (WORD_SIZE() - k)) & MASK_VAL))


def enc_one_round(p, k):
    c0, c1 = p[0], p[1]
    c0 = ror(c0, ALPHA())
    c0 = (c0 + c1) & MASK_VAL
    c0 = c0 ^ k
    c1 = rol(c1, BETA())
    c1 = c1 ^ c0
    return(c0, c1)


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
        l[0], ks[i+1] = enc_one_round((l[0], ks[i]), i)
    return(ks)


def encrypt(p, ks):
    x, y = p[0], p[1]
    for k in ks:
        x, y = enc_one_round((x, y), k)
    return(x, y)


def decrypt(c, ks):
    x, y = c[0], c[1]
    for k in reversed(ks):
        x, y = dec_one_round((x, y), k)
    return(x,y)


def check_testvector():
    key = (0x0d0c0b0a0908, 0x050403020100)
    pt = (0x65776f68202c, 0x656761737520)
    ks = expand_key(key, 28)
    ct = encrypt(pt, ks)
    if (ct == (0x9e4d09ab7178, 0x62bdde8f79aa)):
        print("Testvector verified.")
        return(True)
    else:
        print("Testvector not verified.")
        return(False)


# convert_to_binary takes as input an array of ciphertext pairs
# where the first row of the array contains the lefthand side of the ciphertexts,
# the second row contains the righthand side of the ciphertexts,
# the third row contains the lefthand side of the second ciphertexts,
# and so on
# it returns an array of bit vectors containing the same data
def convert_to_binary(arr):
    n = len(arr)
    X = np.zeros((n * WORD_SIZE(), len(arr[0])), dtype=np.uint8)
    for i in range(n * WORD_SIZE()):
        index = i // WORD_SIZE()
        offset = WORD_SIZE() - (i % WORD_SIZE()) - 1
        X[i] = (arr[index] >> offset) & 1
    X = X.transpose()
    return(X)

# baseline training data generator,  speck96/96
def make_train_data(n, nr, diff=(0x80, 0x0)):
    Y = np.frombuffer(urandom(n), dtype=np.uint8)
    Y = Y & 1
    keys = np.frombuffer(urandom(16*n), dtype=np.uint64).reshape(2, -1)
    plain0l = np.frombuffer(urandom(8*n), dtype=np.uint64)
    plain0r = np.frombuffer(urandom(8*n), dtype=np.uint64)
    plain1l = plain0l ^ diff[0]
    plain1r = plain0r ^ diff[1]
    num_rand_samples = np.sum(Y == 0)
    plain1l[Y == 0] = np.frombuffer(urandom(8*num_rand_samples), dtype=np.uint64)
    plain1r[Y == 0] = np.frombuffer(urandom(8*num_rand_samples), dtype=np.uint64)
    ks = expand_key(keys, nr)
    ctdata0l, ctdata0r = encrypt((plain0l, plain0r), ks)
    ctdata1l, ctdata1r = encrypt((plain1l, plain1r), ks)
    ctdata0y = ror(ctdata0l ^ ctdata0r, BETA())
    ctdata1y = ror(ctdata1l ^ ctdata1r, BETA())
    X = convert_to_binary([ctdata0l ^ ctdata1l, ctdata0l, ctdata0y ^ ctdata1y, ctdata0y])
    return(X,Y)

def make_train_data_only_diff(n, nr, diff):
    Y = np.frombuffer(urandom(n), dtype=np.uint8)
    Y = Y & 1
    keys = np.frombuffer(urandom(16*n), dtype=np.uint64).reshape(2, -1)
    plain0l = np.frombuffer(urandom(8*n), dtype=np.uint64)
    plain0r = np.frombuffer(urandom(8*n), dtype=np.uint64)
    plain1l = plain0l ^ diff[0]
    plain1r = plain0r ^ diff[1]
    num_rand_samples = np.sum(Y == 0)
    plain1l[Y == 0] = np.frombuffer(urandom(8*num_rand_samples), dtype=np.uint64)
    plain1r[Y == 0] = np.frombuffer(urandom(8*num_rand_samples), dtype=np.uint64)
    ks = expand_key(keys, nr)
    ctdata0l, ctdata0r = encrypt((plain0l, plain0r), ks)
    ctdata1l, ctdata1r = encrypt((plain1l, plain1r), ks)
    ctdata0y = ror(ctdata0l ^ ctdata0r, BETA())
    ctdata1y = ror(ctdata1l ^ ctdata1r, BETA())
    X = convert_to_binary([ctdata0l ^ ctdata1l, ctdata0y ^ ctdata1y])
    return(X,Y)

if __name__ == '__main__':
    check_testvector()