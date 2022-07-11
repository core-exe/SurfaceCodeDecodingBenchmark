#include "error_dynamics.hpp"
#include <iostream>

using namespace std;

using namespace ErrorDynamics;



int main(){
    for(int _ = 0; _ < 10; _++){
        int x = 2;
    }
    auto scheme = CodeScheme::RectScheme(7);
    cout << scheme.to_string(true, 2) << endl;

    while(true) {
        int i, j;
        char gate;
        cin >> gate >> i >> j;
        Util::Pauli pauli;
        if(gate == 'x')
            pauli = Util::Pauli::X;
        else if(gate == 'y')
            pauli = Util::Pauli::Y;
        else if(gate == 'z')
            pauli = Util::Pauli::Z;
        else if(gate == 'i')
            pauli = Util::Pauli::I;
        else if(gate == 'e')
            break;
        else
            continue;
        try {
            scheme.add_data_error(CodeScheme::RectIndex(i, j), pauli);
        }
        catch(const std::exception& e) {
            cout << e.what() << '\n';
            continue;
        }
        cout << scheme.to_string(true, 2) << endl;
    }


    return 0;
}