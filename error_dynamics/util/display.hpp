#pragma once

#include <string>
#include "constant.hpp"

namespace ErrorDynamics{
namespace Util{

inline std::string to_string(bool color, Pauli p) {
    if(p == Pauli::I)
        return std::string("I");
    if(p == Pauli::X)
        return color ? std::string("\033[1;33mX\033[0m") : std::string("X");
    if(p == Pauli::Y)
        return color ? std::string("\033[1;32mY\033[0m") : std::string("Y");
    return color ? std::string("\033[1;34mZ\033[0m") : std::string("Z");
}

inline std::string to_string(bool color, Symptom p) {
    if(p == Symptom::POSITIVE)
        return std::string("+");
    return color ? std::string("\033[1;30;41m-\033[0m") : std::string("-");
}

inline std::string show_interval_h(bool color, bool error, int interval) {
    if(interval == 0)
        return std::string("");
    auto str = std::string(2 * interval - 1, '-');
    str = " " + str + " ";
    if(color) {
        if(!error)
            str = "\033[90m" + str + "\033[0m";
        else
            str = "\033[91m" + str + "\033[0m";
    }
    return str;
}

inline std::string show_interval_v(bool color, bool error) {
    auto str = std::string("|");
    if(color) {
        if(!error)
            str = "\033[90m" + str + "\033[0m";
        else
            str = "\033[91m" + str + "\033[0m";
    }
    return str;
}

}}