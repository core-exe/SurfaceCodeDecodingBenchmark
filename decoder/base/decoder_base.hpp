#pragma once

#include "error_dynamics.hpp"
#include <memory>

namespace Decoder {

class DecoderBase {
    public:
    DecoderBase(){}

    virtual std::shared_ptr<ErrorDynamics::CodeScheme::RectError> operator() (ErrorDynamics::RectData data) = 0;
};


}