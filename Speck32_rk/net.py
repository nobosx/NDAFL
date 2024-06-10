from tensorflow.keras.regularizers import l2
from tensorflow.keras.backend import concatenate
from tensorflow.keras.layers import Dense, Conv1D,Input, Reshape, Permute, Add, Flatten, BatchNormalization, Activation
from tensorflow.keras.models import Model, load_model
from pickle import dump
from tensorflow.keras import backend as K
import numpy as np
import os
import gc
import speck_rk as sp
os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'
os.environ["TF_FORCE_GPU_ALLOW_GROWTH"] = "true"

bs = 5000
word_size = sp.WORD_SIZE()

def make_resnet(num_words=6, num_filters=16, num_outputs=1, d1=64, d2=64, word_size=16, ks=3, depth=5, reg_param=0.0001, final_activation='sigmoid'):
    inp = Input(shape=(num_words * word_size,))
    rs = Reshape((num_words,  word_size))(inp)
    perm = Permute((2, 1))(rs)
    conv01 = Conv1D(num_filters, kernel_size=1, padding='same',
                    kernel_regularizer=l2(reg_param))(perm)
    conv02 = Conv1D(num_filters, kernel_size=2, padding='same',
                    kernel_regularizer=l2(reg_param))(perm)
    conv03 = Conv1D(num_filters, kernel_size=7, padding='same',
                    kernel_regularizer=l2(reg_param))(perm)
    c2 = concatenate([conv01, conv02, conv03], axis=-1)
    conv0 = BatchNormalization()(c2)
    conv0 = Activation('relu')(conv0)
    shortcut = conv0
    for i in range(depth):
        conv1 = Conv1D(num_filters*3, kernel_size=ks, padding='same',
                       kernel_regularizer=l2(reg_param))(shortcut)
        conv1 = BatchNormalization()(conv1)
        conv1 = Activation('relu')(conv1)
        conv2 = Conv1D(num_filters*3, kernel_size=ks,
                       padding='same', kernel_regularizer=l2(reg_param))(conv1)
        conv2 = BatchNormalization()(conv2)
        conv2 = Activation('relu')(conv2)
        shortcut = Add()([shortcut, conv2])
        if ks < 7:
            ks = ks + 2
    dense0 = Flatten()(shortcut)
    dense1 = Dense(d1, kernel_regularizer=l2(reg_param))(dense0)
    dense1 = BatchNormalization()(dense1)
    dense1 = Activation('relu')(dense1)
    dense2 = Dense(d2, kernel_regularizer=l2(reg_param))(dense1)
    dense2 = BatchNormalization()(dense2)
    dense2 = Activation('relu')(dense2)
    out = Dense(num_outputs, activation=final_activation,
                kernel_regularizer=l2(reg_param))(dense2)
    model = Model(inputs=inp, outputs=out)
    return(model)


def train_speck_distinguisher(num_epochs, num_rounds, diff, mk_diff, rk_diff_trail, depth, saved_folder):
    batch_size = bs
    net = make_resnet(depth=depth, reg_param=0, num_words=4)
    net.compile(optimizer='adam', loss='mse', metrics=['acc']) 

    X_eval, Y_eval = sp.make_train_data(2**20, num_rounds, diff, mk_diff, rk_diff_trail)
    saved_path = saved_folder + "{}r_distinguisher.h5".format(num_rounds)
    lr = np.linspace(3.5e-3, 2e-4, 10)
    for i in range(num_epochs):
        print("Epoch ",i)
        K.set_value(net.optimizer.learning_rate, lr[i%10])
        X, Y = sp.make_train_data(2**24, num_rounds, diff, mk_diff, rk_diff_trail)
        #train and evaluate
        h = net.fit(X, Y, epochs=1, batch_size=batch_size,
                    validation_data=(X_eval, Y_eval), shuffle=True)
        net.save(saved_path)
        del X,Y
        gc.collect()
    print("Final validation accuracy: ", np.max(h.history['val_acc']))

def train_speck_distinguisher_with_former_net(num_epochs, num_rounds, depth, diff, mk_diff, rk_diff_trail, former_net_path, saved_folder):
    batch_size = bs
    net = load_model(former_net_path)
    net.compile(optimizer='adam', loss='mse', metrics=['acc'])

    X_eval, Y_eval = sp.make_train_data(2**20, num_rounds, diff, mk_diff, rk_diff_trail)
    saved_path = saved_folder + "{}r_distinguisher.h5".format(num_rounds)
    lr = np.linspace(3.5e-3, 2e-4, 10)
    for i in range(num_epochs):
        print("Epoch ",i)
        K.set_value(net.optimizer.learning_rate, lr[i%10])
        X, Y = sp.make_train_data(2**24, num_rounds, diff, mk_diff, rk_diff_trail)
        #train and evaluate
        h = net.fit(X, Y, epochs=1, batch_size=batch_size,
                    validation_data=(X_eval, Y_eval), shuffle=True)
        net.save(saved_path)
        del X,Y
        gc.collect()
    print("Final validation accuracy: ", np.max(h.history['val_acc']))

def eval_distinguisher(n, model_path, num_rounds, diff, mk_diff, rk_diff_trail):
    net = load_model(model_path)
    X_eval, Y_eval = sp.make_train_data(n, num_rounds, diff, mk_diff, rk_diff_trail)
    prediction = net.predict(X_eval, batch_size=10000).flatten() > 0.5
    right_pos = Y_eval == prediction
    acc = np.sum(right_pos) / n
    tpr = np.sum(right_pos & (Y_eval == 1)) / np.sum(Y_eval == 1)
    tnr = np.sum(right_pos & (Y_eval == 0)) / np.sum(Y_eval == 0)
    print("acc is:", acc, ",tpr is:", tpr, ",tnr is:", tnr)
