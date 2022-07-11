#pragma once

#include <string>

namespace ErrorDynamics{
namespace Util{

enum class QubitType{
    DATA, MEASURE_X, MEASURE_Z
};

enum class Pauli{
    I = 0,
    X = 1,
    Y = 2,
    Z = 3
};

static const Pauli pauli_mult[4][4] = {
    Pauli::I, Pauli::X, Pauli::Y, Pauli::Z,
    Pauli::X, Pauli::I, Pauli::Z, Pauli::Y,
    Pauli::Y, Pauli::Z, Pauli::I, Pauli::X,
    Pauli::Z, Pauli::Y, Pauli::X, Pauli::I,
}; // ignore global phases

inline Pauli operator*(Pauli e1, Pauli e2){
    return pauli_mult[(int)e1][(int)e2];
}

inline bool is_xy(Pauli p){
    return (p == Pauli::X) || (p == Pauli::Y);
}

inline bool is_zy(Pauli p){
    return (p == Pauli::Z) || (p == Pauli::Y);
}

enum class Symptom{
    POSITIVE = 0,
    NEGATIVE = 1
};

}}