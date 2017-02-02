// To build:
// g++ -I /nobackup/doma/TRUNK/cme-utest/cme/externalLibs/boost/boost_1_53_0_built/ -g -o test-map test-map.cpp

#include <iostream>
#include <string>
#include <boost/variant.hpp>
#include <vector>

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


int main(void)
{
  std::vector< boost::variant<int, std::string> > vec;
  vec.push_back( 21 );
  vec.push_back( "hello " );

  times_two_generic visitor;
  std::for_each(vec.begin(),
                vec.end(), 
                boost::apply_visitor(visitor));
  

  boost::variant< int, std::string > v;
  v = "hello again ";

  std::cout << "before: " << v << std::endl;
  bool ret = boost::apply_visitor( times_two_visitor(), v );
  std::cout << "after: " << v << ", ret=" << ret << std::endl;

  v = 13;

  std::cout << "before: " << v << std::endl;
  ret = boost::apply_visitor( times_two_visitor(), v );
  std::cout << "after: " << v << ", ret=" << ret << std::endl;

  return 0;
}


/**
You can also return values from a static_visitor, like so:

class TreeVisitor : public boost::static_visitor<bool>
{
public:
  bool operator()(Tree<std::string>& tree) const
  {
    // Do something with the string tree
    return true;
  }

  bool operator()(Tree<int>& tree) const
  {
    // Do something with the int tree
    return false;
  }
};

bool result = boost::apply_visitor(TreeVisitor(), tree);

*/
