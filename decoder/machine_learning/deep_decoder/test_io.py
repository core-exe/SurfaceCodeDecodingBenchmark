import numpy as np

n_train = 0
n_valid = 0
n_test = 0

def receive_train_data(data: np.ndarray):
    print("Received train data of shape: {}".format(data.shape))
    n_train += 1

def receive_valid_data(data: np.ndarray):
    print("Received valid data of shape: {}".format(data.shape))
    n_valid += 1

def receive_test_data(data: np.ndarray):
    print("Received test data of shape: {}".format(data.shape))
    n_test += 1

def init_dataset():
    print("Counter: train = {}, valid = {}, test = {}".format(n_train, n_valid, n_test))

def begin_training():
    print("Training initiated.")

def attach_model(path: str):
    print("Attached model: {}".format(path))

def query_data(data: np.ndarray):
    print("Query data of shape: {}".format(data.shape))