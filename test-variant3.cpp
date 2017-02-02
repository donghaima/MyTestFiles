
// To build:
// g++ -I /nobackup/doma/TRUNK/cme-utest/cme/externalLibs/boost/boost_1_53_0_built/ -g -o test-map test-map.cpp

#include <iostream>
#include <string>
#include <boost/variant.hpp>

typedef boost::variant<int, double> Type;


// visitor class has extra members

class Append: public boost::static_visitor<>
{
public:
    void operator()(int& i)
    {
      std::cout << argument << " = " << i << std::endl;
    }

    void operator()(double& d)
    {
      std::cout << argument << " = " << d << std::endl;
    }
    std::string argument;
};

int main() {
  Type type(1.2);

  Append visitor;
  visitor.argument = "first value";
  boost::apply_visitor(visitor, type);

  visitor.argument = "new value";
  boost::apply_visitor(visitor, type);
}
