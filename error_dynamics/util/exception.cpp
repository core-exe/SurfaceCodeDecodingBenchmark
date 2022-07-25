#include "exception.hpp"

namespace ErrorDynamics{
namespace Util{

BadShape::BadShape(std::string _info){
    info = _info;
}

BadShape& BadShape::operator=(const BadShape& other){
    info = other.info;
    return *this;
}

const char* BadShape::what() const noexcept{
    return info.c_str();
}

BadType::BadType(std::string _info){
    info = _info;
}

BadType& BadType::operator=(const BadType& other){
    info = other.info;
    return *this;
}

const char* BadType::what() const noexcept{
    return info.c_str();
}

BadIndex::BadIndex(std::string _info){
    info = _info;
}

BadIndex& BadIndex::operator=(const BadIndex& other){
    info = other.info;
    return *this;
}

const char* BadIndex::what() const noexcept{
    return info.c_str();
}

}}