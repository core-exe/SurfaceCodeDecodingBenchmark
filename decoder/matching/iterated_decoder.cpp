#include "iterated_decoder.hpp"
#include "Matching.h"
#include <cmath>
#include <algorithm>
#include <list>

namespace Decoder::Matching {

IteratedDecoder::IteratedDecoder(
    double _px,
    double _py,
    double _pz,
    double _pm,
    bool _measurement_error,
    int _n_iter,
    int _starting_graph 
) :
    px(_px),
    py(_py),
    pz(_pz),
    pm(_pm),
    measurement_error(_measurement_error),
    n_iter(_n_iter),
    starting_graph(_starting_graph) {}

IteratedDecoder::IteratedDecoder(
    double p,
    bool _measurement_error,
    int _n_iter,
    int _starting_graph) :
    IteratedDecoder(p, p, p, p * 2 / 3, _measurement_error, _n_iter, _starting_graph) {}

std::shared_ptr<ErrorDynamics::CodeScheme::PlanarError> IteratedDecoder::operator() (ErrorDynamics::PlanarData data) {
    const int t_total = data.first->size();
    const auto shape = data.second->get_shape();
    auto syndrome_graph_x = get_graph(
        data,
        shape,
        measurement_error,
        [](PlanarIndex3d a, PlanarIndex3d b)->std::pair<EdgeType, double> {
            return std::make_pair(EdgeType::IN, 0);
        },
        graph_x
    );
    auto syndrome_graph_z = get_graph(
        data,
        shape,
        measurement_error,
        [](PlanarIndex3d a, PlanarIndex3d b)->std::pair<EdgeType, double> {
            return std::make_pair(EdgeType::IN, 0);
        },
        graph_z
    );
    int current_graph = starting_graph;
    auto index = [&shape](int i, int j, int t)->int {
        return t * shape.x() * shape.y() + i * shape.y() + j;
    };

    /*
        weight_* is the edge weight for graph_*
        if (i, j) is a data qubit position,
        weight_*[i, j, t] is the weight between two spacial adjacent measure-* qubits
        if (i, j) is a measure-* qubit position,
        weight_*[i, j, t] is the weight between this measure-* qubit on time t - 1 and t
        otherwise it is undefined.
    */
    auto weight_x = std::vector<double>(shape.x() * shape.y() * (t_total + 1), 0.0);
    auto weight_z = std::vector<double>(shape.x() * shape.y() * (t_total + 1), 0.0);
    auto matching_x = std::list<int>();
    auto matching_z = std::list<int>();

    // update the edge weight FOR graph_*
    auto update_weight = [&, this](int graph_type, bool first = false)->void {
        auto& weight_update = (graph_type == graph_x ? weight_x : weight_z);

        double pbase = (graph_type == graph_x ? this->px : this->pz);
        double pcobase = (graph_type == graph_x ? this->pz : this->px);
        double logps = (first ? log(pbase + this->py) : log(pbase));
        double logpm = log(this->pm);
        // increases when p_y is larger.
        double logpadd = log(py) - log(pcobase);

        for(int t = 0; t < t_total + 1; t++) {
            for(int i = (graph_type == graph_x ? 1 : 0); i < shape.x(); i += 2)
                for(int j = ((i + 1) % 2); j < shape.y(); j += 2)
                    weight_update[index(i, j, t)] = -logpm;
            if(t == t_total)
                break;
            for(int i = 0; i < shape.x(); i++)
                for(int j = i % 2; j < shape.y(); j += 2)
                    weight_update[index(i, j, t)] = -logps;
        }

        if(first)
            return;
        
        auto& matching = (graph_type == graph_x ? matching_z : matching_x);
        auto& syndrome_graph = (graph_type == graph_x ? syndrome_graph_z : syndrome_graph_x);
        auto& graph = syndrome_graph->graph;
        auto& idx_lookup = syndrome_graph->index_lookup;
        auto& edge_type = syndrome_graph->edge_type;
        auto& weight = syndrome_graph->weight;
        double delta_logp = (-logpadd) - (-logps);

        // log of multinomial coefficient
        auto log_multi_3 = [](int a, int b, int c)->double {
            return lgamma(a + b + c + 1) - lgamma(a + 1) - lgamma(b + 1) - lgamma(c + 1);
        };

        for(auto m_it = matching.begin(); m_it != matching.end(); m_it++) {
            auto edge = graph.GetEdge(*m_it);
            int u = edge.first, v = edge.second;
            auto idx_u = idx_lookup[u];
            auto idx_v = idx_lookup[v];
            auto e_type = edge_type[*m_it];
            if(idx_v.virt) {
                if(e_type == EdgeType::TIME)
                    continue;
                bool axis = (idx_u.i() % 2 == 1); // true for index i, false for index j
                bool dir = get_chain_direction(e_type, 0);
                int delta = (dir ? 1 : -1);
                int bound = (dir ? (axis ? shape.x() : shape.y()) : -1);
                int current_t = idx_u.t();
                for(int current_idx = (axis ? idx_u.i() : idx_u.j()); current_idx != bound; current_idx += delta * 2)
                    weight_update[axis ? index(current_idx + delta, idx_u.j(), current_t) : index(idx_u.i(), current_idx + delta, current_t)] += delta_logp;
                continue;
            }
            if(e_type == EdgeType::IN) {
                // if there is no measurement error, no matching will cross the boundary.
                // so safe to consider any matching.

                int delta_x = abs(idx_v.i() - idx_u.i());
                int delta_y = abs(idx_v.j() - idx_u.j());
                int delta_t = abs(idx_v.t() - idx_u.t());
                int dx = (idx_v.i() - idx_u.i() > 0 ? 1 : -1);
                int dy = (idx_v.j() - idx_u.j() > 0 ? 1 : -1);
                int dt = (idx_v.t() - idx_u.t() > 0 ? 1 : -1);
                double logn_total = log_multi_3(delta_x, delta_y, delta_t);
                for(int i = 0; i < delta_x + 2; i += 2) {
                    int from_i = idx_u.i() + i * dx;
                    for(int j = 0; j < delta_y + 2; j += 2) {
                        int from_j = idx_u.j() + j * dy;
                        for(int t = 0; t <= delta_t; t += 1) {
                            int from_t = idx_u.t() + t * dt;
                            double logn_from = log_multi_3(i, j, t);
                            if(i != delta_x) {
                                double logn_tox = log_multi_3(delta_x - i - 1, delta_y - j, delta_t - t);
                                double correction_coef_x = exp(logn_tox + logn_from - logn_total); // <= 1
                                weight_update[index(from_i + dx, from_j, from_t)] += correction_coef_x * delta_logp;
                            }
                            if(j != delta_y) {
                                double logn_toy = log_multi_3(delta_x - i, delta_y - j - 1, delta_t - t);
                                double correction_coef_y = exp(logn_toy + logn_from - logn_total);
                                weight_update[index(from_i, from_j + dy, from_t)] += correction_coef_y * delta_logp;
                            }
                        }
                    }
                }
                
            } else if(e_type == EdgeType::TIME) {
                // no correction made
                continue;
            } else { // in space direction
                bool axis = (idx_u.i() % 2 == 1); // true for index i, false for index j
                bool dir = get_chain_direction(e_type, 0);
                int delta = (dir ? 1 : -1);
                int bound = (dir ? (axis ? shape.x() : shape.y()) : -1);
                int current_t = idx_u.t();
                for(int current_idx = (axis ? idx_u.i() : idx_u.j()); current_idx != bound; current_idx += delta * 2)
                    weight_update[axis ? index(current_idx + delta, idx_u.j(), current_t) : index(idx_u.i(), current_idx + delta, current_t)] += delta_logp;
                dir = get_chain_direction(e_type, 1);
                delta = (dir ? 1 : -1);
                bound = (dir ? (axis ? shape.x() : shape.y()) : -1);
                current_t = idx_v.t();
                for(int current_idx = (axis ? idx_v.i() : idx_v.j()); current_idx != bound; current_idx += delta * 2)
                    weight_update[axis ? index(current_idx + delta, idx_u.j(), current_t) : index(idx_u.i(), current_idx + delta, current_t)] += delta_logp;
            }
        }
    };
    
    // compute the edge weight FOR graph_*
    auto compute_weight = [&, this](int graph_type)->void {
        auto& weight_ref = (graph_type == graph_x ? weight_x : weight_z);
        auto& syndrome_graph = (graph_type == graph_x ? syndrome_graph_x : syndrome_graph_z);
        auto& graph = syndrome_graph->graph;
        auto& idx_lookup = syndrome_graph->index_lookup;
        auto& edge_type = syndrome_graph->edge_type;
        auto& weight = syndrome_graph->weight;
        
        // for every vertex, maintain a vector for distance to other vertices
        // also for edge to space boundary, edge to time boundary
        // for virtual vertex, distance to all boundary is set to 0
        // distance to all vertex is set to +inf

        // to_vertex, space, time
        // for direction, 0 is negative direction, 1 is positive direction
        auto vv_distance = std::vector<std::vector<double>>(graph.GetNumVertices(), std::vector<double>(graph.GetNumVertices(), INFINITY));
        auto vs_distance = std::vector<double>(graph.GetNumVertices(), INFINITY);
        auto vs_direction = std::vector<int>(graph.GetNumVertices(), 0);
        auto vt_distance = std::vector<double>(graph.GetNumVertices(), INFINITY);

        enum class BoundaryType {
            IN,
            SPACEN,
            SPACEP,
            TIME
        };

        struct Vertex {
            int i, j, t;
            Vertex(){}
            Vertex(int _i, int _j, int _t) {
                i = _i, j = _j, t = _t;
            }
        };

        struct Edge {
            int i, j, t;
            Vertex from, to;
            double to_dist;
            Edge(){}
            Edge(int _i, int _j, int _t, Vertex _from, Vertex _to, double _to_dist) {
                i = _i, j = _j, t = _t, from = _from, to = _to, to_dist = _to_dist;
            }
        };

        auto cmp = [&](Edge a, Edge b)->bool {
            return a.to_dist >= b.to_dist;
        };

        
        
        auto get_vertex_type = [&](Vertex v)->BoundaryType {
            if(v.i < 0 || v.j < 0)
                return BoundaryType::SPACEN;
            if(v.i >= shape.x() || v.j >= shape.y())
                return BoundaryType::SPACEP;
            if(v.t < 0 || v.t >= t_total)
                return BoundaryType::TIME;
            return BoundaryType::IN;
        };

        for(int v_id = 0; v_id < graph.GetNumVertices(); v_id++) {
            auto& v_idx = idx_lookup[v_id];
            if(v_idx.virt){
                for(int v_to_id = 0; v_to_id < graph.GetNumVertices(); v_to_id++)
                    vv_distance[v_id][v_to_id] = INFINITY;
                vs_distance[v_id] = 0;
                vt_distance[v_id] = 0;
                break;
            }
            auto edge_visited = std::vector<bool>(shape.x() * shape.y() * (t_total + 1), false);
            auto vertex_visited = std::vector<bool>(shape.x() * shape.y() * t_total, false);
            auto vertex_dist = std::vector<double>(shape.x() * shape.y() * t_total, INFINITY);
            bool space_bound_visited = false, time_bound_visited = false;

            Vertex first_vertex = Vertex(v_idx.i(), v_idx.j(), v_idx.t());
            vertex_visited[index(first_vertex.i, first_vertex.j, first_vertex.t)] = true;
            vertex_dist[index(first_vertex.i, first_vertex.j, first_vertex.t)] = 0;

            auto get_adj_edge = [&, this](Vertex v)->std::vector<Edge> {
                const int adj_v_delta[6][3] = {
                    2, 0, 0,
                    0, 2, 0,
                    0, 0, 1,
                    -2, 0, 0,
                    0, -2, 0,
                    0, 0, -1
                };
                const int adj_e_delta[6][3] = {
                    1, 0, 0,
                    0, 1, 0,
                    0, 0, 1,
                    -1, 0, 0,
                    0, -1, 0,
                    0, 0, 0
                };
                int N = 6;
                auto ret = std::vector<Edge>();
                for(int n = 0; n < N; n++) {
                    int e_i = v.i + adj_e_delta[n][0];
                    int e_j = v.j + adj_e_delta[n][1];
                    int e_t = v.t + adj_e_delta[n][2];
                    if(e_i < 0 || e_i >= shape.x())
                        continue;
                    if(e_j < 0 || e_j >= shape.y())
                        continue;
                    if(e_t < 0 || e_t > t_total)
                        continue;
                    if((e_i + e_j) % 2 == 1 && !this->measurement_error)
                        continue;
                    ret.push_back(Edge(e_i, e_j, e_t, v, Vertex(v.i + adj_v_delta[n][0], v.j + adj_v_delta[n][1], v.t + adj_v_delta[n][2]), weight_ref[index(e_i, e_j, e_t)] + vertex_dist[index(v.i, v.j, v.t)]));
                }
                return ret;
            };

            auto edge_heap = get_adj_edge(first_vertex);
            for(auto e_it = edge_heap.cbegin(); e_it != edge_heap.cend(); e_it++) {
                auto& e = *e_it;
                edge_visited[index(e.i, e.j, e.t)] = true;
            }
            std::make_heap(edge_heap.begin(), edge_heap.end(), cmp);

            while(!edge_heap.empty()) {
                std::pop_heap(edge_heap.begin(), edge_heap.end(), cmp);
                const auto current_e = edge_heap.back();
                edge_heap.pop_back();

                auto& v_from = current_e.from;
                auto& v_to = current_e.to;
                if(vertex_visited[index(v_to.i, v_to.j, v_to.t)]) // already visited
                    continue;
                auto vertex_type = get_vertex_type(v_to);
                if(vertex_type == BoundaryType::TIME) { // touching the time boundary
                    if(time_bound_visited)
                        continue;
                    time_bound_visited = true;
                    vt_distance[v_id] = vertex_dist[index(v_from.i, v_from.j, v_from.t)] + weight_ref[index(current_e.i, current_e.j, current_e.t)];
                } else if(vertex_type == BoundaryType::SPACEN || vertex_type == BoundaryType::SPACEP) { // touching the space boundary
                    if(space_bound_visited)
                        continue;
                    space_bound_visited = true;
                    vs_distance[v_id] = vertex_dist[index(v_from.i, v_from.j, v_from.t)] + weight_ref[index(current_e.i, current_e.j, current_e.t)];
                    vs_direction[v_id] = (vertex_type == BoundaryType::SPACEN ? 0 : 1);
                } else { // inside, add new edges
                    vertex_dist[index(v_to.i, v_to.j, v_to.t)] = vertex_dist[index(v_from.i, v_from.j, v_from.t)] + weight_ref[index(current_e.i, current_e.j, current_e.t)];
                    vertex_visited[index(v_to.i, v_to.j, v_to.t)] = true;
                    auto new_edge = get_adj_edge(v_to);
                    for(auto e_it = new_edge.cbegin(); e_it != new_edge.cend(); e_it++) {
                        auto& e = *e_it;
                        if(edge_visited[index(e.i, e.j, e.t)])
                            continue;
                        edge_visited[index(e.i, e.j, e.t)] = true;
                        edge_heap.push_back(e);
                        std::push_heap(edge_heap.begin(), edge_heap.end(), cmp);
                    }
                }

                for(int v_to_id = 0; v_to_id < graph.GetNumVertices(); v_to_id++) {
                    auto& v_to_idx = idx_lookup[v_to_id];
                    if(v_to_idx.virt)
                        vv_distance[v_id][v_to_id] = INFINITY;
                    else
                        vv_distance[v_id][v_to_id] = vertex_dist[index(v_to_idx.i(), v_to_idx.j(), v_to_idx.t())];
                }
            }
        };

        for(int e_id = 0; e_id < graph.GetNumEdges(); e_id++) {
            auto edge = graph.GetEdge(e_id);
            int u = edge.first, v = edge.second;
            auto idx_u = idx_lookup[u];
            auto idx_v = idx_lookup[v];

            int dx = abs(idx_u.i() - idx_v.i()) / 2;
            int dy = abs(idx_u.j() - idx_v.j()) / 2;
            int dt = abs(idx_u.t() - idx_v.t());

            double weight_dist = vv_distance[u][v];
            double weight_degen = -(lgamma(dx + dy + dt + 1) - lgamma(dx + 1) - lgamma(dy + 1) - lgamma(dt + 1));
            //double weight_degen = 0;
            double weight_in = weight_dist + weight_degen;

            double weight_space = vs_distance[u] + vs_distance[v];
            double weight_time = vt_distance[u] + vt_distance[v];

            int space_direction = 4;
            space_direction += (vs_direction[u] * 2 + vs_direction[v]);

            auto weight_vec = std::vector<double>({weight_in, weight_time, weight_space});
            int argmin = distance(weight_vec.begin(), min_element(weight_vec.begin(), weight_vec.end()));
            auto e_type = (EdgeType)(argmin != 2 ? argmin : space_direction);

            weight[e_id] = weight_vec[argmin];
            edge_type[e_id] = e_type;
        }

    };

    auto show_graph = [&](int graph_type)->void {
        auto& syndrome_graph = (graph_type == graph_x ? syndrome_graph_x : syndrome_graph_z);
        auto& graph = syndrome_graph->graph;
        auto& idx_lookup = syndrome_graph->index_lookup;
        auto& edge_type = syndrome_graph->edge_type;
        auto& weight = syndrome_graph->weight;

        if(graph_type == graph_x)
            printf("graph X:\n\n");
        else
            printf("graph Z:\n\n");
        
        printf("Vertex:\n");
        for(int i = 0; i < graph.GetNumVertices(); i++) {
            auto v_idx = idx_lookup[i];
            if(v_idx.virt)
                printf("%4d virtual\n", i);
            else
                printf("%4d i = %4d j = %4d t = %4d\n", i, v_idx.i(), v_idx.j(), v_idx.t());
        }

        printf("\nEdge:\n");
        for(int i = 0; i < graph.GetNumEdges(); i++) {
            auto edge = graph.GetEdge(i);
            printf("%4d: %4d %4d, weight = %8.5f, type = %1d\n", i, edge.first, edge.second, weight[i], (int)edge_type[i]);
        }
    };
    auto show_matching = [&](int graph_type)->void {
        auto& syndrome_graph = (graph_type == graph_x ? syndrome_graph_x : syndrome_graph_z);
        auto& graph = syndrome_graph->graph;
        auto& edge_type = syndrome_graph->edge_type;
        auto& weight = syndrome_graph->weight;
        auto& matching = (graph_type == graph_x ? matching_x : matching_z);

        if(graph_type == graph_x)
            printf("matching X:\n\n");
        else
            printf("matching Z:\n\n");

        double total_weight = 0;
        for(auto e_it = matching.cbegin(); e_it != matching.cend(); e_it++) {
            int e_id = *e_it;
            auto edge = graph.GetEdge(e_id);
            printf("%4d: %4d %4d, weight = %8.5f, type = %1d\n", e_id, edge.first, edge.second, weight[e_id], (int)edge_type[e_id]);
            total_weight += weight[e_id];
        }
        printf("total weight: %.5f", total_weight);
    };
    auto show_weight = [&](int graph_type)->void {
        auto& weight_ref = (graph_type == graph_x ? weight_x : weight_z);
        if(graph_type == graph_x)
            printf("weight X:\n\n");
        else
            printf("weight Z:\n\n");
        printf("x = %3d, y = %3d, total t = %3d\n\n", shape.x(), shape.y(), t_total);

        for(int t = 0; t < t_total + 1; t++) {
            printf("t = %3d\n", t);
            for(int i = 0; i < shape.x(); i++) {
                for(int j = 0; j < shape.y(); j++) {
                    printf("%5.2f   ", weight_ref[index(i, j, t)]);
                }
                printf("\n");
            }
            printf("\n");
        }
    };

    for(int current_iter = 0; current_iter < n_iter; current_iter++) {
        // update corrections for the this graph
        update_weight(current_graph, current_iter == 0 ? true : false);
        //show_weight(current_graph);
        //printf("\n\n");
        // compute new weight for the graph to match
        compute_weight(current_graph);
        //show_graph(current_graph);
        //printf("\n\n");
        // compute matching
        auto syndrome_graph = (current_graph == graph_x ? syndrome_graph_x : syndrome_graph_z);
        auto& current_matching = (current_graph == graph_x ? matching_x : matching_z);
        auto matching_algorithm = MWPM::Matching(syndrome_graph->graph);
        current_matching = matching_algorithm.SolveMinimumCostPerfectMatching(syndrome_graph->weight).first;
        //show_matching(current_graph);
        //printf("\n\n");
        // switch graph
        current_graph = (current_graph == graph_x ? graph_z : graph_x);
    }

    auto correction_x = matching_to_correction(syndrome_graph_x, shape, matching_x);
    auto correction_z = matching_to_correction(syndrome_graph_z, shape, matching_z);
    
    return correction_x * correction_z;
}


}