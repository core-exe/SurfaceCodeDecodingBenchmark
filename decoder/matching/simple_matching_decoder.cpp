#include "simple_matching_decoder.hpp"
#include "Matching.h"
#include <cmath>
using namespace std;

namespace Decoder::Matching {

SimpleMatchingDecoder::SimpleMatchingDecoder(
    double px,
    double py,
    double pz,
    double pm,
    bool _measurement_error,
    ErrorDynamics::CodeScheme::PlanarShape _shape) : 
    log_px(log(px)), log_py(log(py)), log_pz(log(pz)), log_pm(log(pm)), measurement_error(_measurement_error), shape(_shape) {}

SimpleMatchingDecoder::SimpleMatchingDecoder(
    double p,
    bool measurement_error,
    ErrorDynamics::CodeScheme::PlanarShape _shape) :
    SimpleMatchingDecoder::SimpleMatchingDecoder(p, p, p, (measurement_error ? p * 2 / 3 : 1), measurement_error, _shape) {}

std::shared_ptr<ErrorDynamics::CodeScheme::PlanarError> SimpleMatchingDecoder::operator() (ErrorDynamics::PlanarData data) {
    auto syndrome_graph = get_graph(
        data,
        shape,
        measurement_error,
        [this](PlanarIndex3d idx_a, PlanarIndex3d idx_b) {
            return this->distance_function(idx_a, idx_b);
        },
        [this](PlanarIndex3d idx) {
            return this->edge_distance_function_space(idx);
        },
        [this, data](PlanarIndex3d idx) {
            return this->edge_distance_function_time(idx, data.first->size());
        }
    );
    auto matching_algorithm = MWPM::Matching(syndrome_graph->graph);
    auto matching = matching_algorithm.SolveMinimumCostPerfectMatching(syndrome_graph->weight).first;
    return matching_to_correction(syndrome_graph, shape, matching);
}

StandardMWPMDecoder::StandardMWPMDecoder(
    double px,
    double py,
    double pz,
    double pm,
    bool measurement_error,
    ErrorDynamics::CodeScheme::PlanarShape _shape) : StandardMWPMDecoder::SimpleMatchingDecoder(px, py, pz, pm, measurement_error, _shape) {}

StandardMWPMDecoder::StandardMWPMDecoder(
    double p,
    bool measurement_error,
    ErrorDynamics::CodeScheme::PlanarShape _shape) : StandardMWPMDecoder::SimpleMatchingDecoder(p, measurement_error, _shape) {}

std::pair<bool, double> StandardMWPMDecoder::distance_function(PlanarIndex3d idx_a, PlanarIndex3d idx_b) {
    if((idx_a.i() - idx_b.i()) % 2 != 0) {
            return make_pair(false, (double)0.0);
        }
    return make_pair(true, -((abs(idx_a.i() - idx_b.i()) + abs(idx_a.j() - idx_b.j())) / 2) * (idx_a.i() % 2 == 0 ? log_px : log_pz));
}

std::pair<bool, double> StandardMWPMDecoder::edge_distance_function_space(PlanarIndex3d idx) {
    int pos = ((idx.i() % 2 == 1) ? idx.i() : idx.j());
    int length = ((idx.i() % 2 == 1) ? shape.x() : shape.y());
    int direction = ((2 * pos) <= length - 1 ? 0 : 1);
    int distance = (((direction == 0) ? pos : (length - 1) - pos) + 1) / 2;
    return make_pair(direction, -distance * (idx.i() % 2 == 0 ? log_px : log_pz));
}

std::pair<bool, double> StandardMWPMDecoder::edge_distance_function_time(PlanarIndex3d idx, int t_total) {
    if(!measurement_error)
        make_pair(0, (double)0.0);
    int direction = (idx.t() < t_total / 2 ? 0 : 1);
    int distance = (direction == 0 ? idx.t() + 1 : t_total - idx.t());
    return make_pair(direction, -distance * log_pm);
}

}