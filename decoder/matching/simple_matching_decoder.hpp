#pragma once
#include "decoder_base.hpp"
#include "matching_util.hpp"
#include "error_dynamics.hpp"
#include <utility>

namespace Decoder::Matching {

class SimpleMatchingDecoder: public DecoderBase {
    protected:
    double log_px, log_py, log_pz, log_pm;
    ErrorDynamics::CodeScheme::RectShape shape;
    bool measurement_error;
    
    public:
    SimpleMatchingDecoder() = delete;
    SimpleMatchingDecoder(double px, double py, double pz, double pm, bool _measurement_error, ErrorDynamics::CodeScheme::RectShape _shape);
    SimpleMatchingDecoder(double p, bool _measurement_error, ErrorDynamics::CodeScheme::RectShape _shape);

    virtual std::pair<bool, double> distance_function(RectIndex3d idx_a, RectIndex3d idx_b) = 0;
    virtual std::pair<bool, double> edge_distance_function_space(RectIndex3d idx) = 0;
    virtual std::pair<bool, double> edge_distance_function_time(RectIndex3d idx, int t_total) = 0;
    std::shared_ptr<ErrorDynamics::CodeScheme::RectError> operator() (ErrorDynamics::RectData data);
};

class StandardMWPMDecoder: public SimpleMatchingDecoder {
    public:
    StandardMWPMDecoder() = delete;
    StandardMWPMDecoder(double px, double py, double pz, double pm, bool measurement_error, ErrorDynamics::CodeScheme::RectShape _shape);
    StandardMWPMDecoder(double p, bool measurement_error, ErrorDynamics::CodeScheme::RectShape _shape);

    std::pair<bool, double> distance_function(RectIndex3d idx_a, RectIndex3d idx_b);
    std::pair<bool, double> edge_distance_function_space(RectIndex3d idx);
    std::pair<bool, double> edge_distance_function_time(RectIndex3d idx, int t_total);
};

}