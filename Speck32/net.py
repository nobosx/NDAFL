import speck as sp
import gc
import numpy as np

from pickle import dump

from keras.callbacks import ModelCheckpoint, LearningRateScheduler
from keras.models import Model, load_model
from keras.optimizers import Adam
from keras.layers import Dense, Conv1D, Input, Reshape, Permute, Add, Flatten, BatchNormalization, Activation
from keras import backend as K
from keras.regularizers import l2

bs = 5000
wdir = './freshly_trained_nets/'


def extract_selected_bits(X, IBs):
    data_width = X.shape[1]
    extracted_X = np.zeros(X.shape, dtype=np.uint8)
    for i in IBs:
        extracted_X[:, data_width - 1 - i] = X[:, data_width - 1 - i]
    return extracted_X

def cyclic_lr(num_epochs, high_lr, low_lr):
    res = lambda i: low_lr + ((num_epochs-1) - i % num_epochs)/(num_epochs-1) * (high_lr - low_lr);
    return(res)


def make_checkpoint(datei):
    res = ModelCheckpoint(datei, monitor='val_loss', save_best_only = True);
    return(res)


#make residual tower of convolutional blocks
def make_resnet(num_words=4, num_filters=32, num_outputs=1, d1=64, d2=64, word_size=16, ks=3, depth=5, reg_param=0.0001, final_activation='sigmoid'):
    #Input and preprocessing layers
    inp = Input(shape=(num_words * word_size,))
    rs = Reshape((num_words, word_size))(inp)
    perm = Permute((2,1))(rs)
    #add a single residual layer that will expand the data to num_filters channels
    #this is a bit-sliced layer
    conv0 = Conv1D(num_filters, kernel_size=1, padding='same', kernel_regularizer=l2(reg_param))(perm)
    conv0 = BatchNormalization()(conv0)
    conv0 = Activation('relu')(conv0)
    #add residual blocks
    shortcut = conv0
    for i in range(depth):
        conv1 = Conv1D(num_filters, kernel_size=ks, padding='same', kernel_regularizer=l2(reg_param))(shortcut)
        conv1 = BatchNormalization()(conv1)
        conv1 = Activation('relu')(conv1)
        conv2 = Conv1D(num_filters, kernel_size=ks, padding='same',kernel_regularizer=l2(reg_param))(conv1)
        conv2 = BatchNormalization()(conv2)
        conv2 = Activation('relu')(conv2)
        shortcut = Add()([shortcut, conv2])
    #add prediction head
    flat1 = Flatten()(shortcut)
    dense1 = Dense(d1,kernel_regularizer=l2(reg_param))(flat1)
    dense1 = BatchNormalization()(dense1)
    dense1 = Activation('relu')(dense1)
    dense2 = Dense(d2, kernel_regularizer=l2(reg_param))(dense1)
    dense2 = BatchNormalization()(dense2)
    dense2 = Activation('relu')(dense2)
    out = Dense(num_outputs, activation=final_activation, kernel_regularizer=l2(reg_param))(dense2)
    model = Model(inputs=inp, outputs=out)
    return(model)


def train_speck_distinguisher(num_epochs, num_rounds=7, depth=1, diff=(0x808000, 0x808004), folder='./', data_form='l_r'):
    #create the network
    num_words = 3 if data_form == 'dl_dy_y' else 4
    net = make_resnet(num_words=num_words, depth=depth, reg_param=10**-5)
    net.compile(optimizer='adam', loss='mse', metrics=['acc'])

    X, Y = sp.make_train_data(10**7, num_rounds, diff, data_form)
    X_eval, Y_eval = sp.make_train_data(10**6, num_rounds, diff, data_form)

    #set up model checkpoint
    check = make_checkpoint(wdir+'best'+str(num_rounds)+'depth'+str(depth)+'.h5')
    #create learnrate schedule
    lr = LearningRateScheduler(cyclic_lr(10, 0.002, 0.0001))
    #train and evaluate
    h = net.fit(X, Y, epochs=num_epochs, batch_size=bs, validation_data=(X_eval, Y_eval), shuffle=True, callbacks=[lr, check])
    np.save(wdir+'h'+str(num_rounds)+'r_depth'+str(depth)+'.npy', h.history['val_acc'])
    np.save(wdir+'h'+str(num_rounds)+'r_depth'+str(depth)+'.npy', h.history['val_loss'])
    dump(h.history, open(wdir+'hist'+str(num_rounds)+'r_depth'+str(depth)+'.p','wb'))
    print("Best validation accuracy: ", np.max(h.history['val_acc']))

    # save model
    net.save(folder + '{}r_distinguisher_{}.h5'.format(num_rounds, data_form))

    return(net, h)

def train_speck_distinguisher_using_stage_training(num_epochs, num_rounds, num_pre_rounds, diff, auxiliary_diff, folder, data_form):
    net = load_model(folder + "{}r_distinguisher_{}.h5".format(num_rounds - 1, data_form))

    # stage one
    net.compile(optimizer=Adam(10**-4), loss='mse', metrics=['acc'])
    X, Y = sp.make_train_data(10**7, num_rounds - num_pre_rounds, auxiliary_diff, data_form)
    X_eval, Y_eval = sp.make_train_data(10**6, num_rounds - num_pre_rounds, auxiliary_diff, data_form)
    h = net.fit(X, Y, epochs=num_epochs, batch_size=bs, validation_data=(X_eval, Y_eval), shuffle=True)
    net.save("stage_one_net.h5")

    # stage two
    net = load_model("stage_one_net.h5")
    net.compile(optimizer=Adam(10**-4), loss='mse', metrics=['acc'])
    X, Y = sp.make_train_data(10**8, num_rounds, diff, data_form)
    X_eval, Y_eval = sp.make_train_data(10**6, num_rounds, diff, data_form)
    h = net.fit(X, Y, epochs=num_epochs, batch_size=bs, validation_data=(X_eval, Y_eval), shuffle=True)
    net.save("stage_two_net.h5")
    
    gc.collect()

    # stage three
    net = load_model("stage_two_net.h5")
    net.compile(optimizer=Adam(10**-5), loss='mse', metrics=['acc'])
    X, Y = sp.make_train_data(10**8, num_rounds, diff, data_form)
    X_eval, Y_eval = sp.make_train_data(10**6, num_rounds, diff, data_form)
    h = net.fit(X, Y, epochs=num_epochs, batch_size=bs, validation_data=(X_eval, Y_eval), shuffle=True)
    print("Best validation accuracy: ", np.max(h.history['val_acc']))
    # save model
    net.save(folder + '{}r_distinguisher_{}.h5'.format(num_rounds, data_form))

def eval_distinguisher(n, num_rounds, diff, net_path, data_form="l_r"):
    net = load_model(net_path, compile=False)
    X, Y = sp.make_train_data(n, num_rounds, diff, data_form)
    predict = net.predict(X, batch_size=10000, verbose=0).flatten() > 0.5
    true_pos = Y == predict
    acc = np.sum(true_pos) / n
    tpr = np.sum(true_pos & (Y == 1)) / np.sum(Y == 1)
    tnr = np.sum(true_pos & (Y == 0)) / np.sum(Y == 0)
    print("Acc is {}, tpr is {}, tnr is {}.".format(acc, tpr, tnr))

def train_auxiliary_distinguisher(num_epochs, num_rounds, depth, diff, folder, IBs, data_form='dl_l_dy_y'):
    #create the network
    num_words = 3 if data_form == 'dl_dy_y' else 4
    net = make_resnet(num_words=num_words, depth=depth, reg_param=10**-5)
    net.compile(optimizer='adam', loss='mse', metrics=['acc'])

    X, Y = sp.make_train_data(10**7, num_rounds, diff, data_form)
    X = extract_selected_bits(X, IBs)
    X_eval, Y_eval = sp.make_train_data(10**6, num_rounds, diff, data_form)
    X_eval = extract_selected_bits(X_eval, IBs)

    #set up model checkpoint
    check = make_checkpoint(wdir+'best'+str(num_rounds)+'depth'+str(depth)+'.h5')
    #create learnrate schedule
    lr = LearningRateScheduler(cyclic_lr(10, 0.002, 0.0001))
    #train and evaluate
    h = net.fit(X, Y, epochs=num_epochs, batch_size=bs, validation_data=(X_eval, Y_eval), shuffle=True, callbacks=[lr, check])
    np.save(wdir+'h'+str(num_rounds)+'r_depth'+str(depth)+'.npy', h.history['val_acc'])
    np.save(wdir+'h'+str(num_rounds)+'r_depth'+str(depth)+'.npy', h.history['val_loss'])
    dump(h.history, open(wdir+'hist'+str(num_rounds)+'r_depth'+str(depth)+'.p','wb'))
    print("Best validation accuracy: ", np.max(h.history['val_acc']))

    # save model
    net.save(folder + '{}r_auxiliary_distinguisher_{}.h5'.format(num_rounds, data_form))

    return(net, h)

def eval_auxiliary_distinguisher(n, num_rounds, diff, net_path, IBs, data_form='dl_l_dy_y'):
    net = load_model(net_path, compile=False)
    X, Y = sp.make_train_data(n, num_rounds, diff, data_form)
    X = extract_selected_bits(X, IBs)
    predict = net.predict(X, batch_size=10000, verbose=0).flatten() > 0.5
    true_pos = Y == predict
    acc = np.sum(true_pos) / n
    tpr = np.sum(true_pos & (Y == 1)) / np.sum(Y == 1)
    tnr = np.sum(true_pos & (Y == 0)) / np.sum(Y == 0)
    print("Acc is {}, tpr is {}, tnr is {}.".format(acc, tpr, tnr))

def train_auxiliary_distinguisher_using_stage_training(num_epochs, num_rounds, num_pre_rounds, diff, auxiliary_diff, folder, IBs, data_form):
    net = load_model(folder + "{}r_auxiliary_distinguisher_{}.h5".format(num_rounds - 1, data_form), compile=False)

    # stage one
    net.compile(optimizer=Adam(10**-4), loss='mse', metrics=['acc'])
    X, Y = sp.make_train_data(10**7, num_rounds - num_pre_rounds, auxiliary_diff, data_form)
    X = extract_selected_bits(X, IBs)
    X_eval, Y_eval = sp.make_train_data(10**6, num_rounds - num_pre_rounds, auxiliary_diff, data_form)
    X_eval = extract_selected_bits(X_eval, IBs)
    h = net.fit(X, Y, epochs=num_epochs, batch_size=bs, validation_data=(X_eval, Y_eval), shuffle=True)
    net.save("stage_one_net.h5")

    # stage two
    net = load_model("stage_one_net.h5")
    net.compile(optimizer=Adam(10**-4), loss='mse', metrics=['acc'])
    X, Y = sp.make_train_data(10**8, num_rounds, diff, data_form)
    X = extract_selected_bits(X, IBs)
    X_eval, Y_eval = sp.make_train_data(10**6, num_rounds, diff, data_form)
    X_eval = extract_selected_bits(X_eval, IBs)
    h = net.fit(X, Y, epochs=num_epochs, batch_size=bs, validation_data=(X_eval, Y_eval), shuffle=True)
    net.save("stage_two_net.h5")
    
    gc.collect()

    # stage three
    net = load_model("stage_two_net.h5")
    net.compile(optimizer=Adam(10**-5), loss='mse', metrics=['acc'])
    X, Y = sp.make_train_data(10**8, num_rounds, diff, data_form)
    X = extract_selected_bits(X, IBs)
    X_eval, Y_eval = sp.make_train_data(10**6, num_rounds, diff, data_form)
    X_eval = extract_selected_bits(X_eval, IBs)
    h = net.fit(X, Y, epochs=num_epochs, batch_size=bs, validation_data=(X_eval, Y_eval), shuffle=True)
    print("Best validation accuracy: ", np.max(h.history['val_acc']))
    # save model
    net.save(folder + '{}r_auxiliary_distinguisher_{}.h5'.format(num_rounds, data_form))