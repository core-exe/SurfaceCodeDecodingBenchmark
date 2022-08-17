#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

namespace py = pybind11;

static int pauli_mult[4][4] = {
    0, 1, 2, 3,
    1, 0, 3, 2,
    2, 3, 0, 1,
    3, 2, 1, 0,
};

py::array_t<int> get_logical_error(
    py::array_t<int, py::array::forcecast | py::array::c_style> &physical_errors
);

py::array_t<int> apply_logical_error(
    py::array_t<int, py::array::forcecast | py::array::c_style> &physical_errors,
    py::array_t<int, py::array::forcecast | py::array::c_style> &logical_errors
);

py::array_t<int> is_valid(
    py::array_t<int, py::array::forcecast | py::array::c_style> &physical_errors
);

py::array_t<int> qubit_type(
    int x, int y
);