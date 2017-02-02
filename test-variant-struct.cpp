// To build:
// g++ -I /nobackup/doma/TRUNK/cme-utest/cme/externalLibs/boost/boost_1_53_0_built/ -g -o test-map test-map.cpp

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include <boost/variant.hpp>
#include <boost/foreach.hpp>

class times_two_generic
  : public boost::static_visitor<>
{
public:

  template <typename T>
  void operator()( T & operand ) const
  {
    std::cout << "before: " << operand << std::endl;
    operand += operand;
    std::cout << "after: " << operand << std::endl;

  }

};


class times_two_visitor
  : public boost::static_visitor<bool>  // return bool
{
public:

  bool operator()(int & i) const
  {
    i *= 2;
    return false;
  }

  bool operator()(std::string & str) const
  {
    str += str;
    return true;
  }

};


// Test variant in a struct, which in turn in a map
class registryValue {
public:
  registryValue(int v, bool in, std::string r = "")
  {
    value = v;
    inRegistry = in;
    altRootPath = r;
  }

  registryValue(std::string v, bool in, std::string r = "")
  {
    value = v;
    inRegistry = in;
    altRootPath = r;
  }
public:
  boost::variant<int, std::string> value;
  bool inRegistry;   // true if in registry
  std::string altRootPath;   // set if different than the default root path
};


typedef std::map<std::string, registryValue> RegistryMap;
typedef std::pair<std::string, registryValue> RegistryParameter;


boost::variant<int, std::string> queryRegistry(std::string key)
{
  if (key == "key1" || key == "key4")
    return 1234;
  else
    return "TestString";

}

bool queryRegistry(std::string key, registryValue& val)
{

  if (val.value.type() == typeid(std::string)) {
    std::cout << "Value for key " << key << " is of type std::string" << std::endl;
  }
  else if (val.value.type() == typeid(int)) {
    std::cout << "Value for key " << key << " is of type int" << std::endl;
  }

  return false;
}

int main(void)
{
  bool ret;

  // 
  RegistryMap rm;
  registryValue v1(100, true);
  registryValue v2("Cisco AnyRes", false, "Software\\Inlet\\Spinnaker");

  rm.insert(RegistryParameter("key1", v1));
  rm.insert(RegistryParameter("key2", v2));
  rm.insert(RegistryParameter("key3", v2));

  v1 = registryValue("100", true);
  rm.insert(RegistryParameter("key4", v1));


  BOOST_FOREACH( RegistryMap::value_type &v, rm ) {
    registryValue rv = v.second;

    std::cout << "before key=" << v.first << 
      "; value=" << rv.value << ", " << rv.inRegistry << ", " << rv.altRootPath << std::endl;

    //boost::variant<int, std::string> val = queryRegistry(v.first);

    ret = queryRegistry(v.first, rv);

    // 
    //ret = boost::apply_visitor( times_two_visitor(), rv.value );

    std::cout << "after key=" << v.first << 
      "; value=" << rv.value << ", " << rv.inRegistry << ", " << rv.altRootPath << std::endl;

  }

  return 0;
}

