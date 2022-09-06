#pragma once

#include "error_dynamics.hpp"
#include <memory>

namespace Decoder {

class DecoderBase {    
    public:
    DecoderBase(){}

    virtual std::shared_ptr<ErrorDynamics::CodeScheme::PlanarError> operator() (ErrorDynamics::PlanarData data) = 0;
    virtual std::vector<std::shared_ptr<ErrorDynamics::CodeScheme::PlanarError>> operator()(std::vector<ErrorDynamics::PlanarData> datas);
};

class BatchDecoder: public DecoderBase {
    public:
    BatchDecoder(){}

    std::vector<ErrorDynamics::PlanarData> generate_batch(std::shared_ptr<ErrorDynamics::PlanarSurfaceCode> code, int batch_size, int step);

    inline virtual std::shared_ptr<ErrorDynamics::CodeScheme::PlanarError> operator() (ErrorDynamics::PlanarData data) {
        return this->operator()(std::vector<ErrorDynamics::PlanarData>({ data }))[0];
    }
    virtual std::vector<std::shared_ptr<ErrorDynamics::CodeScheme::PlanarError>> operator()(std::vector<ErrorDynamics::PlanarData> datas) = 0;
};


}