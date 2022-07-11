#include <memory>
#include <vector>
#include <string>
#include "rectangular_scheme.hpp"

namespace ErrorDynamics{
namespace CodeScheme{

RectangularScheme::RectangularScheme(int _x, int _y) {
    if(_x % 2 == 0 || _y % 2 == 0)
        throw Util::BadShape(std::string("The length and width of the scheme should be odd."));
    x = _x, y = _y;
    syndrome = std::make_shared<RectangularSyndrome>(x, y);
    syndrome_error = std::make_shared<RectangularSyndrome>(x, y);
    data_error = std::make_shared<RectangularError>(x, y);
}

RectangularScheme::RectangularScheme(int _d) : RectangularScheme(_d, _d) {}

std::shared_ptr<RectangularSyndrome> RectangularScheme::get_syndrome() const {
    auto corrupted_syndrome = std::make_shared<RectangularSyndrome>(*syndrome);
    // corrupt Z-syndromes
    for(int i = 0; i < x; i++) {
        for(int j = (i + 1) % 2; j < y; j += 2) {
            if(syndrome_error->get_symptom(RectangularIndex(i, j)) == Util::Symptom::NEGATIVE) {
                corrupted_syndrome->change_symptom(RectangularIndex(i, j));
            }
        }
    }
    return corrupted_syndrome;
}

void RectangularScheme::add_data_error(RectangularIndex index, Util::Pauli pauli) {
    if(qubit_type(index) != Util::QubitType::DATA)
        throw Util::BadIndex(std::string("Requires index on data qubit."));
    data_error->mult_error(index, pauli);
    if((Util::is_xy(pauli) && index.i() % 2 == 1) || (Util::is_zy(pauli) && index.i() % 2 == 0)) {
        auto new_index = index.i_move(-1);
        if(valid_index(new_index))
            syndrome->change_symptom(new_index);
        new_index = index.i_move(1);
        if(valid_index(new_index))
            syndrome->change_symptom(new_index);
    }
    if((Util::is_zy(pauli) && index.i() % 2 == 1) || (Util::is_xy(pauli) && index.i() % 2 == 0)) {
        auto new_index = index.j_move(-1);
        if(valid_index(new_index))
            syndrome->change_symptom(new_index);
        new_index = index.j_move(1);
        if(valid_index(new_index))
            syndrome->change_symptom(new_index);
    }
}

void RectangularScheme::add_data_error(std::shared_ptr<RectangularError> _data_error) {
    for(int i = 0; i < x; i++) {
        for(int j = i % 2; j < y; j += 2) {
            add_data_error(RectangularIndex(i, j), _data_error->get_error(RectangularIndex(i, j)));
        }
    }
}

void RectangularScheme::add_syndrome_error(RectangularIndex index) {
    syndrome_error->change_symptom(index);
}

void RectangularScheme::add_syndrome_error(std::shared_ptr<RectangularSyndrome> _syndrome_error) {
    for(int i = 0; i < x; i++) {
        for(int j = (i + 1) % 2; j < y; j += 2) {
            if(_syndrome_error->get_symptom(RectangularIndex(i, j)) == Util::Symptom::NEGATIVE) {
                syndrome_error->change_symptom(RectangularIndex(i, j));
            }
        }
    }
}

void RectangularScheme::clear_syndrome_error() {
    syndrome_error = std::make_shared<RectangularSyndrome>(x, y);
}

bool RectangularScheme::is_valid() const {
    for(int i = 0; i < x; i++) {
        for(int j = (i + 1) % 2; j < y; j += 2) {
            if(syndrome->get_symptom(RectangularIndex(i, j)) == Util::Symptom::NEGATIVE) {
                return false;
            }
        }
    }
    return true;
}

bool RectangularScheme::is_correct() const {
    // X error
    int x_counter = 0;
    for(int i = 0; i < x; i += 2) {
        if(Util::is_xy(data_error->get_error(RectangularIndex(i, 0)))) {
            x_counter++;
        }
    }
    if(x_counter % 2 == 1)
        return false;
    int z_counter = 0;
    for(int j = 0; j < y; j += 2) {
        if(Util::is_zy(data_error->get_error(RectangularIndex(0, j)))) {
            z_counter++;
        }
    }
    if(z_counter % 2 == 1)
        return false;
    return true;
}

std::string RectangularScheme::to_string(bool color, int interval) const {
    std::string ret = std::string("");
    for(int i = 0; i < x; i++) {
        if(i != 0)
            ret += Util::show_interval_v(color, interval, y);
        for(int j = 0; j < y; j++) {
            auto index = RectangularIndex(i, j);
            if(j != 0)
                ret += Util::show_interval_h(color, interval);
            if(qubit_type(index) == Util::QubitType::DATA)
                ret += Util::to_string(color, data_error->get_error(index));
            else
                ret += Util::to_string(color, syndrome->get_symptom(index));
        }
        ret += "\n";
    }
    return ret;
}

RectangularSyndrome::RectangularSyndrome(int _x, int _y) {
    x = _x, y = _y;
    list = std::vector<std::vector<int>>(x, std::vector<int>(y, 0));
}

RectangularSyndrome::RectangularSyndrome(int _d) : RectangularSyndrome::RectangularSyndrome(_d, _d) {}

RectangularSyndrome::RectangularSyndrome(const RectangularSyndrome& other) {
    x = other.x;
    y = other.y;
    list = other.list;
}

RectangularError::RectangularError(int _x, int _y) {
    x = _x, y = _y;
    list = std::vector<std::vector<int>>(x, std::vector<int>(y, 0));
}

RectangularError::RectangularError(int _d) : RectangularError::RectangularError(_d, _d) {}

}}