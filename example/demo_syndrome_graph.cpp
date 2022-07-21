#include "error_dynamics.hpp"
#include "decoder.hpp"
#include <iostream>
#include <functional>
#include <cmath>

using namespace std;
using namespace ErrorDynamics;
using namespace Decoder;

int main() {
    int t_total = 1;
    double p = 0.01;
    auto code = RectangularSurfaceCode(7, p);
    code.step(t_total);
    auto data = code.get_data();

    cout << "Error: " << endl;
    cout << code.to_string(true, 2) << endl << endl;

    int t = 0;
    for(auto p = data.first->cbegin(); p != data.first->cend(); p++) {
        cout << "Syndrome at t = " << (t++) << endl;
        cout << (*p)->to_string(true, 2) << endl;
    }

    auto shape = code.get_shape();
    auto dist_func = [p, shape](Matching::RectIndex3d idx_a, Matching::RectIndex3d idx_b) -> pair<bool, double> {
        if((idx_a.i() - idx_b.i()) % 2 != 0) {
            return make_pair(false, (double)0.0);
        }
        return make_pair(true, -((abs(idx_a.i() - idx_b.i()) + abs(idx_a.j() - idx_b.j())) / 2) * (double)log(p));
    };
    auto dist_func_space = [p, shape](Matching::RectIndex3d idx_a) -> pair<int, double> {
        int pos = ((idx_a.i() % 2 == 1) ? idx_a.i() : idx_a.j());
        int length = ((idx_a.i() % 2 == 1) ? shape.x() : shape.y());
        int direction = ((2 * pos) <= length - 1 ? 0 : 1);
        int distance = (((direction == 0) ? pos : (length - 1) - pos) + 1) / 2;
        return make_pair(direction, - distance * (double)log(p));
    };
    auto dist_func_time = [](Matching::RectIndex3d idx_a) -> pair<int, double> {
        return make_pair(0, (double)0.0);
    };
    auto syndrome_graph = Matching::get_graph(
        data,
        shape,
        false,
        dist_func,
        dist_func_space,
        dist_func_time
    );

    return 0;

}