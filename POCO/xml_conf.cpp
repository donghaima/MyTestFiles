/**
 * $ g++ -o xml_conf xml_conf.cc -lPocoUtil
 * $ ./xml_conf
 * prop1 + prop2 = 3.57
 * This is an empty string: 
 * prop4 + prop4_0 + prop4_1 = 4
 * Hello, world!
 * Hello, world!
 *
 */


#include <Poco/Util/AbstractConfiguration.h>
#include <Poco/Util/XMLConfiguration.h>
#include <iostream>

using namespace Poco::Util;
using namespace std;

int main()
{
  AbstractConfiguration *cfg = new XMLConfiguration("conf.xml");

  double prop1 = cfg->getDouble("prop1");
  double prop2 = cfg->getDouble("prop2");
  cout << "prop1 + prop2 = " << prop1 + prop2 << endl;

  cout << "This is an empty string: " 
       << cfg->getString("prop3.prop4") << endl;

  int prop4   = cfg->getInt("prop3.prop4[@attr]");
  int prop4_0 = cfg->getInt("prop3.prop4[0][@attr]");
  int prop4_1 = cfg->getInt("prop3.prop4[1][@attr]");
  cout << "prop4 + prop4_0 + prop4_1 = " 
       << prop4 + prop4_0 + prop4_1 << endl;

  cout << cfg->getString("prop5[0]") 
       << cfg->getString("prop5[1]") << endl;
  cout << cfg->getString("prop5[@id=first]") 
       << cfg->getString("prop5[@id='second'") << endl;

  // No need to delete cfg, since it is reference counted for garbage collection

  return 0;
}
