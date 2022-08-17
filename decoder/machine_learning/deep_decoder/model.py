import torch
import torch.nn as nn
import torch.nn.functional as F
import deep_decoder_util

class TwoLevelLLD(nn.Module):
    def __init__(self, in_channel: int) -> None:
        super().__init__()
        self.in_channel = in_channel
        self.conv1 = nn.Conv2d(in_channel, 16, (5, 5), (2, 2))
        self.conv2 = nn.Conv2d(16, 16, (5, 5), (2, 2))
        self.conv3 = nn.Conv3d(16, 4, (3, 3), (1, 1))
        self.relu = nn.Relu()
        self.device = "cpu"
    
    def to_device(self, device):
        self.device = device
        self.to(device)
    
    def forward(self, x):
        out = self.conv1(x)
        out = self.relu(out)
        out = self.conv2(out)
        out = self.relu(out)
        out = self.conv3(out)
        return out
    
    def get_loss(self, logit, label):
        # logit: (B, 4, X, Y)
        # label: (B, X, Y)
        x = logit.size(1)
        y = logit.size(2)
        type = torch.from_numpy(deep_decoder_util.qubit_type(x, y)).to(self.device)
        return nn.CrossEntropyLoss(ignore_index=(type==1)*(-100))(logit, label)

class TwoLevelHLD(nn.Module):
    def __init__(self, in_channel: int, x: int, y: int):
        super().__init__()
        self.in_channel = in_channel
        self.x = x
        self.y = y
        self.in_channel = in_channel
        self.conv1 = nn.Conv2d(in_channel, 16, (5, 5), (2, 2))
        self.conv2 = nn.Conv2d(16, 16, (5, 5), (2, 2))
        self.conv3 = nn.Conv3d(16, 64, (3, 3), (1, 1))
        self.fc1 = nn.Linear(64, 16)
        self.fc2 = nn.Linear(16, 4)
        self.relu = nn.ReLU()
    
    def forward(self, x):
        out = self.conv1(x)
        out = self.relu(out)
        out = self.conv2(out)
        out = self.relu(out)
        out = self.conv3(out)
        out = self.relu(out)
        out = torch.mean(out, dim=3)
        out = torch.mean(out, dim=2)
        out = self.fc1(out)
        out = self.relu(out)
        return self.fc2(out)
    
    def get_loss(self, logit, label):
        return nn.CrossEntropyLoss()(logit, label)