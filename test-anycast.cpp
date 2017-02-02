// To build:
// g++ -I /nobackup/doma/TRUNK/cme-utest/cme/externalLibs/boost/boost_1_53_0_built/ -g -o test-map test-map.cpp

#include <iostream>
using std::cout;
using std::endl;

#include <map>
#include <boost/any.hpp>

using boost::any_cast;
typedef std::map<std::string, boost::any> t_map;


int main(int argc, char **argv)
{

  t_map map;
  char *pc = "boo yeah!";

  map["a"] = 2.1;
  map["b"] = pc;

  cout << "map contents" << endl;
  cout << any_cast<double>(map["a"]) << endl;
  cout << any_cast<char*>(map["b"]) << endl;

  return 0;
}
