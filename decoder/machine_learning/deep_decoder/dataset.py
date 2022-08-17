import torch
from torch.utils.data.dataset import Dataset

class LowLevelDataset(Dataset):
    # input: syndrome label: correction operation
    def __init__(self, split = 'train') -> None:
        super().__init__()
        self.syndrome_list = []
        self.error_list = []
        self.length = 0
        self.split = split
    
    def insert_data(self, data):
        # data: ([B, C, H, W], [B, H, W])
        syndrome, error = data
        self.syndrome_list.append(torch.from_numpy(syndrome))
        self.error_list.append(torch.from_numpy(error))

    def prepare(self):
        print(self.split)
        print("syndrome_list length:")
        print(self.syndrome_list.__len__())
        print("error_list length:")
        print(self.error_list.__len__())
        self.syndrome_tensor = torch.cat(self.syndrome_list, dim = 0)
        self.error_tensor = torch.cat(self.error_list, dim = 0)
        self.length = self.error_tensor.size(0)
    
    def __len__(self):
        return self.length
    
    def __getitem__(self, index):
        return (self.syndrome_tensor[index,...], self.error_tensor[index,...])

class HighLevelDataset(Dataset):
    # input: syndrome label: correction operation
    def __init__(self, split = 'train') -> None:
        super().__init__()
        self.syndrome_list = []
        self.length = 0
        self.split = split
    
    def insert_data(self, data):
        self.syndrome_list.append(torch.from_numpy(data[0]))
    
    def prepare(self):
        print(self.split)
        print("syndrome_list length:")
        print(self.syndrome_list.__len__())
        self.syndrome_tensor = torch.cat(self.syndrome_list, dim = 0)
        self.length = self.syndrome_tensor.size(0)
    
    def __len__(self):
        return self.length
    
    def get_syndrome_tensor(self):
        return self.syndrome_tensor

    def add_logical_error(self, logical_error):
        self.error_tensor = logical_error
    
    def __getitem__(self, index):
        return (self.syndrome_tensor[index,...], self.error_tensor[index])

def tensor_cat_collate_fn(datas):
    tuple_length = len(datas[0])
    ls = [[] for _ in range(tuple_length)]
    for t in datas:
        for i in range(tuple_length):
            ls[i].append(t[i])
    return tuple((torch.cat(ls[i], dim=0) for i in range(tuple_length)))