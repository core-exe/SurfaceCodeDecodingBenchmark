#include "error_dynamics.hpp"
#include <iostream>


using namespace std;
using namespace ErrorDynamics;

int main() {
    int t_total = 10;
    auto code = RectangularSurfaceCode(11, 0.01);
    code.step(t_total);
    auto data = code.get_data();

    cout << "Error: " << endl;
    cout << code.to_string(true, 2) << endl << endl;

    int t = 0;
    for(auto p = data.first->cbegin(); p != data.first->cend(); p++) {
        cout << "Syndrome at t = " << (t++) << endl;
        cout << (*p)->to_string(true, 2) << endl;
    }
    return 0;
}