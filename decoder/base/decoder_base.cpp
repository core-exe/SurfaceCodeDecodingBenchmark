#include "decoder_base.hpp"
#include "error_dynamics.hpp"
#include <vector>
#include <memory>

namespace Decoder {

std::vector<std::shared_ptr<ErrorDynamics::CodeScheme::RectError>> DecoderBase::operator()(std::vector<ErrorDynamics::RectData> datas) {
    auto ret = std::vector<std::shared_ptr<ErrorDynamics::CodeScheme::RectError>>();
    for(auto it = datas.begin(); it != datas.end(); it++) {
        ret.push_back(this->operator()(*it));
    }
    return ret;
}

std::vector<ErrorDynamics::RectData> BatchDecoder::generate_batch(std::shared_ptr<ErrorDynamics::RectangularSurfaceCode> code, int batch_size, int step) {
    auto batch_data = std::vector<ErrorDynamics::RectData>(0);
    for(int _ = 0; _ < batch_size; _++) {
        code->step(step);
        auto data = code->get_data();
        batch_data.push_back(data);
    }
    return batch_data;
}

}