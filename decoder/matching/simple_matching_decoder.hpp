#pragma once
#include "decoder_base.hpp"
#include "matching_util.hpp"
#include "error_dynamics.hpp"
#include <utility>

namespace Decoder::Matching {

class SimpleMatchingDecoder: public DecoderBase {
    protected:
    double log_px, log_py, log_pz, log_pm;
    ErrorDynamics::CodeScheme::PlanarShape shape;
    bool measurement_error;
    
    public:
    SimpleMatchingDecoder() = delete;
    SimpleMatchingDecoder(double px, double py, double pz, double pm, bool _measurement_error, ErrorDynamics::CodeScheme::PlanarShape _shape);
    SimpleMatchingDecoder(double p, bool _measurement_error, ErrorDynamics::CodeScheme::PlanarShape _shape);

    virtual std::pair<bool, double> distance_function(PlanarIndex3d idx_a, PlanarIndex3d idx_b) = 0;
    virtual std::pair<bool, double> edge_distance_function_space(PlanarIndex3d idx) = 0;
    virtual std::pair<bool, double> edge_distance_function_time(PlanarIndex3d idx, int t_total) = 0;
    std::shared_ptr<ErrorDynamics::CodeScheme::PlanarError> operator() (ErrorDynamics::PlanarData data);
};

class StandardMWPMDecoder: public SimpleMatchingDecoder {
    public:
    StandardMWPMDecoder() = delete;
    StandardMWPMDecoder(double px, double py, double pz, double pm, bool measurement_error, ErrorDynamics::CodeScheme::PlanarShape _shape);
    StandardMWPMDecoder(double p, bool measurement_error, ErrorDynamics::CodeScheme::PlanarShape _shape);

    std::pair<bool, double> distance_function(PlanarIndex3d idx_a, PlanarIndex3d idx_b);
    std::pair<bool, double> edge_distance_function_space(PlanarIndex3d idx);
    std::pair<bool, double> edge_distance_function_time(PlanarIndex3d idx, int t_total);
};

}