#include <pybind11/embed.h>
#include <pybind11/numpy.h>
#include "ml_decoder.hpp"
#include <vector>
#include <string>

namespace py = pybind11;

namespace Decoder::ML {

MLDecoder::MLDecoder(std::string submodule_name) {
    py::module sys = py::module::import("sys");
    sys.attr("path").attr("append")(DEEP_DECODER_PY_PKG_PATH);
    sys.attr("path").attr("append")(DEEP_DECODER_PY_PKG_PATH + std::string("/deep_decoder/"));
    module = py::module::import(("deep_decoder." + submodule_name).c_str());
}

std::pair<py::array_t<int>, py::array_t<int>> MLDecoder::to_pyarray(std::vector<ErrorDynamics::RectData> datas){
    int batch_size = datas.size();
    int length = datas[0].first->size();
    auto shape = datas[0].second->get_shape();

    auto qubit_type = std::vector<int>(shape.x() * shape.y(), 0);
    for(int i = 0; i < shape.x(); i++) {
        for(int j = (i + 1) % 2; j < shape.y(); j += 2) {
            qubit_type[i * shape.y() + j] = 1;
        }
    }

    std::vector<ssize_t> shape_data = {
        (ssize_t)batch_size,
        (ssize_t)(1 + length),
        (ssize_t)shape.x(),
        (ssize_t)shape.y()
    };

    std::vector<ssize_t> shape_target = {
        (ssize_t)batch_size,
        (ssize_t)shape.x(),
        (ssize_t)shape.y()
    };

    std::vector<ssize_t> stride_data = {
        (ssize_t)((1 + length) * shape.x() * shape.y() * sizeof(int)),
        (ssize_t)(shape.x() * shape.y() * sizeof(int)),
        (ssize_t)(shape.y() * sizeof(int)),
        sizeof(int)
    };

    std::vector<ssize_t> stride_target = {
        (ssize_t)(shape.x() * shape.y() * sizeof(int)),
        (ssize_t)(shape.y() * sizeof(int)),
        sizeof(int)
    };

    std::shared_ptr<int[]> data(new int[batch_size * (1 + length) * shape.x() * shape.y()]);
    std::shared_ptr<int[]> target(new int[batch_size * shape.x() * shape.y()]);

    int batch_counter = 0;
    for(auto it_batch = datas.begin(); it_batch != datas.end(); batch_counter++, it_batch++) {
        std::memcpy(
            data.get() + (stride_data[0] / sizeof(int)) * batch_counter,
            qubit_type.data(),
            shape.x() * shape.y() * sizeof(int)
        );
        int time_counter = 0;
        for(auto it_time = (*it_batch).first->begin(); it_time != (*it_batch).first->end(); time_counter++, it_time++)
            std::memcpy(
                data.get() + 
                (stride_data[0] / sizeof(int)) * batch_counter + 
                (stride_data[1] / sizeof(int)) * (time_counter + 1),
                (*it_time)->to_vector().data(),
                shape.x() * shape.y() * sizeof(int)
            );
        std::memcpy(target.get() + (stride_target[0] / sizeof(int)) * batch_counter, (*it_batch).second->to_vector().data(), shape.x() * shape.y() * sizeof(int));
    }

    return std::make_pair(
        py::array_t<int>(py::buffer_info(
            data.get(),
            sizeof(int),
            py::format_descriptor<int>::format(),
            4,
            shape_data,
            stride_data
        )),
        py::array_t<int>(py::buffer_info(
            target.get(),
            sizeof(int),
            py::format_descriptor<int>::format(),
            3,
            shape_target,
            stride_target
        ))
    );
}

void MLDecoder::add_train_data(std::pair<py::array_t<int>, py::array_t<int>> train_data) {
    module.attr("receive_train_data")(train_data);
}

void MLDecoder::add_valid_data(std::pair<py::array_t<int>, py::array_t<int>> valid_data) {
    module.attr("receive_valid_data")(valid_data);
}

void MLDecoder::add_test_data(std::pair<py::array_t<int>, py::array_t<int>> test_data) {
    module.attr("receive_test_data")(test_data);
}

void MLDecoder::init() {
    module.attr("init_dataset")();
}

void MLDecoder::train() {
    module.attr("begin_training")();
}

void MLDecoder::set_path(std::string path) {
    module.attr("set_path")(path);
}

void MLDecoder::set_name(std::string name) {
    module.attr("set_name")(name);
}

std::vector<std::shared_ptr<ErrorDynamics::CodeScheme::RectError>> MLDecoder::operator()(std::vector<ErrorDynamics::RectData> datas) {
    auto query = MLDecoder::to_pyarray(datas);
    auto ret = py::array_t<int>(module.attr("query_data")(query.first));
    int batch_size = ret.shape(0);
    int x = ret.shape(1);
    int y = ret.shape(2);

    auto corrections = std::vector<std::shared_ptr<ErrorDynamics::CodeScheme::RectError>>(0);
    for(int b = 0; b < batch_size; b++) {
        auto list = std::vector<int>(x * y);
        std::memcpy(list.data(), ret.data() + b * x * y, x * y * sizeof(int));
        corrections.push_back(std::make_shared<ErrorDynamics::CodeScheme::RectError>(x, y, list));
    }

    return corrections;
}

}