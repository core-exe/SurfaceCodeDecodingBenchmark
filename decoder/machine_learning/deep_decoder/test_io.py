import numpy as np
from typing import Tuple

n_train = 0
n_valid = 0
n_test = 0

def set_path(path):
    print("set path: {}".format(path))

def set_name(name):
    print("set name: {}".format(name))

def receive_train_data(data: Tuple[np.ndarray, np.ndarray]):
    global n_train
    print("Received train data of shape: {}, {}".format(data[0].shape, data[1].shape))
    n_train += 1

def receive_valid_data(data: Tuple[np.ndarray, np.ndarray]):
    global n_valid
    print("Received valid data of shape: {}, {}".format(data[0].shape, data[1].shape))
    n_valid += 1

def receive_test_data(data: Tuple[np.ndarray, np.ndarray]):
    global n_test
    print("Received test data of shape: {}, {}".format(data[0].shape, data[1].shape))
    print(data[0])
    print(data[1])
    n_test += 1

def init_dataset():
    print("Counter: train = {}, valid = {}, test = {}".format(n_train, n_valid, n_test))

def begin_training():
    print("Training initiated.")

def attach_model(path: str):
    print("Attached model: {}".format(path))

def query_data(data: np.ndarray):
    print("Query data of shape: {}".format(data.shape))
    b, t, x, y = data.shape
    return np.zeros((b, x, y), dtype=np.int32)