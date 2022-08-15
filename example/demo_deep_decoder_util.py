#%%
import deep_decoder_util
import numpy as np
print(deep_decoder_util.__dict__)

error_i = np.zeros((7, 7), dtype=np.int32)
error_x = np.array([
    [0, 0, 0, 0, 0, 0, 0],
    [0, 0, 0, 0, 0, 0, 0],
    [1, 0, 1, 0, 1, 0, 1],
    [0, 0, 0, 0, 0, 0, 0],
    [0, 0, 0, 0, 0, 0, 0],
    [0, 0, 0, 0, 0, 0, 0],
    [0, 0, 0, 0, 0, 0, 0],
], dtype=np.int32)
error_y = np.array([
    [0, 0, 3, 0, 0, 0, 0],
    [0, 0, 0, 0, 0, 0, 0],
    [1, 0, 2, 0, 1, 0, 1],
    [0, 0, 0, 0, 0, 0, 0],
    [0, 0, 3, 0, 0, 0, 0],
    [0, 0, 0, 0, 0, 0, 0],
    [0, 0, 3, 0, 0, 0, 0],
], dtype=np.int32)
error_z = np.array([
    [0, 0, 3, 0, 0, 0, 0],
    [0, 0, 0, 0, 0, 0, 0],
    [0, 0, 3, 0, 0, 0, 0],
    [0, 0, 0, 0, 0, 0, 0],
    [0, 0, 3, 0, 0, 0, 0],
    [0, 0, 0, 0, 0, 0, 0],
    [0, 0, 3, 0, 0, 0, 0],
], dtype=np.int32)
error_invalid = np.array([
    [0, 0, 0, 0, 0, 0, 0],
    [0, 0, 0, 0, 0, 0, 0],
    [0, 0, 1, 0, 0, 0, 0],
    [0, 0, 0, 0, 0, 0, 0],
    [0, 0, 0, 0, 0, 0, 0],
    [0, 0, 0, 0, 0, 0, 0],
    [0, 0, 0, 0, 0, 0, 0],
], dtype=np.int32)

errors = np.stack([error_i, error_x, error_y, error_z, error_invalid], axis = 0)
# %%
deep_decoder_util.get_logical_error(errors)
# %%
deep_decoder_util.apply_logical_error(errors, np.array([0, 1, 2, 3, 0], dtype=np.int32))
# %%
deep_decoder_util.is_valid(errors)
# %%
