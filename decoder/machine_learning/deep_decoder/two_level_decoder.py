import numpy as np
import torch
import dataset
from typing import Tuple

low_level_datasets = {x: dataset.LowLevelDataset(x) for x in ['train', 'valid']}
high_level_datasets = {x: dataset.HighLevelDataset(x) for x in ['train', 'valid']}

model_path = ""
model_name_low = ""
model_name_high = ""

def set_path(path):
    global model_path
    model_path = path

def set_name(name):
    global model_name_high, model_name_low
    model_name_high = "{}_hi.pth".format(name)
    model_name_low = "{}_lo.pth".format(name)

def receive_train_data(data: Tuple[np.ndarray, np.ndarray]):
    global low_level_datasets, high_level_datasets
    low_level_datasets['train'].insert_data(data)
    high_level_datasets['train'].insert_data(data)

def receive_valid_data(data: Tuple[np.ndarray, np.ndarray]):
    global low_level_datasets, high_level_datasets
    low_level_datasets['valid'].insert_data(data)
    high_level_datasets['valid'].insert_data(data)

def receive_test_data(data: Tuple[np.ndarray, np.ndarray]):
    #global low_level_datasets, high_level_datasets
    #low_level_datasets['test'].insert_data(data)
    #high_level_datasets['test'].insert_data(data)
    pass

def init_dataset():
    global low_level_datasets, high_level_datasets
    for ds in [low_level_datasets, high_level_datasets]:
        for d in ds.values():
            d.prepare()
    for split in ['train', 'valid']:
        print("low level, split: {}, length: {}".format(split, len(low_level_datasets[split])))
        print("high level, split: {}, length: {}".format(split, len(high_level_datasets[split])))

def begin_training():
    print("Training initiated.")

def query_data(data: np.ndarray):
    print("Query data of shape: {}".format(data.shape))
    b, t, x, y = data.shape
    return np.zeros((b, x, y), dtype=np.int32)