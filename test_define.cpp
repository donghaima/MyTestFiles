#include <iostream>
#include <string>

// const #define vs. const string

#define DEFAULT_NAME "Cisco AnyRes/1.0"

const std::string default_name_string = "Cisco AnyRes/2.0";


int fun1(int a, std::string s = DEFAULT_NAME)
{
  std::cout << "In fun1(): s= " << s << std::endl;
  return 0;
}

int fun2(int a, std::string s = default_name_string)
{
  std::cout << "In fun2(): s= " << s << std::endl;
  return 0;
}


int main (void)
{
  std::cout << "#define: " << DEFAULT_NAME << std::endl;

  std::cout << "const string: " << default_name_string << std::endl;

  std::string str = "InletSpinnaker/1.0";

  if (str == default_name_string)
    std::cout<< "same string" << std::endl;
  else
    std::cout<< "not same string" << std::endl;

  if (str == DEFAULT_NAME)
    std::cout<< "is default name" << std::endl;
  else
    std::cout<< "not default name" << std::endl;

  fun1(2);
  fun2(4);

  return 0;
}
