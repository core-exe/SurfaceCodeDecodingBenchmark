import numpy as np
import torch
import dataset
import model
from typing import Tuple
from torch.utils.data.dataloader import DataLoader
from tqdm import tqdm
from torch.optim import Adam
import deep_decoder_util

low_level_datasets = {x: dataset.LowLevelDataset(x) for x in ['train', 'valid']}
high_level_datasets = {x: dataset.HighLevelDataset(x) for x in ['train', 'valid']}

working_device = "cuda:0"

model_path : str
model_name_low : str
model_name_high : str

low_level_dataloaders : "dict[str, DataLoader]"
high_level_dataloaders : "dict[str, DataLoader]"

low_level_model: model.TwoLevelLLD
high_level_model: model.TwoLevelHLD

low_level_epoch = 100
low_level_lr = 2e-4
low_level_batch = 100

high_level_epoch = 100
high_level_lr = 2e-4
high_level_batch = 100

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
    #use query_data function to do testing
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
    global low_level_dataloaders, low_level_model, high_level_dataloaders, high_level_model
    print("Training low level decoder.")
    low_level_dataloaders = {split: DataLoader(
        low_level_datasets[split],
        low_level_batch,
        True,
        collate_fn=dataset.tensor_stack_collate_fn
    ) for split in ['train', 'valid']}
    low_level_model = model.TwoLevelLLD(low_level_datasets['train'][0][0].size(0))
    low_level_model.to_device(working_device)
    low_level_optimizer = Adam(low_level_model.parameters(), low_level_lr)
    low_best_acc = 0
    for n_epoch in range(low_level_epoch):
        # train
        pbar = tqdm(low_level_dataloaders['train'])
        pbar.set_description("Low  | Epoch {:03d} | Loss {} |".format(n_epoch, "------"))
        loss_total = 0
        batch_counter = 0
        for input, label in pbar:
            loss = low_level_model.get_loss(low_level_model(input), label)
            low_level_optimizer.zero_grad()
            loss.backward()
            low_level_optimizer.step()
            loss_total += float(loss.cpu())
            batch_counter += 1
            pbar.set_description("Train Low  | Epoch {:03d} | Loss {:.6f} |".format(n_epoch, loss_total / batch_counter))
        pbar.close()
        # valid
        pbar = tqdm(low_level_dataloaders['valid'])
        total_num = 0
        correct_num = 0
        loss_total = 0
        batch_counter = 0
        pbar.set_description("Valid Low  | Epoch {:03d} | Loss {} | Acc {}".format(n_epoch, "------", "-----"))
        for input, label in pbar:
            logit = low_level_model(input) #(B, 4, X, Y)
            loss = low_level_model.get_loss(logit, label)
            _, idx = torch.topk(logit, 1, 1)
            correction = idx.squeeze(1).detach().cpu().numpy() #(B, X, Y)
            is_valid = deep_decoder_util.is_valid(deep_decoder_util.apply_physical_correction(
                label.detach().cpu().numpy(),
                correction
            ))
            total_num += is_valid.shape[0]
            correct_num += int(np.sum(is_valid))
            loss_total += float(loss.detach().cpu())
            batch_counter += 1
            pbar.set_description("Valid Low  | Epoch {:03d} | Loss {:.6f} | Acc {:.5f}".format(n_epoch, loss_total / batch_counter, correct_num / total_num))
        pbar.close()
        acc = correct_num / total_num
        if(acc > low_best_acc):
            low_best_acc = acc
            torch.save(low_level_model, model_path + "/" + model_name_low)

    print("Initializing high level datasets.")
    for d in high_level_datasets.values():
        length = len(d)
        high_level_label_list = []
        for i in range(int(length / low_level_batch)):
            input = d.syndrome_tensor[i * low_level_batch : (i + 1) * low_level_batch, ...]
            logit = low_level_model(input) #(B, 4, X, Y)
            _, idx = torch.topk(logit, 1, 1)
            correction = idx.squeeze(1).detach().cpu().numpy() #(B, X, Y)
            logical_error = deep_decoder_util.get_logical_error(deep_decoder_util.apply_physical_correction(
                label.detach().cpu().numpy(),
                correction
            )) #(B, )
            high_level_label_list.append(torch.from_numpy(logical_error).long())
        d.add_logical_error(torch.cat(high_level_label_list, dim=0))

    print("Training high level decoder.")
    high_level_dataloaders = {split: DataLoader(
        high_level_datasets[split],
        high_level_batch,
        True,
        collate_fn=dataset.tensor_stack_collate_fn
    ) for split in ['train', 'valid']}
    high_level_model = model.TwoLevelHLD(
        high_level_datasets['train'][0][0].size(0),
        high_level_datasets['train'][0][0].size(1),
        high_level_datasets['train'][0][0].size(2)
    )
    high_level_model.to_device(working_device)
    high_level_optimizer = Adam(high_level_model.parameters(), high_level_lr)
    high_best_acc = 0
    for n_epoch in range(high_level_epoch):
        #train
        pbar = tqdm(high_level_dataloaders['train'])
        pbar.set_description("Train High | Epoch {:03d} | Loss {} |".format(n_epoch, "------"))
        loss_total = 0
        batch_counter = 0
        for input, label in pbar:
            loss = high_level_model.get_loss(high_level_model(input), label)
            high_level_optimizer.zero_grad()
            loss.backward()
            high_level_optimizer.step()
            loss_total += float(loss.cpu())
            batch_counter += 1
            pbar.set_description("Train High | Epoch {:03d} | Loss {:.6f} |".format(n_epoch, loss_total / batch_counter))
        pbar.close()
        # valid
        pbar = tqdm(high_level_dataloaders['valid'])
        total_num = 0
        correct_num = 0
        loss_total = 0
        batch_counter = 0
        pbar.set_description("Valid High | Epoch {:03d} | Loss {} | Acc {}".format(n_epoch, "------", "-----"))
        for input, label in pbar:
            logit = high_level_model(input) #(B, 4, X, Y)
            loss = high_level_model.get_loss(logit, label)
            _, idx = torch.topk(logit, 1, 1) #idx: (B, 1)
            idx = idx.squeeze(1).cpu()
            total_num += idx.size(0)
            correct_num += int(torch.sum((idx == label), dim=0).detach().cpu())
            loss_total += float(loss.detach().cpu())
            batch_counter += 1
            pbar.set_description("Valid High | Epoch {:03d} | Loss {:.6f} | Acc {:.5f}".format(n_epoch, loss_total / batch_counter, correct_num / total_num))
        pbar.close()
        acc = correct_num / total_num
        if(acc > high_best_acc):
            high_best_acc = acc
            torch.save(high_level_model, model_path + "/" + model_name_high)

def query_data(data: np.ndarray):
    print("Query data of shape: {}".format(data.shape))
    b, t, x, y = data.shape
    return np.zeros((b, x, y), dtype=np.int32)