#include <pybind11/embed.h>
#include "ml_decoder.hpp"
#include <vector>

namespace py = pybind11;


namespace Decoder::ML {

MLDecoder::MLDecoder(std::string submodule_name) {
    py::module sys = py::module::import("sys");
    sys.attr("path").attr("append")(DEEP_DECODER_PY_PKG_PATH);
    module = py::module::import(("deep_decoder." + submodule_name).c_str());
}

std::pair<PyBuffer, PyBuffer> MLDecoder::to_pybuffer(std::vector<ErrorDynamics::RectData> datas){
    int batch_size = datas.size();
    int length = datas[0].first->size();
    auto shape = datas[0].second->get_shape();

    std::vector<ssize_t> shape_data = {
        (ssize_t)batch_size,
        (ssize_t)length,
        (ssize_t)shape.x(),
        (ssize_t)shape.y()
    };

    std::vector<ssize_t> shape_target = {
        (ssize_t)batch_size,
        (ssize_t)shape.x(),
        (ssize_t)shape.y()
    };

    std::vector<ssize_t> stride_data = {
        (ssize_t)(length * shape.x() * shape.y() * sizeof(int)),
        (ssize_t)(shape.x() * shape.y() * sizeof(int)),
        (ssize_t)(shape.y() * sizeof(int)),
        sizeof(int)
    };

    std::vector<ssize_t> stride_target = {
        (ssize_t)(shape.x() * shape.y() * sizeof(int)),
        (ssize_t)(shape.y() * sizeof(int)),
        sizeof(int)
    };

    std::shared_ptr<void> data = std::make_shared<int[]>(new int[batch_size * length * shape.x() * shape.y()]);
    std::shared_ptr<void> target = std::make_shared<int[]>(new int[batch_size * shape.x() * shape.y()]);

    int batch_counter = 0;
    for(auto it_batch = datas.begin(); it_batch != datas.end(); batch_counter++, it_batch++) {
        int time_counter = 0;
        for(auto it_time = (*it_batch).first->begin(); it_time != (*it_batch).first->end(); time_counter++, it_time++)
            std::memcpy(data.get() + (stride_data[0] / sizeof(int)) * batch_counter + (stride_data[1] / sizeof(int)) * time_counter, (*it_time)->to_vector().data(), shape.x() * shape.y() * sizeof(int));
        std::memcpy(target.get() + (stride_target[0] / sizeof(int)) * batch_counter, (*it_batch).second->to_vector().data(), shape.x() * shape.y() * sizeof(int));
    }

    return std::make_pair(
        PyBuffer(
            data,
            sizeof(int),
            py::format_descriptor<int>::format(),
            4,
            shape_data,
            stride_data
        ),
        PyBuffer(
            target,
            sizeof(int),
            py::format_descriptor<int>::format(),
            3,
            shape_target,
            stride_target
        )
    );
}

}