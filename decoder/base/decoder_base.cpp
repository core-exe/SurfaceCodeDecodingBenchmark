#include "decoder_base.hpp"
#include "error_dynamics.hpp"
#include <vector>
#include <memory>

namespace Decoder {

std::vector<std::shared_ptr<ErrorDynamics::CodeScheme::PlanarError>> DecoderBase::operator()(std::vector<ErrorDynamics::PlanarData> datas) {
    auto ret = std::vector<std::shared_ptr<ErrorDynamics::CodeScheme::PlanarError>>();
    for(auto it = datas.begin(); it != datas.end(); it++) {
        ret.push_back(this->operator()(*it));
    }
    return ret;
}

std::vector<ErrorDynamics::PlanarData> BatchDecoder::generate_batch(std::shared_ptr<ErrorDynamics::PlanarSurfaceCode> code, int batch_size, int step) {
    auto batch_data = std::vector<ErrorDynamics::PlanarData>(0);
    for(int _ = 0; _ < batch_size; _++) {
        code->step(step);
        auto data = code->get_data();
        batch_data.push_back(data);
        code->reset();
    }
    return batch_data;
}

}