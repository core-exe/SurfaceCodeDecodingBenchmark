#include <exception>
#include <string>

namespace ErrorDynamics{
namespace Util{

class BadShape: public std::exception{
    private:
    std::string info;
    
    public:
    BadShape(std::string _info);
    BadShape& operator=(const BadShape& other);
    const char* what() const noexcept;
};

class BadType: public std::exception{
    private:
    std::string info;
    
    public:
    BadType(std::string _info);
    BadType& operator=(const BadType& other);
    const char* what() const noexcept;
};

class BadIndex: public std::exception{
    private:
    std::string info;
    
    public:
    BadIndex(std::string _info);
    BadIndex& operator=(const BadIndex& other);
    const char* what() const noexcept;
};

}}