// To build:
// g++ -I /nobackup/doma/TRUNK/cme-utest/cme/externalLibs/boost/boost_1_53_0_built/ -g -o test-map test-map.cpp

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>

using namespace std;
using namespace boost;
using namespace boost::assign;

int main() {
  typedef map<string, vector<int> > collection;
  collection m;

  m["Group 1"] = list_of(1)(2)(3);
  m["Group 2"] = list_of(10)(11)(12);

  collection::iterator g2 = m.find("Group 2");

  if (g2 != m.end()) {
    BOOST_FOREACH(int& i, g2->second) {
      cout << i << "\n";
    }
  }
}
