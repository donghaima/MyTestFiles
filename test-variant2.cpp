// To build:
// g++ -I /nobackup/doma/TRUNK/cme-utest/cme/externalLibs/boost/boost_1_53_0_built/ -g -o test-map test-map.cpp

#include <iostream>
#include <string>
#include <boost/variant.hpp>
#include <vector>
#include <map>
#include <iomanip>




class VariantPrinter : boost::static_visitor<void>
{
    void operator()(int int_val)
    {
        std::cout << int_val << std::endl;
    }
    void operator()(double double_val)
    {
      std::cout << std::setprecision(15) << double_val << std::endl;
    }
    // etc.
};

void PrintVariant(const boost::variant<int, double>& the_variant)
{
    boost::apply_visitor(VariantPrinter(), the_variant);
}

int main()
{
  std::map<std::string, boost::variant<int, double> > mapValues;

    mapValues["int"] = 10;
    PrintVariant(mapValues["int"]);

    mapValues["double"] = 10.123;
    PrintVariant(mapValues["double"]);

}
