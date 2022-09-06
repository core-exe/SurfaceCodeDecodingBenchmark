#include "error_dynamics.hpp"
#include "decoder.hpp"
#include <memory>
#include <pybind11/embed.h>

namespace Dc = Decoder;
namespace Err = ErrorDynamics;
namespace Cs = Err::CodeScheme;

int main() {
    pybind11::scoped_interpreter guard{};

    auto code = std::make_shared<Err::PlanarSurfaceCode>(7, 0.01);
    
    auto decoder = Dc::ML::MLDecoder("test_io");
    
    decoder.add_train_data(decoder.to_pyarray(decoder.generate_batch(code, 16, 4)));
    decoder.add_valid_data(decoder.to_pyarray(decoder.generate_batch(code, 16, 4)));
    decoder.add_test_data(decoder.to_pyarray(decoder.generate_batch(code, 2, 1)));
    decoder.init();
    decoder.train();
    decoder(decoder.generate_batch(code, 16, 4));

    return 0;
}