import net

# ID1
diff = (0x80, 0)
diff_index = 55
saved_folder = "./saved_models/{}_".format(diff_index)
# Train neural distinguishers NDs
num_rounds = 7
net.train_speck_distinguisher(num_epochs=10, num_rounds=num_rounds, depth=5, diff=diff, folder=saved_folder)
num_rounds = 8
net.train_speck_distinguisher_with_former_net(num_epochs=10, num_rounds=num_rounds, depth=5, diff=diff, folder=saved_folder)
# Train neural differential distinguishers NDDs
num_rounds = 7
net.train_speck_distinguisher_only_diff(num_epochs=10, num_rounds=num_rounds, depth=5, diff=diff, folder=saved_folder)
num_rounds = 8
net.train_speck_distinguisher_only_diff_with_former_net(num_epochs=10, num_rounds=num_rounds, depth=5, diff=diff, folder=saved_folder)

# ID2
diff = (0x80, 0x800000000000)
diff_index = (47, 55)
saved_folder = "./saved_models/{}_".format(diff_index)
# Train neural distinguishers NDs
num_rounds = 7
net.train_speck_distinguisher(num_epochs=10, num_rounds=num_rounds, depth=5, diff=diff, folder=saved_folder)
num_rounds = 8
net.train_speck_distinguisher_with_former_net(num_epochs=10, num_rounds=num_rounds, depth=5, diff=diff, folder=saved_folder)
num_rounds = 9
net.train_speck_distinguisher_with_former_net(num_epochs=10, num_rounds=num_rounds, depth=5, diff=diff, folder=saved_folder)
# Train neural differential distinguishers NDDs
num_rounds = 7
net.train_speck_distinguisher_only_diff(num_epochs=10, num_rounds=num_rounds, depth=5, diff=diff, folder=saved_folder)
num_rounds = 8
net.train_speck_distinguisher_only_diff_with_former_net(num_epochs=10, num_rounds=num_rounds, depth=5, diff=diff, folder=saved_folder)
num_rounds = 9
net.train_speck_distinguisher_only_diff_with_former_net(num_epochs=10, num_rounds=num_rounds, depth=5, diff=diff, folder=saved_folder)

# ID3
diff = (0, 0x800000000000)
diff_index = 47
saved_folder = "./saved_models/{}_".format(diff_index)
# Train neural distinguishers NDs
num_rounds = 7
net.train_speck_distinguisher(num_epochs=10, num_rounds=num_rounds, depth=5, diff=diff, folder=saved_folder)
num_rounds = 8
net.train_speck_distinguisher_with_former_net(num_epochs=10, num_rounds=num_rounds, depth=5, diff=diff, folder=saved_folder)
# Train neural differential distinguishers NDDs
num_rounds = 7
net.train_speck_distinguisher_only_diff(num_epochs=10, num_rounds=num_rounds, depth=5, diff=diff, folder=saved_folder)
num_rounds = 8
net.train_speck_distinguisher_only_diff_with_former_net(num_epochs=10, num_rounds=num_rounds, depth=5, diff=diff, folder=saved_folder)