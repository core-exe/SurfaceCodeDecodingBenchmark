#pragma once
#include "decoder_base.hpp"
#include <vector>
#include <utility>
#include <string>
#include <pybind11/embed.h>
#include <pybind11/numpy.h>

namespace Decoder::ML {

class MLDecoder: public BatchDecoder {
    pybind11::module module;
    public:
    MLDecoder(std::string submodule_name);
    
    static std::pair<pybind11::array_t<int>, pybind11::array_t<int>> to_pyarray(std::vector<ErrorDynamics::RectData> datas);

    virtual void add_train_data(std::pair<pybind11::array_t<int>, pybind11::array_t<int>> train_data);
    virtual void add_valid_data(std::pair<pybind11::array_t<int>, pybind11::array_t<int>> valid_data);
    virtual void add_test_data(std::pair<pybind11::array_t<int>, pybind11::array_t<int>> test_data);

    virtual void init();
    virtual void train();
    
    virtual void set_path(std::string path);
    virtual void set_name(std::string name);

    virtual std::vector<std::shared_ptr<ErrorDynamics::CodeScheme::RectError>> operator()(std::vector<ErrorDynamics::RectData> datas);
};

}