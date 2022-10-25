#include "error_dynamics.hpp"
#include "decoder.hpp"
#include <iostream>

#define MODE 0 // 0 for randomly generated errors.

using namespace std;
namespace Err = ErrorDynamics;
namespace Dc = Decoder;

int main() {
    int t_total = 1;
    double p = exp(-3.5);
    int d = 9;
    auto error_model_ptr = new Err::ErrorModel::IIDError(p, p, p, 0);
    shared_ptr<Err::ErrorModel::ErrorModelBase> error_model(error_model_ptr);
    auto code = Err::PlanarSurfaceCode(d, error_model);
    if(MODE == 0)
        code.step(t_total);
    else {
        auto d_error = make_shared<Err::CodeScheme::PlanarError>(d);
        auto m_error = make_shared<Err::CodeScheme::PlanarSyndrome>(d);
        //d_error->mult_error(Err::CodeScheme::PlanarIndex(0, 0), Err::Util::Pauli::Y);
        code.manual_step(d_error, m_error);
    }

    auto decoder = Dc::Matching::IteratedDecoder(p, p, p, 0, false, 2);
    auto data = code.get_data();
    cout << "Error: " << endl;
    cout << code.to_string(true, 1) << endl << endl;

    int t = 0;
    for(auto p = data.first->cbegin(); p != data.first->cend(); p++) {
        cout << "Syndrome at t = " << (t++) << endl;
        cout << (*p)->to_string(true, 1) << endl;
    }

    auto correction = decoder(data);
    code.apply_correction(correction);
    
    cout << "Corrected: " << endl;
    cout << code.to_string(true, 1) << endl << endl;
}