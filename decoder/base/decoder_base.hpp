#pragma once

#include "error_dynamics.hpp"
#include <memory>

namespace Decoder {

class DecoderBase {    
    public:
    DecoderBase(){}

    virtual std::shared_ptr<ErrorDynamics::CodeScheme::RectError> operator() (ErrorDynamics::RectData data) = 0;
    virtual std::vector<std::shared_ptr<ErrorDynamics::CodeScheme::RectError>> operator()(std::vector<ErrorDynamics::RectData> datas);
};

class BatchDecoder: public DecoderBase {
    public:
    BatchDecoder(){}

    std::vector<ErrorDynamics::RectData> generate_batch(std::shared_ptr<ErrorDynamics::RectangularSurfaceCode> code, int batch_size, int step);

    inline virtual std::shared_ptr<ErrorDynamics::CodeScheme::RectError> operator() (ErrorDynamics::RectData data) {
        return this->operator()(std::vector<ErrorDynamics::RectData>({ data }))[0];
    }
    virtual std::vector<std::shared_ptr<ErrorDynamics::CodeScheme::RectError>> operator()(std::vector<ErrorDynamics::RectData> datas) = 0;
};


}