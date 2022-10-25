#include "matching_util.hpp"
#include <iostream>

using namespace std;
namespace Err = ErrorDynamics;
namespace Cs = Err::CodeScheme;

namespace Decoder::Matching {

shared_ptr<SyndromeGraph> get_graph(
    ErrorDynamics::PlanarData data,
    ErrorDynamics::CodeScheme::PlanarShape shape,
    bool measurement_error,
    const distance_function& distance_func,
    int graph_type
) {
    auto syndrome_graph = make_shared<SyndromeGraph>();
    auto syndromes = data.first;
    int total_time = syndromes->size();
    int vertex_count = 0;
    auto& graph = syndrome_graph->graph;
    auto& idx_lookup = syndrome_graph->index_lookup;
    auto& edge_type = syndrome_graph->edge_type;
    auto& weight = syndrome_graph->weight;

    if(graph_type & graph_z) {
        for(int i = 0; i < shape.x(); i += 2) {
            for(int j = 1; j < shape.y(); j += 2) {
                int t = 0;
                for(auto p = syndromes->cbegin(); p != syndromes->cend(); t++, p++) {
                    if((*p)->get_symptom(Cs::PlanarIndex(i, j)) == Err::Util::Symptom::NEGATIVE) {
                        idx_lookup.push_back(PlanarIndex3d(i, j, t));
                        graph.AddVertex();
                        for(int k = 0; k < vertex_count; k++) {
                            auto edge_weight = distance_func(idx_lookup[k], PlanarIndex3d(i, j, t));
                            graph.AddEdge(k, vertex_count);
                            edge_type.push_back(edge_weight.first);
                            weight.push_back(edge_weight.second);
                        }
                        vertex_count++;
                    }
                }
            }
        }
        if(vertex_count % 2 == 1) {
            idx_lookup.push_back(PlanarIndex3d());
            graph.AddVertex();
            for(int k = 0; k < vertex_count; k++) {
                auto edge_weight = distance_func(idx_lookup[k], PlanarIndex3d());
                graph.AddEdge(k, vertex_count);
                edge_type.push_back(edge_weight.first);
                weight.push_back(edge_weight.second);
            }
            vertex_count++;
        }
    }
    int init_x_vertex = vertex_count;
    
    if(graph_type & graph_x) {
        for(int i = 1; i < shape.x(); i += 2) {
            for(int j = 0; j < shape.y(); j += 2) {
                int t = 0;
                for(auto p = syndromes->cbegin(); p != syndromes->cend(); t++, p++) {
                    if((*p)->get_symptom(Cs::PlanarIndex(i, j)) == Err::Util::Symptom::NEGATIVE) {
                        idx_lookup.push_back(PlanarIndex3d(i, j, t));
                        graph.AddVertex();
                        for(int k = init_x_vertex; k < vertex_count; k++) {
                            auto edge_weight = distance_func(idx_lookup[k], PlanarIndex3d(i, j, t));
                            graph.AddEdge(k, vertex_count);
                            edge_type.push_back(edge_weight.first);
                            weight.push_back(edge_weight.second);
                        }
                        vertex_count++;
                    }
                }
            }
        }
        if(vertex_count % 2 == 1) {
            idx_lookup.push_back(PlanarIndex3d());
            graph.AddVertex();
            for(int k = 0; k < vertex_count; k++) {
                auto edge_weight = distance_func(idx_lookup[k], PlanarIndex3d());
                graph.AddEdge(k, vertex_count);
                edge_type.push_back(edge_weight.first);
                weight.push_back(edge_weight.second);
            }
            vertex_count++;
        }
    }
    return syndrome_graph;
}

std::shared_ptr<ErrorDynamics::CodeScheme::PlanarError> matching_to_correction(
    std::shared_ptr<SyndromeGraph> syndrome_graph,
    ErrorDynamics::CodeScheme::PlanarShape shape,
    const std::list<int>& matching
) {
    auto error = std::make_shared<ErrorDynamics::CodeScheme::PlanarError>(shape.x(), shape.y());
    auto& graph = syndrome_graph->graph;
    auto& idx_lookup = syndrome_graph->index_lookup;
    auto& edge_type = syndrome_graph->edge_type;
    auto& weight = syndrome_graph->weight;
    
    for(auto it = matching.cbegin(); it != matching.cend(); it++) {
        auto edge = graph.GetEdge(*it);
        int u = edge.first, v = edge.second;
        auto idx_u = idx_lookup[u];
        auto idx_v = idx_lookup[v];
        auto e_type = edge_type[*it];
        if(idx_v.virt) {
            if(e_type == EdgeType::TIME)
                continue;
            bool axis = (idx_u.i() % 2 == 1); // true for index i, false for index j
            auto pauli = ((idx_u.i() % 2 == 0) ? Err::Util::Pauli::X : Err::Util::Pauli::Z);
            bool dir = get_chain_direction(e_type, 0);
            int delta = (dir ? 1 : -1);
            int bound = (dir ? (axis ? shape.x() : shape.y()) : -1);
            for(int current_idx = (axis ? idx_u.i() : idx_u.j()); current_idx != bound; current_idx += delta * 2)
                error->mult_error(axis ? Cs::PlanarIndex(current_idx + delta, idx_u.j()) : Cs::PlanarIndex(idx_u.i(), current_idx + delta), pauli);
        } else {
            if(e_type == EdgeType::TIME)
                continue;
            if(e_type == EdgeType::IN) {
                auto pauli = ((idx_u.i() % 2 == 0) ? Err::Util::Pauli::X : Err::Util::Pauli::Z);
                int dx = (idx_v.i() - idx_u.i() >= 0 ? 1 : -1);
                int dy = (idx_v.j() - idx_u.j() >= 0 ? 1 : -1);
                for(int current_i = idx_u.i(); current_i != idx_v.i(); current_i += 2 * dx)
                    error->mult_error(Cs::PlanarIndex(current_i + dx, idx_u.j()), pauli);
                for(int current_j = idx_u.j(); current_j != idx_v.j(); current_j += 2 * dy)
                    error->mult_error(Cs::PlanarIndex(idx_v.i(), current_j + dy), pauli);
            } else {
                bool axis = (idx_u.i() % 2 == 1); // true for index i, false for index j
                auto pauli = ((idx_u.i() % 2 == 0) ? Err::Util::Pauli::X : Err::Util::Pauli::Z);
                bool dir = get_chain_direction(e_type, 0);
                int delta = (dir ? 1 : -1);
                int bound = (dir ? (axis ? shape.x() : shape.y()) : -1);
                for(int current_idx = (axis ? idx_u.i() : idx_u.j()); current_idx != bound; current_idx += delta * 2)
                    error->mult_error(axis ? Cs::PlanarIndex(current_idx + delta, idx_u.j()) : Cs::PlanarIndex(idx_u.i(), current_idx + delta), pauli);
                dir = get_chain_direction(e_type, 1);
                delta = (dir ? 1 : -1);
                bound = (dir ? (axis ? shape.x() : shape.y()) : -1);
                for(int current_idx = (axis ? idx_v.i() : idx_v.j()); current_idx != bound; current_idx += delta * 2)
                    error->mult_error(axis ? Cs::PlanarIndex(current_idx + delta, idx_v.j()) : Cs::PlanarIndex(idx_v.i(), current_idx + delta), pauli);
            }
        }
    }
    return error;
}

void show_syndrome_graph(std::shared_ptr<SyndromeGraph> syndrome_graph) {
    auto& graph = syndrome_graph->graph;
    auto& idx_lookup = syndrome_graph->index_lookup;
    auto& edge_type = syndrome_graph->edge_type;
    auto& weight = syndrome_graph->weight;
    
    int v = graph.GetNumVertices();
    int e = graph.GetNumEdges();

    printf("Number of vertex: %d\n", v);
    for(int i = 0; i < v; i++) {
        if(idx_lookup[i].virt)
            printf("Vertex %4d | Virtual\n", i);
        else
            printf("Vertex %4d | i = %4d | j = %4d | t = %4d\n", i, idx_lookup[i].i(), idx_lookup[i].j(), idx_lookup[i].t());
    }
    printf("\n");

    printf("Number of edge: %d\n", e);
    for(int j = 0; j < e; j++) {
        printf("Edge %4d | %4d %4d | weight = %8.2f | type = %d\n", j, graph.GetEdge(j).first, graph.GetEdge(j).second, weight[j], (int)edge_type[j]);
    }
    printf("\n");
    printf("\n");
}

}