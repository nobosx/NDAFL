from os import urandom
import numpy as np

from cipher.abstract_cipher import AbstractCipher


def convert_to_binary(arr, word_size) -> np.ndarray:
    """
    Converts a list of words to an array of bits
    :param arr: Ciphertext pair
    :param word_size: Size of one word (in bits)
    :return: 
    """
    sample_len = len(arr) * word_size
    n_samples = len(arr[0])
    x = np.zeros((sample_len, n_samples), dtype=np.uint8)
    for i in range(sample_len):
        index = i // word_size
        offset = word_size - (i % word_size) - 1
        x[i] = (arr[index] >> offset) & 1
    x = x.transpose()
    return x


def preprocess_samples(ct0, ct1, pt0, pt1, cipher: AbstractCipher, calc_back=0, data_format=None) -> np.ndarray:
    """
    Preprocesses the samples and returns them as an array of bits
    :param ct0: First numpy array of ciphertexts
    :param ct1: Second numpy array of ciphertexts
    :param pt0: Plaintexts corresponding to ct0
    :param pt1: Plaintexts corresponding to ct1
    :param cipher: A cipher object used for backwards calculation
    :param calc_back: The variant of calculating back (0 means not calculating back)
    :param data_format: Only dx_x_dy is allowed here
    """
    if calc_back != 0:
        ct0 = cipher.calc_back(ct0, pt0, calc_back)
        ct1 = cipher.calc_back(ct1, pt1, calc_back)
    if data_format is not None:
        if data_format == "dx_x_dy":
            dx = ct0[0] ^ ct1[0]
            dy = ct0[1] ^ ct1[1]
            return convert_to_binary([dx, ct0[0], dy], cipher.get_word_size())
        else:
            raise Exception("Error: found not supported data format!")
    else:
        return convert_to_binary(np.concatenate((ct0, ct1), axis=0), cipher.get_word_size())


def make_train_data(
        n_samples, cipher, diff, calc_back=0, y=None, additional_conditions=None, data_format=None):
    """
    Generates data for the differential scenario
    :param n_samples: The number of samples
    :param cipher: A cipher object used for encryption
    :param diff: The plaintext difference
    :param calc_back: The variant of calculating back (0 means not calculating back)
    :param y: The label to use for all data. 'None' means random labels
    :param data_format: Only dx_x_dy is allowed here
    :return: Training/validation samples
    """
    # generate labels
    if y is None:
        y = np.frombuffer(urandom(n_samples), dtype=np.uint8) & 1
    elif y == 0 or y == 1:
        y = np.array([y for _ in range(n_samples)], dtype=np.uint8)
    # draw keys and plaintexts
    keys = cipher.draw_keys(n_samples)
    pt0 = cipher.draw_plaintexts(n_samples)
    if additional_conditions is not None:
        pt0 = additional_conditions(pt0)
    pt1 = pt0 ^ np.array(diff, dtype=cipher.word_dtype)[:, np.newaxis]
    # replace plaintexts in pt1 with random ones if label is 0
    num_rand_samples = np.sum(y == 0)
    pt1[:, y == 0] = cipher.draw_plaintexts(num_rand_samples)
    # encrypt
    ct0 = cipher.encrypt(pt0, keys)
    ct1 = cipher.encrypt(pt1, keys)
    # perform backwards calculation and other preprocessing
    x = preprocess_samples(ct0, ct1, pt0, pt1, cipher, calc_back, data_format)
    return x, y