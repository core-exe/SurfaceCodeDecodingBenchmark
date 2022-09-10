#pragma once

#include "Graph.h"
#include "error_dynamics.hpp"
#include <vector>
#include <memory>
#include <functional>

namespace Decoder::Matching {

enum class EdgeType {
    IN = 0,
    TIME = 1,
    SPACENN = 4,
    SPACENP = 5,
    SPACEPN = 6,
    SPACEPP = 7
};

inline bool get_chain_direction(EdgeType type, int vertex) {
    // type: EdgeType, only valid for SPACEXX
    // vertex: 0 for first, 1 for second
    return (int)type & (1 << (1 - vertex));
}

class PlanarIndex3d{
    public:
    ErrorDynamics::CodeScheme::PlanarIndex index;
    bool virt;
    int _t;
    inline PlanarIndex3d(): index(0, 0), _t(0), virt(true) {}
    inline PlanarIndex3d(int i, int j, int __t): index(i, j), _t(__t), virt(false) {}
    inline const int& i() { return index.i(); }
    inline const int& j() { return index.j(); }
    inline const int& t() { return _t; }
};

using distance_function = std::function<std::pair<EdgeType, double>(PlanarIndex3d, PlanarIndex3d)>;

struct SyndromeGraph {
    MWPM::Graph graph;
    std::vector<PlanarIndex3d> index_lookup;
    std::vector<EdgeType> edge_type;
    std::vector<double> weight;
    inline SyndromeGraph(): graph(), index_lookup(), edge_type(), weight() {}
};

std::shared_ptr<SyndromeGraph> get_graph(
    ErrorDynamics::PlanarData data,
    ErrorDynamics::CodeScheme::PlanarShape shape,
    bool measurement_error,
    const distance_function& distance_func
);

std::shared_ptr<ErrorDynamics::CodeScheme::PlanarError> matching_to_correction(
    std::shared_ptr<SyndromeGraph> syndrome_graph,
    ErrorDynamics::CodeScheme::PlanarShape shape,
    const std::list<int>& matching
);

void show_syndrome_graph(std::shared_ptr<SyndromeGraph> syndrome_graph);

}