#pragma once

#include <memory>
#include <vector>
#include <utility>
#include <string>
#include "util.hpp"

namespace ErrorDynamics {

class RectangularSurfaceCode;

namespace CodeScheme {

class RectangularIndex {
    int _i, _j;

    public:
    RectangularIndex() = delete;
    inline RectangularIndex(int __i, int __j) { _i = __i, _j = __j; }
    inline const int& i() const { return _i; }
    inline const int& j() const { return _j; }
    inline RectangularIndex i_move(int d) const { return RectangularIndex(_i + d, _j); }
    inline RectangularIndex j_move(int d) const { return RectangularIndex(_i, _j + d); }
};

class RectangularShape {
    int _x, _y;
    public:
    RectangularShape() = delete;
    inline RectangularShape(int __x, int __y) { _x = __x, _y = __y; }
    inline const int& x() const { return _x; }
    inline const int& y() const { return _y; }
    inline bool operator==(const RectangularShape& other) const {
        return _x == other._x && _y == other._y;
    }
};

class RectangularScheme;
class RectangularSyndrome;
class RectangularError;

class RectangularScheme {
    /*
    A rectangular surface code scheme on a x by y qubits array.
    The array have x rows and y columns, x and y are both odd integers.
    The first and final rows are Z-edges.
    The first and final columns are X-edges.
    
    With x = 3, y = 5, the qubit types of the array are:

    D - Z - D - Z - D
    |   |   |   |   |
    X - D - X - D - X
    |   |   |   |   |
    D - Z - D - Z - D
    */

    friend RectangularSurfaceCode;

    int x, y;
    std::shared_ptr<RectangularSyndrome> syndrome, syndrome_error;
    std::shared_ptr<RectangularError> data_error;
    inline bool valid_index(RectangularIndex index){
        return index.i() >= 0 && index.i() < x && index.j() >= 0 && index.j() < y;
    }

    public:
    RectangularScheme() = delete;
    RectangularScheme(int _x, int _y);
    RectangularScheme(int _d);

    inline Util::QubitType qubit_type(RectangularIndex index) const {
        const int& i = index.i();
        const int& j = index.j();
        if((i + j) % 2 == 0)
            return Util::QubitType::DATA;
        if(i % 2 == 0)
            return Util::QubitType::MEASURE_Z;
        return Util::QubitType::MEASURE_X;
    }
    inline const RectangularShape get_shape() const {
        return RectangularShape(x, y);
    }

    std::shared_ptr<RectangularSyndrome> get_syndrome() const;
    void add_data_error(RectangularIndex index, Util::Pauli pauli);
    void add_data_error(std::shared_ptr<RectangularError> _data_error);
    void add_syndrome_error(RectangularIndex index);
    void add_syndrome_error(std::shared_ptr<RectangularSyndrome> _syndrome_error);
    void clear_syndrome_error();

    // return true if all the symptoms are POSITIVE
    bool is_valid() const;

    // return true if no logical error happened.
    bool is_correct() const;

    // provide a string about all the info. colorization based on ANSI color code
    std::string to_string(bool color = false, int interval = 1) const;
};

class RectangularSyndrome {
    int x, y;
    std::vector<int> list;
    public:
    RectangularSyndrome() = delete;
    RectangularSyndrome(int _x, int _y);
    RectangularSyndrome(int _d);
    RectangularSyndrome(const RectangularSyndrome& other);

    inline const RectangularShape get_shape() const {
        return RectangularShape(x, y);
    }
    inline Util::Symptom get_symptom(RectangularIndex index) const {
        return (Util::Symptom)list[index.i() * y + index.j()];
    }
    inline void change_symptom(RectangularIndex index) {
        list[index.i() * y + index.j()] = 1 - list[index.i() * y + index.j()];
    }
    inline void change_symptom(RectangularIndex index, Util::Symptom symptom) {
        if(symptom == Util::Symptom::NEGATIVE)
            change_symptom(index);
    }
    inline std::vector<int> to_vector() const {
        return std::vector<int>(list);
    }

    std::string to_string(bool color = false, int interval = 1) const;
};

class RectangularError {
    int x, y;
    std::vector<int> list;
    public:
    RectangularError() = delete;
    RectangularError(int _x, int _y);
    RectangularError(int _x, int _y, std::vector<int> _list);
    RectangularError(int _d);
    RectangularError(const RectangularError& other);

    inline const RectangularShape get_shape() const {
        return RectangularShape(x, y);
    }
    inline void set_error(RectangularIndex index, Util::Pauli pauli) {
        list[index.i() * y + index.j()] = (int)pauli;
    }
    inline void mult_error(RectangularIndex index, Util::Pauli pauli) {
        list[index.i() * y + index.j()] = (int)(pauli * (Util::Pauli)list[index.i() * y + index.j()]);
    }
    inline Util::Pauli get_error(RectangularIndex index) const {
        return (Util::Pauli)list[index.i() * y + index.j()];
    }
    inline std::vector<int> to_vector() const {
        return std::vector<int>(list);
    }

    std::vector<int> count_errors() const;

    // return true if all the symptoms are POSITIVE
    bool is_valid() const;

    // return true if no logical error happened.
    bool is_correct() const;
    Util::Pauli logical_error() const;    
};

inline std::shared_ptr<RectangularSyndrome> operator^(std::shared_ptr<RectangularSyndrome> a, std::shared_ptr<RectangularSyndrome> b) {
    if(!(a->get_shape() == b->get_shape()))
        throw Util::BadShape(std::string("The two syndromes should have the same shape."));
    auto ret = std::make_shared<RectangularSyndrome>(*a);
    for(int i = 0; i < b->get_shape().x(); i++) {
        for(int j = (i + 1) % 2; j < b->get_shape().y(); j += 2) {
            ret->change_symptom(RectangularIndex(i, j), b->get_symptom(RectangularIndex(i, j)));
        }
    }
    return ret;
}

inline std::shared_ptr<RectangularError> operator*(std::shared_ptr<RectangularError> a, std::shared_ptr<RectangularError> b) {
    if(!(a->get_shape() == b->get_shape()))
        throw Util::BadShape(std::string("The two syndromes should have the same shape."));
    auto ret = std::make_shared<RectangularError>(*a);
    for(int i = 0; i < b->get_shape().x(); i++) {
        for(int j = (i % 2); j < b->get_shape().y(); j++) {
            ret->mult_error(RectangularIndex(i, j), b->get_error(RectangularIndex(i, j)));
        }
    }
    return ret;
}

using RectScheme = RectangularScheme;
using RectSyndrome = RectangularSyndrome;
using RectError = RectangularError;
using RectIndex = RectangularIndex;
using RectShape = RectangularShape;

}}