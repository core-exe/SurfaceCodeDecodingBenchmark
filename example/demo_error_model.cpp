#include "error_dynamics.hpp"
#include <iostream>

using namespace std;

using namespace ErrorDynamics;


int main(){
    auto scheme = CodeScheme::RectScheme(11);
    cout << scheme.to_string(true, 1) << endl;
    float p = 0.01;
    auto error_model = ErrorModel::IIDError(p);

    int t = 0;
    char c;
    while(true) {
        c = getchar();
        if(c == 'e')
            break;
        auto errors = error_model.generate_rectangular_error(scheme.get_shape());
        scheme.add_data_error(errors.first);
        scheme.add_syndrome_error(errors.second);
        cout << "t = " << t << endl;
        cout << "Error: " << endl;
        cout << scheme.to_string(true, 1) << endl;
        cout << "Syndrome: " << endl;
        cout << scheme.get_syndrome()->to_string(true, 1) << endl;
        scheme.clear_syndrome_error();
        cout << "\n\n" << endl;
        t++;
    }

    return 0;
}