import lea
import numpy as np

from pickle import dump

from keras.callbacks import ModelCheckpoint, LearningRateScheduler
from keras.models import Model, load_model
from keras.layers import Dense, Conv1D, Input, Reshape, Permute, Add, Flatten, BatchNormalization, Activation
from keras.regularizers import l2
from keras.optimizers import Adam

bs = 5000
wdir = './freshly_trained_nets/'

def cyclic_lr(num_epochs, high_lr, low_lr):
    res = lambda i: low_lr + ((num_epochs-1) - i % num_epochs)/(num_epochs-1) * (high_lr - low_lr)
    return(res)

def make_checkpoint(datei):
    res = ModelCheckpoint(datei, monitor='val_loss', save_best_only = True)
    return(res)

#make residual tower of convolutional blocks
def make_resnet(num_blocks=2, num_filters=32, num_outputs=1, d1=64, d2=64, word_size=32, ks=3,depth=5, reg_param=0.0001, final_activation='sigmoid'):
    #Input and preprocessing layers
    inp = Input(shape=(num_blocks * word_size * 4,))
    rs = Reshape((4 * num_blocks, word_size))(inp)
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
    return model

def train_distinguisher(num_epochs, diff, num_rounds, saved_folder, depth=1, key_bit_length=128, data_format="x_x\'", former_net_path=None, calc_back=False):
    #create the network
    if former_net_path is not None:
      net = load_model(former_net_path)
    else:
      net = make_resnet(depth=depth, reg_param=10**-5)
      net.compile(optimizer='adam',loss='mse',metrics=['acc'])
    #generate training and validation data
    X, Y = lea.make_train_data(10**7, num_rounds, diff, key_bit_length, data_format=data_format, calc_back=calc_back)
    X_eval, Y_eval = lea.make_train_data(10**6, num_rounds, diff, key_bit_length, data_format=data_format, calc_back=calc_back)
    #set up model checkpoint
    check = make_checkpoint(wdir+'best'+str(num_rounds)+'depth'+str(depth)+'.h5')
    #create learnrate schedule
    lr = LearningRateScheduler(cyclic_lr(10,0.002, 0.0001))
    #train and evaluate
    h = net.fit(X,Y,epochs=num_epochs,batch_size=bs,validation_data=(X_eval, Y_eval), callbacks=[lr,check])
    dump(h.history, open(wdir+'hist'+str(num_rounds)+'r_depth'+str(depth)+'.p','wb'))
    print("Best validation accuracy: ", np.max(h.history['val_acc']))
    net.save(saved_folder + '{}r_distinguisher_{}.h5'.format(num_rounds, data_format))
    return (net, h)

def train_distinguisher_with_only_diff(num_epochs, diff, num_rounds, saved_folder, depth=1, key_bit_length=128, former_net_path=None, calc_back=False):
    #create the network
    if former_net_path is not None:
      net = load_model(former_net_path)
    else:
      net = make_resnet(depth=depth, reg_param=10**-5, num_blocks=1)
      net.compile(optimizer='adam',loss='mse',metrics=['acc'])
    #generate training and validation data
    X, Y = lea.make_train_data(10**7, num_rounds, diff, key_bit_length, data_format="dx_x", calc_back=calc_back)
    X_with_only_diff = X[:, :lea.BLOCK_SIZE]
    X_eval, Y_eval = lea.make_train_data(10**6, num_rounds, diff, key_bit_length, data_format="dx_x", calc_back=calc_back)
    X_eval_with_only_diff = X_eval[:, :lea.BLOCK_SIZE]
    #set up model checkpoint
    check = make_checkpoint(wdir+'best'+str(num_rounds)+'depth'+str(depth)+'.h5')
    #create learnrate schedule
    lr = LearningRateScheduler(cyclic_lr(10,0.002, 0.0001))
    #train and evaluate
    h = net.fit(X_with_only_diff,Y,epochs=num_epochs,batch_size=bs,validation_data=(X_eval_with_only_diff, Y_eval), callbacks=[lr,check])
    dump(h.history, open(wdir+'hist'+str(num_rounds)+'r_depth'+str(depth)+'.p','wb'))
    print("Best validation accuracy: ", np.max(h.history['val_acc']))
    net.save(saved_folder + '{}r_distinguisher_only_diff.h5'.format(num_rounds))
    return (net, h)

def eval_distinguisher(n, num_rounds, diff, model_path, data_format="x_x\'", calc_back=False):
    net = load_model(model_path)
    X, Y = lea.make_train_data(n, num_rounds, diff, data_format=data_format, calc_back=calc_back)
    predict = net.predict(X, batch_size=10000).flatten() > 0.5
    acc = np.sum(predict == Y) / n
    tpr = np.sum((predict == Y) & (Y == 1)) / np.sum(Y == 1)
    tnr = np.sum((predict == Y) & (Y == 0)) / np.sum(Y == 0)
    print("acc is {}, tpr is {}, tnr is {}.".format(acc, tpr, tnr))

def eval_distinguisher_with_only_diff(n, num_rounds, diff, model_path, calc_back=False):
    net = load_model(model_path)
    X, Y = lea.make_train_data(n, num_rounds, diff, data_format="dx_x", calc_back=calc_back)
    X = X[:, :lea.BLOCK_SIZE]
    predict = net.predict(X, batch_size=10000).flatten() > 0.5
    acc = np.sum(predict == Y) / n
    tpr = np.sum((predict == Y) & (Y == 1)) / np.sum(Y == 1)
    tnr = np.sum((predict == Y) & (Y == 0)) / np.sum(Y == 0)
    print("acc is {}, tpr is {}, tnr is {}.".format(acc, tpr, tnr))