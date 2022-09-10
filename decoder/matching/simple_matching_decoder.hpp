#pragma once
#include "decoder_base.hpp"
#include "matching_util.hpp"
#include "error_dynamics.hpp"
#include <utility>

namespace Decoder::Matching {

class SimpleMatchingDecoder: public DecoderBase {
    protected:
    double log_px, log_py, log_pz, log_pm;
    int t_total;
    ErrorDynamics::CodeScheme::PlanarShape shape;
    bool measurement_error;
    
    public:
    SimpleMatchingDecoder() = delete;
    SimpleMatchingDecoder(double px, double py, double pz, double pm, bool _measurement_error, int _t_total, ErrorDynamics::CodeScheme::PlanarShape _shape);
    SimpleMatchingDecoder(double p, bool _measurement_error, int _t_total, ErrorDynamics::CodeScheme::PlanarShape _shape);

    virtual std::pair<EdgeType, double> distance_function(PlanarIndex3d idx_a, PlanarIndex3d idx_b) = 0;

    std::shared_ptr<ErrorDynamics::CodeScheme::PlanarError> operator() (ErrorDynamics::PlanarData data);
};

class StandardMWPMDecoder: public SimpleMatchingDecoder {
    public:
    StandardMWPMDecoder() = delete;
    StandardMWPMDecoder(double px, double py, double pz, double pm, bool measurement_error, int _t_total,  ErrorDynamics::CodeScheme::PlanarShape _shape);
    StandardMWPMDecoder(double p, bool measurement_error, int _t_total, ErrorDynamics::CodeScheme::PlanarShape _shape);

    std::pair<EdgeType, double> distance_function(PlanarIndex3d idx_a, PlanarIndex3d idx_b);
};

}