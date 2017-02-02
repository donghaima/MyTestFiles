#include <ostream>
#include <iostream>
#include <sstream>

class Foo
{
public:
  Foo(int x)
  { 
    m_x = x;
  }

  // std::ostream::operator<< overloading
  friend std::ostream& operator<<(std::ostream&os, const Foo& foo)
  {
    os << "From Foo: " << foo.m_x;
    return os;
  }

private:
  int m_x;

};


int main(void)
{
  Foo my_foo(10);

  std::cout << my_foo << std::endl;

  std::stringstream ss;
  ss << my_foo;
  std::cout << ss.str() << std::endl;

  return 0;
}
