#include "error_dynamics.hpp"
#include "decoder.hpp"
#include <iostream>

#define MODE 0 // 0 for randomly generated errors.

using namespace std;
namespace Err = ErrorDynamics;
namespace Dc = Decoder;

int main() {
    int t_total = 5;
    double p = 0.05;
    int d = 9;
    auto error_model_ptr = new Err::ErrorModel::IIDError(p / 3, p / 3, p / 3, 2 * p / 3);
    shared_ptr<Err::ErrorModel::ErrorModelBase> error_model(error_model_ptr);
    auto code = Err::PlanarSurfaceCode(9, error_model);
    if(MODE == 0)
        code.step(t_total);
    else {
        auto d_error = make_shared<Err::CodeScheme::PlanarError>(d);
        auto m_error = make_shared<Err::CodeScheme::PlanarSyndrome>(d);
        //d_error->mult_error(Err::CodeScheme::PlanarIndex(0, 4), Err::Util::Pauli::X);
        code.manual_step(d_error, m_error);
    }

    auto decoder_standard = Dc::Matching::StandardMWPMDecoder(p / 3, p / 3, p / 3, 2 * p / 3, true, t_total, code.get_shape());
    auto decoder_code_space = Dc::Matching::StandardMWPMDecoder(p / 3, p / 3, p / 3, 0, false, 1, code.get_shape());
    auto data_standard = code.get_data();
    cout << "Error: " << endl;
    cout << code.to_string(true, 1) << endl << endl;

    int t = 0;
    for(auto p = data_standard.first->cbegin(); p != data_standard.first->cend(); p++) {
        cout << "Syndrome at t = " << (t++) << endl;
        cout << (*p)->to_string(true, 1) << endl;
    }

    auto correction_standard = decoder_standard(data_standard);
    code.apply_correction(correction_standard);
    cout << "Corrected (standard): " << endl;
    cout << code.to_string(true, 1) << endl << endl;

    auto data_code_space = code.get_data();
    auto correction_code_space = decoder_code_space(data_code_space);
    code.apply_correction(correction_code_space);
    cout << "Corrected (code space): " << endl;
    cout << code.to_string(true, 1) << endl << endl;
    
}