#pragma once

#include <memory>
#include <vector>
#include <utility>
#include <string>
#include "util.hpp"

namespace ErrorDynamics {

class PlanarSurfaceCode;

namespace CodeScheme {

class PlanarIndex {
    int _i, _j;

    public:
    PlanarIndex() = delete;
    inline PlanarIndex(int __i, int __j) { _i = __i, _j = __j; }
    inline const int& i() const { return _i; }
    inline const int& j() const { return _j; }
    inline PlanarIndex i_move(int d) const { return PlanarIndex(_i + d, _j); }
    inline PlanarIndex j_move(int d) const { return PlanarIndex(_i, _j + d); }
};

class PlanarShape {
    int _x, _y;
    public:
    PlanarShape() = delete;
    inline PlanarShape(int __x, int __y) { _x = __x, _y = __y; }
    inline const int& x() const { return _x; }
    inline const int& y() const { return _y; }
    inline bool operator==(const PlanarShape& other) const {
        return _x == other._x && _y == other._y;
    }
};

class PlanarScheme;
class PlanarSyndrome;
class PlanarError;

class PlanarScheme {
    /*
    A planar surface code scheme on a x by y qubits array.
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

    friend PlanarSurfaceCode;

    int x, y;
    std::shared_ptr<PlanarSyndrome> syndrome, syndrome_error;
    std::shared_ptr<PlanarError> data_error;
    inline bool valid_index(PlanarIndex index){
        return index.i() >= 0 && index.i() < x && index.j() >= 0 && index.j() < y;
    }

    public:
    PlanarScheme() = delete;
    PlanarScheme(int _x, int _y);
    PlanarScheme(int _d);

    inline Util::QubitType qubit_type(PlanarIndex index) const {
        const int& i = index.i();
        const int& j = index.j();
        if((i + j) % 2 == 0)
            return Util::QubitType::DATA;
        if(i % 2 == 0)
            return Util::QubitType::MEASURE_Z;
        return Util::QubitType::MEASURE_X;
    }
    inline const PlanarShape get_shape() const {
        return PlanarShape(x, y);
    }

    std::shared_ptr<PlanarSyndrome> get_syndrome() const;
    void add_data_error(PlanarIndex index, Util::Pauli pauli);
    void add_data_error(std::shared_ptr<PlanarError> _data_error);
    void add_syndrome_error(PlanarIndex index);
    void add_syndrome_error(std::shared_ptr<PlanarSyndrome> _syndrome_error);
    void clear_syndrome_error();

    // return true if all the symptoms are POSITIVE
    bool is_valid() const;

    // return true if no logical error happened.
    bool is_correct() const;

    // provide a string about all the info. colorization based on ANSI color code
    std::string to_string(bool color = false, int interval = 1) const;
};

class PlanarSyndrome {
    int x, y;
    std::vector<int> list;
    public:
    PlanarSyndrome() = delete;
    PlanarSyndrome(int _x, int _y);
    PlanarSyndrome(int _d);
    PlanarSyndrome(const PlanarSyndrome& other);

    inline const PlanarShape get_shape() const {
        return PlanarShape(x, y);
    }
    inline Util::Symptom get_symptom(PlanarIndex index) const {
        return (Util::Symptom)list[index.i() * y + index.j()];
    }
    inline void change_symptom(PlanarIndex index) {
        list[index.i() * y + index.j()] = 1 - list[index.i() * y + index.j()];
    }
    inline void change_symptom(PlanarIndex index, Util::Symptom symptom) {
        if(symptom == Util::Symptom::NEGATIVE)
            change_symptom(index);
    }
    inline std::vector<int> to_vector() const {
        return std::vector<int>(list);
    }

    std::string to_string(bool color = false, int interval = 1) const;
};

class PlanarError {
    int x, y;
    std::vector<int> list;
    public:
    PlanarError() = delete;
    PlanarError(int _x, int _y);
    PlanarError(int _x, int _y, const std::vector<int>& _list);
    PlanarError(int _d);
    PlanarError(const PlanarError& other);

    inline const PlanarShape get_shape() const {
        return PlanarShape(x, y);
    }
    inline void set_error(PlanarIndex index, Util::Pauli pauli) {
        list[index.i() * y + index.j()] = (int)pauli;
    }
    inline void mult_error(PlanarIndex index, Util::Pauli pauli) {
        list[index.i() * y + index.j()] = (int)(pauli * (Util::Pauli)list[index.i() * y + index.j()]);
    }
    inline Util::Pauli get_error(PlanarIndex index) const {
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

inline std::shared_ptr<PlanarSyndrome> operator^(std::shared_ptr<PlanarSyndrome> a, std::shared_ptr<PlanarSyndrome> b) {
    if(!(a->get_shape() == b->get_shape()))
        throw Util::BadShape(std::string("The two syndromes should have the same shape."));
    auto ret = std::make_shared<PlanarSyndrome>(*a);
    for(int i = 0; i < b->get_shape().x(); i++) {
        for(int j = (i + 1) % 2; j < b->get_shape().y(); j += 2) {
            ret->change_symptom(PlanarIndex(i, j), b->get_symptom(PlanarIndex(i, j)));
        }
    }
    return ret;
}

inline std::shared_ptr<PlanarError> operator*(std::shared_ptr<PlanarError> a, std::shared_ptr<PlanarError> b) {
    if(!(a->get_shape() == b->get_shape()))
        throw Util::BadShape(std::string("The two syndromes should have the same shape."));
    auto ret = std::make_shared<PlanarError>(*a);
    for(int i = 0; i < b->get_shape().x(); i++) {
        for(int j = (i % 2); j < b->get_shape().y(); j++) {
            ret->mult_error(PlanarIndex(i, j), b->get_error(PlanarIndex(i, j)));
        }
    }
    return ret;
}

}}