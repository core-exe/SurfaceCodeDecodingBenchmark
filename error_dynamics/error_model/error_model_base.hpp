#pragma once
#include "util.hpp"
#include "code_scheme.hpp"

#include <memory>
#include <utility>

namespace ErrorDynamics {
namespace ErrorModel {

class ErrorModelBase {
    public:
    ErrorModelBase(){}
    
    virtual std::pair<std::shared_ptr<CodeScheme::PlanarError>, std::shared_ptr<CodeScheme::PlanarSyndrome>> generate_planar_error(const CodeScheme::PlanarShape shape) = 0;
};

}};