#include "matching_util.hpp"

using namespace std;

namespace Decoder::Matching {

shared_ptr<SyndromeGraph> get_graph(
    ErrorDynamics::RectData data,
    ErrorDynamics::CodeScheme::RectShape shape,
    bool measurement_error,
    const distance_function& distance_func,
    const edge_distance_function& edge_distance_func_space,
    const edge_distance_function& edge_distance_func_time
) {
    auto syndrome_graph = make_shared<SyndromeGraph>();
    auto syndromes = data.first;
    int total_time = syndromes->size();
    int t = 0;
    int vertex_count = 0;
    auto& weight = syndrome_graph->weight;
    for(auto p = syndromes->cbegin(); p != syndromes->cend(); t++, p++) {
        for(int i = 0; i < shape.x(); i++) {
            for(int j = (i + 1) % 2; j < shape.y(); j += 2) {
                if((*p)->get_symptom(ErrorDynamics::CodeScheme::RectIndex(i, j)) == ErrorDynamics::Util::Symptom::NEGATIVE) {
                    // the inside vertex
                    RectIndex3d inside_idx = RectIndex3d(i, j, t);
                    int vertex_inside = vertex_count++;
                    syndrome_graph->graph.AddVertex();
                    syndrome_graph->index_lookup.push_back(inside_idx);

                    // the space edge vertex
                    int vertex_edge_space = vertex_count++;
                    syndrome_graph->graph.AddVertex();
                    auto edge_weight = edge_distance_func_space(inside_idx);
                    if((i % 2) == 1) // connect towards i = 0 or i = x ( measure-X qubit)
                        syndrome_graph->index_lookup.push_back(RectIndex3d(NodeType::ESX, (Direction)edge_weight.first));
                    else // connect towards j = 0 or i = y (measure-Z qubit)
                        syndrome_graph->index_lookup.push_back(RectIndex3d(NodeType::ESZ, (Direction)edge_weight.first));
                    
                    syndrome_graph->graph.AddEdge(vertex_inside, vertex_edge_space);
                    weight.push_back(edge_weight.second);
                    
                    // the time edge vertex
                    if(measurement_error) {
                        int vertex_edge_time = vertex_count++;
                        syndrome_graph->graph.AddVertex();
                        auto edge_weight = edge_distance_func_time(inside_idx);

                        if((i % 2) == 1) // connect towards t = 0 or t = T ( measure-X qubit)
                            syndrome_graph->index_lookup.push_back(RectIndex3d(NodeType::ETX, (Direction)edge_weight.first));
                        else // connect towards t = 0 or t = T (measure-Z qubit)
                            syndrome_graph->index_lookup.push_back(RectIndex3d(NodeType::ETZ, (Direction)edge_weight.first));

                        syndrome_graph->graph.AddEdge(vertex_inside, vertex_edge_time);
                        weight.push_back(edge_weight.second);
                    }
                    
                    for(int k = 0; k < vertex_inside; k++) {
                        if(syndrome_graph->index_lookup[k].is_in()) {
                            auto edge_weight = distance_func(inside_idx, syndrome_graph->index_lookup[k]);
                            if(edge_weight.first) {
                                syndrome_graph->graph.AddEdge(vertex_inside, k);
                                weight.push_back(edge_weight.second);
                            }
                        } else if(syndrome_graph->index_lookup[k].node_type == syndrome_graph->index_lookup[vertex_inside + 1].node_type) {
                            syndrome_graph->graph.AddEdge(vertex_inside + 1, k);
                            weight.push_back(0);
                        } else if(measurement_error) {
                            if(syndrome_graph->index_lookup[k].node_type == syndrome_graph->index_lookup[vertex_inside + 2].node_type) {
                                syndrome_graph->graph.AddEdge(vertex_inside + 2, k);
                                weight.push_back(0);
                            }
                        }
                    }

                }
            }
        }
    }
    return syndrome_graph;
}



}