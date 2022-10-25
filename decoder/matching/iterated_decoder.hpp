#pragma once
#include "decoder_base.hpp"
#include "matching_util.hpp"
#include "error_dynamics.hpp"
#include <utility>
#include <vector>
#include <list>

namespace Decoder::Matching {

class IteratedDecoder: public DecoderBase {
    private:
    double px, py, pz, pm;
    bool measurement_error;
    int n_iter;
    int starting_graph;

    public:
    IteratedDecoder() = delete;
    IteratedDecoder(double _px, double _py, double _pz, double _pm, bool _measurement_error, int _n_iter, int _starting_graph = graph_x);
    IteratedDecoder(double p, bool _measurement_error, int _n_iter, int _starting_graph = graph_x);

    std::shared_ptr<ErrorDynamics::CodeScheme::PlanarError> operator() (ErrorDynamics::PlanarData data);
};

}