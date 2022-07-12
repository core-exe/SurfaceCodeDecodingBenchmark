#pragma once
#include "util.hpp"
#include "code_scheme.hpp"

#include <memory>
#include <utility>

namespace ErrorDynamics {
namespace ErrorModel {

class ErrorModelBase{
    public:
    ErrorModelBase(){}
    
    virtual std::pair<std::shared_ptr<CodeScheme::RectError>, std::shared_ptr<CodeScheme::RectSyndrome>> generate_rectangular_error(const CodeScheme::RectShape shape) = 0;
};

}};