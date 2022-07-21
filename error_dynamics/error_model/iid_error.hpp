#pragma once
#include "error_model_base.hpp"

namespace ErrorDynamics {
namespace ErrorModel {

class IIDError: public ErrorModelBase{
    private:
    double px, py, pz;
    double pm;

    public:
    IIDError() = delete;
    inline IIDError(double _px, double _py, double _pz, double _pm) {
        px = _px, py = _py, pz = _pz, pm = _pm;
    };
    inline IIDError(double p) : IIDError(p, p, p, p / 3.0f * 2.0f) {}

    std::pair<std::shared_ptr<CodeScheme::RectError>, std::shared_ptr<CodeScheme::RectSyndrome>> generate_rectangular_error(const CodeScheme::RectShape shape);

};

}}