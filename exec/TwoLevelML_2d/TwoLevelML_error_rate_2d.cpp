#include "error_dynamics.hpp"
#include "decoder.hpp"
#include <memory>
#include <pybind11/embed.h>
#include <string>
#include <filesystem>

#define TEST_SAMPLE_NUM 1000000
#define TRAIN_SAMPLE_NUM 500000
#define VALID_SAMPLE_NUM 200000
#define INSERT_BATCH_SIZE 1000

namespace py = pybind11;
namespace Dc = Decoder;
namespace Err = ErrorDynamics;
namespace Cs = Err::CodeScheme;

std::string get_name(int d, float p) {
    return std::string("test_name");
}

int main() {
    py::scoped_interpreter guard{};

    int d = 27;
    float p = 0.01;

    auto data_path = std::filesystem::path(PROJECT_ROOT_PATH) / "exec" / "TwoLevelML_2d" / "out" / "models";
    auto ml_decoder = Dc::ML::MLDecoder("two_level_decoder");

    auto code = std::make_shared<Err::RectangularSurfaceCode>(
        d,
        std::make_shared<Err::ErrorModel::IIDError>(p / 3, p / 3, p / 3, 0)
    );

    for(int _ = 0; _ < TRAIN_SAMPLE_NUM / INSERT_BATCH_SIZE; _++) {
        ml_decoder.add_train_data(ml_decoder.to_pyarray(ml_decoder.generate_batch(code, INSERT_BATCH_SIZE, 1)));
    }
    for(int _ = 0; _ < VALID_SAMPLE_NUM / INSERT_BATCH_SIZE; _++) {
        ml_decoder.add_valid_data(ml_decoder.to_pyarray(ml_decoder.generate_batch(code, INSERT_BATCH_SIZE, 1)));
    }
    ml_decoder.init();
    ml_decoder.set_path(data_path);
    ml_decoder.set_name(get_name(d, p));
    ml_decoder.train();
    return 0;
}