#pragma once

#include "Graph.h"
#include "error_dynamics.hpp"
#include <vector>
#include <memory>
#include <functional>

namespace Decoder::Matching {

enum class NodeType {
    IN, ESX, ESZ, ETX, ETZ
};

enum class Direction {
    NEG, POS
};

class PlanarIndex3d{
    public:
    ErrorDynamics::CodeScheme::PlanarIndex index;
    int _t;
    NodeType node_type;
    Direction direction;
    inline PlanarIndex3d(int i, int j, int __t): index(i, j), _t(__t) { node_type = NodeType::IN; }
    inline PlanarIndex3d(NodeType _type, Direction _direction): index(0, 0), _t(0) { node_type = _type, direction = _direction; }
    inline const int& i() { return index.i(); }
    inline const int& j() { return index.j(); }
    inline const int& t() { return _t; }
    inline bool is_in() { return node_type == NodeType::IN; }
};

using distance_function = std::function<std::pair<bool, double>(PlanarIndex3d, PlanarIndex3d)>;
using edge_distance_function = std::function<std::pair<int, double>(PlanarIndex3d)>;

struct SyndromeGraph {
    MWPM::Graph graph;
    std::vector<PlanarIndex3d> index_lookup;
    std::vector<double> weight;
    inline SyndromeGraph(): graph(), index_lookup(), weight() {}
};

std::shared_ptr<SyndromeGraph> get_graph(
    ErrorDynamics::PlanarData data,
    ErrorDynamics::CodeScheme::PlanarShape shape,
    bool measurement_error,
    const distance_function& distance_func,
    const edge_distance_function& edge_distance_func_space,
    const edge_distance_function& edge_distance_func_time
);

std::shared_ptr<ErrorDynamics::CodeScheme::PlanarError> matching_to_correction(
    std::shared_ptr<SyndromeGraph> syndrome_graph,
    ErrorDynamics::CodeScheme::PlanarShape shape,
    const std::list<int>& matching
);

}