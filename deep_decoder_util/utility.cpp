#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include <vector>
#include "utility.hpp"
#include "error_dynamics.hpp"
#include <memory>

namespace py = pybind11;
namespace Err = ErrorDynamics;

py::array_t<int> get_logical_error(
    py::array_t<int, py::array::forcecast | py::array::c_style> &physical_errors
) {
    // physical_errors: shape (B, X, Y)

    ssize_t batch_size = physical_errors.shape(0);
    ssize_t x = physical_errors.shape(1);
    ssize_t y = physical_errors.shape(2);
    auto phy_err_acc = physical_errors.unchecked<3>();
    
    auto ret = std::vector<int>(batch_size);
    for(int b = 0; b < batch_size; b++) {
        int z_cnt = 0;
        for(int j = 0; j < y; j += 2) {
            if(phy_err_acc(b, 0, j) == 2 || phy_err_acc(b, 0, j) == 3)
                z_cnt++;
        }
        int x_cnt = 0;
        for(int i = 0; i < x; i += 2) {
            if(phy_err_acc(b, i, 0) == 1 || phy_err_acc(b, i, 0) == 2)
                x_cnt++;
        }
        if(z_cnt == 1)
            ret[b] = pauli_mult[ret[b]][3];
        if(x_cnt == 1)
            ret[b] = pauli_mult[ret[b]][1];
    }
    return py::array_t<int>(py::buffer_info(
        ret.data(),
        sizeof(int),
        py::format_descriptor<int>::format(),
        1,
        std::vector<ssize_t>({batch_size}),
        std::vector<ssize_t>({sizeof(int)})
    ));
}

py::array_t<int> apply_logical_error(
    py::array_t<int, py::array::forcecast | py::array::c_style> &physical_errors,
    py::array_t<int, py::array::forcecast | py::array::c_style> &logical_errors
) {
    // physical_errors: shape (B, X, Y)
    // logical_errors: shape (B, )

    ssize_t batch_size = physical_errors.shape(0);
    ssize_t x = physical_errors.shape(1);
    ssize_t y = physical_errors.shape(2);
    auto phy_err_acc = physical_errors.unchecked<3>();
    auto log_err_acc = logical_errors.unchecked<1>();

    auto ret = std::vector<int>(batch_size * x * y);
    std::memcpy(ret.data(), physical_errors.data(), batch_size * x * y * sizeof(int));
    
    for(int b = 0; b < batch_size; b++) {
        if(log_err_acc(b) == 1 || log_err_acc(b) == 2) {
            for(int j = 0; j < y; j += 2) {
                ret[b * x * y + 0 * y + j] = pauli_mult[ret[b * x * y + 0 * y + j]][1];
            }
        }
        if(log_err_acc(b) == 3 || log_err_acc(b) == 2) {
            for(int i = 0; i < x; i += 2) {
                ret[b * x * y + i * y + 0] = pauli_mult[ret[b * x * y + i * y + 0]][3];
            }
        }
    }

    return py::array_t<int>(py::buffer_info(
        ret.data(),
        sizeof(int),
        py::format_descriptor<int>::format(),
        3, 
        std::vector<ssize_t>({batch_size, x, y}),
        std::vector<ssize_t>({(ssize_t)sizeof(int) * x * y, (ssize_t)sizeof(int) * y, (ssize_t)sizeof(int)})
    ));
}

py::array_t<int> is_valid(
    py::array_t<int, py::array::forcecast | py::array::c_style> &physical_errors
) {
    ssize_t batch_size = physical_errors.shape(0);
    ssize_t x = physical_errors.shape(1);
    ssize_t y = physical_errors.shape(2);
    auto ret = std::vector<int>(batch_size);

    for(int b = 0; b < batch_size; b++) {
        auto list = std::vector<int>(x * y);
        std::memcpy(list.data(), physical_errors.data() + b * x * y, x * y * sizeof(int));
        bool is_valid = std::make_shared<Err::CodeScheme::RectError>(x, y, list)->is_valid();
        ret[b] = is_valid;
    }

    return py::array_t<int>(py::buffer_info(
        ret.data(),
        sizeof(int),
        py::format_descriptor<int>::format(),
        1,
        std::vector<ssize_t>({batch_size}),
        std::vector<ssize_t>({sizeof(int)})
    ));
}

py::array_t<int> qubit_type(
    int x, int y
) {
    auto ret = std::vector<int>(x * y);
    for(int i = 0; i < x; i++)
        for(int j = (i + 1) % 2; j < y; j += 2)
            ret[i * y + j] = 1;
    return py::array_t<int>(py::buffer_info(
        ret.data(),
        sizeof(int),
        py::format_descriptor<int>::format(),
        2,
        {x, y},
        {y * sizeof(int), sizeof(int)}
    ));
}

py::array_t<int> apply_physical_correction(
    py::array_t<int, py::array::forcecast | py::array::c_style> &physical_errors,
    py::array_t<int, py::array::forcecast | py::array::c_style> &corrections
) {
    py::array_t<int> ret = py::array_t<int>(physical_errors);
    ssize_t batch_size = physical_errors.shape(0);
    ssize_t x = physical_errors.shape(1);
    ssize_t y = physical_errors.shape(2);
    
    auto phy_acc = physical_errors.unchecked<3>();
    auto cor_acc = corrections.unchecked<3>();
    auto ret_acc = ret.mutable_unchecked<3>();
    for(int b = 0; b < batch_size; b++) {
        for(int i = 0; i < x; i++) {
            for(int j = i % 2; j < y; j += 2) {
                ret_acc(b, i, j) = pauli_mult[phy_acc(b, i, j)][cor_acc(b, i, j)];
            }
        }
    }
    return ret;
}

PYBIND11_MODULE(deep_decoder_util, m) {
    m.def("get_logical_error", &get_logical_error);
    m.def("apply_logical_error", &apply_logical_error);
    m.def("is_valid", &is_valid);
    m.def("qubit_type", &qubit_type);
    m.def("apply_physical_correction", &apply_physical_correction);
}