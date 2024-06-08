from net import eval_distinguisher
from cipher.simon import Simon

simon = Simon()
simon.verify_test_vectors()

# -----------------------------Simon32/64------------------------------
for num_rounds in [9,10,11]:
    print("Testing {}r ND on Simon32...".format(num_rounds))
    simon = Simon(n_rounds=num_rounds)
    model_path = "./saved_models/Simon32/{}r_distinguisher_dx_x_dy.h5".format(num_rounds)
    eval_distinguisher(simon, [0, 0x40], model_path, 2**24, 1, "dx_x_dy")
# ----------------------------------------------------------------------

# -----------------------------simon64/128------------------------------
for num_rounds in [12,13]:
    print("Testing {}r ND on Simon64...".format(num_rounds))
    simon = Simon(n_rounds=num_rounds, word_size=32, const_seq=3)
    model_path = "./saved_models/Simon64/{}r_distinguisher_dx_x_dy.h5".format(num_rounds)
    eval_distinguisher(simon, [0, 0x1], model_path, 2**24, 1, "dx_x_dy")
# ----------------------------------------------------------------------

# -----------------------------simon128/128------------------------------
num_rounds = 19
print("Testing {}r ND on Simon128...".format(num_rounds))
simon = Simon(n_rounds=num_rounds, word_size=64, const_seq=2, m=2)
model_path = "./saved_models/Simon128/{}r_distinguisher_dx_x_dy.h5".format(num_rounds)
eval_distinguisher(simon, [0, 0x1], model_path, 2**24, 1, "dx_x_dy")
# -----------------------------------------------------------------------