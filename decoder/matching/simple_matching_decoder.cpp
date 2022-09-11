#include "simple_matching_decoder.hpp"
#include "Matching.h"
#include <cmath>
using namespace std;

namespace Err = ErrorDynamics;
namespace Cs = Err::CodeScheme;

namespace Decoder::Matching {

SimpleMatchingDecoder::SimpleMatchingDecoder(
    double px,
    double py,
    double pz,
    double pm,
    bool _measurement_error,
    int _t_total,
    ErrorDynamics::CodeScheme::PlanarShape _shape) : 
    log_px(log(px)), log_py(log(py)), log_pz(log(pz)), log_pm(log(pm)), measurement_error(_measurement_error), t_total(_t_total), shape(_shape) {}

SimpleMatchingDecoder::SimpleMatchingDecoder(
    double p,
    bool measurement_error,
    int _t_total,
    ErrorDynamics::CodeScheme::PlanarShape _shape) :
    SimpleMatchingDecoder::SimpleMatchingDecoder(p, p, p, (measurement_error ? p * 2 / 3 : 1), measurement_error, _t_total, _shape) {}

std::shared_ptr<ErrorDynamics::CodeScheme::PlanarError> SimpleMatchingDecoder::operator() (ErrorDynamics::PlanarData data) {
    auto syndrome_graph = get_graph(
        data,
        shape,
        measurement_error,
        [this](PlanarIndex3d idx_a, PlanarIndex3d idx_b) {
            return this->distance_function(idx_a, idx_b);
        }
    );
    //show_syndrome_graph(syndrome_graph);
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
    int _t_total,
    ErrorDynamics::CodeScheme::PlanarShape _shape) : StandardMWPMDecoder::SimpleMatchingDecoder(px, py, pz, pm, measurement_error, _t_total, _shape) {}

StandardMWPMDecoder::StandardMWPMDecoder(
    double p,
    bool measurement_error,
    int _t_total,
    ErrorDynamics::CodeScheme::PlanarShape _shape) : StandardMWPMDecoder::SimpleMatchingDecoder(p, measurement_error, _t_total, _shape) {}

std::pair<EdgeType, double> StandardMWPMDecoder::distance_function(PlanarIndex3d idx_a, PlanarIndex3d idx_b) {
    auto pauli = ((idx_a.i() % 2 == 0) ? Err::Util::Pauli::X : Err::Util::Pauli::Z); // type of pauli operator on the chain
    // only idx_b can be virtual pratically
    if(idx_b.virt) {
        double time_weight = (measurement_error ? (min(idx_a.t(), t_total - idx_a.t()) * (-log_pm)) : INFINITY);
        int length_space_chain = (
            pauli == Err::Util::Pauli::X ?
            min(shape.y() - idx_a.j(), idx_a.j() + 1) :
            min(shape.x() - idx_a.i(), idx_a.i() + 1)
        ) / 2;
        double space_weight = -(pauli == Err::Util::Pauli::X ? log_px : log_pz) * length_space_chain;
        double weight = min(time_weight, space_weight);
        auto edge_type = (time_weight < space_weight ?
            EdgeType::TIME :
            (
                pauli == Err::Util::Pauli::X ?
                shape.y() - idx_a.j() > idx_a.j() + 1 :
                shape.x() - idx_a.i() > idx_a.i() + 1
            ) ? EdgeType::SPACENP : EdgeType::SPACEPN
        );
        return make_pair(edge_type, weight);
    }
    double time_weight = (measurement_error ? ((t_total + 1 - abs(idx_a.t() - idx_b.t())) * (-log_pm)) : INFINITY);

    int space_direction = 4;
    int length_space_chain_u = (
        pauli == Err::Util::Pauli::X ?
        min(shape.y() - idx_a.j(), idx_a.j() + 1) :
        min(shape.x() - idx_a.i(), idx_a.i() + 1)
    ) / 2;
    int length_space_chain_v = (
        pauli == Err::Util::Pauli::X ?
        min(shape.y() - idx_b.j(), idx_b.j() + 1) :
        min(shape.x() - idx_b.i(), idx_b.i() + 1)
    ) / 2;
    space_direction += ((
        pauli == Err::Util::Pauli::X ?
        shape.y() - idx_a.j() > idx_a.j() + 1 :
        shape.x() - idx_a.i() > idx_a.i() + 1
    ) ? 0 : 2);
    space_direction += ((
        pauli == Err::Util::Pauli::X ?
        shape.y() - idx_b.j() > idx_b.j() + 1 :
        shape.x() - idx_b.i() > idx_b.i() + 1
    ) ? 0 : 1);
    double space_weight = -(pauli == Err::Util::Pauli::X ? log_px : log_pz) * (length_space_chain_u + length_space_chain_v);

    int dx = abs(idx_a.i() - idx_b.i()) / 2;
    int dy = abs(idx_a.j() - idx_b.j()) / 2;
    int dt = abs(idx_a.t() - idx_b.t());
    double in_weight_space = (dx + dy) * -(pauli == Err::Util::Pauli::X ? log_px : log_pz);
    double in_weight_time = (idx_a.t() == idx_b.t() ? 0 : (measurement_error ? abs(idx_a.t() - idx_b.t()) * (-log_pm) : INFINITY));
    double in_weight_degenerate = lgamma(dx + dy + dt + 1) - lgamma(dx + 1) - lgamma(dy + 1) - lgamma(dt + 1);
    double in_weight = in_weight_space + in_weight_time + in_weight_degenerate;

    auto weight_vec = vector<double>({in_weight, time_weight, space_weight});
    int argmin = distance(weight_vec.begin(), min_element(weight_vec.begin(), weight_vec.end()));
    auto edge_type = (EdgeType)(argmin != 2 ? argmin : space_direction);
    return make_pair(edge_type, weight_vec[argmin]);
}

}