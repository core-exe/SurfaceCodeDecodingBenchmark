#include "decoder_base.hpp"
#include <vector>
#include <utility>
#include <string>
#include <pybind11/embed.h>
#include <pybind11/numpy.h>

namespace Decoder::ML {

struct PyBuffer{
    std::shared_ptr<void> ptr;
    ssize_t item_size;
    std::string format;
    int n_dim;
    std::vector<ssize_t> shape;
    std::vector<ssize_t> stride;
    inline PyBuffer(std::shared_ptr<void> _ptr, ssize_t _item_size, std::string _format, int _n_dim, std::vector<ssize_t> _shape, std::vector<ssize_t> _stride) :
        ptr(_ptr),
        item_size(_item_size),
        format(_format),
        n_dim(_n_dim),
        shape(_shape),
        stride(_stride)
        {}
};

class MLDecoder: public BatchDecoder {
    pybind11::module module;
    public:
    MLDecoder(std::string submodule_name);
    
    static std::pair<pybind11::array_t<int>, pybind11::array_t<int>> to_pyarray(std::vector<ErrorDynamics::RectData> datas);

    void add_train_data(std::pair<pybind11::array_t<int>, pybind11::array_t<int>> train_data);
    void add_valid_data(std::pair<pybind11::array_t<int>, pybind11::array_t<int>> valid_data);
    void add_test_data(std::pair<pybind11::array_t<int>, pybind11::array_t<int>> test_data);

    void init();
    void train();
    void load_model(std::string path);

    std::vector<std::shared_ptr<ErrorDynamics::CodeScheme::RectError>> operator()(std::vector<ErrorDynamics::RectData> datas);
};

}