// To build:
//
// g++ -I /nobackup/doma/SVN/bugfix/cme/build/include/boost-1_36 -g -o test-boost-thread test-boost-thread.cpp /nobackup/doma/SVN/bugfix/cme/build/lib/libboost_thread.so

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <iostream>

namespace
{
  void print( int x, int y )
  {
    std::cout << x << ", " << y << std::endl;
    std::cout << x << ", " << y << std::endl;
    std::cout << x << ", " << y << std::endl;
    std::cout << x << ", " << y << std::endl;
  }
}

int main()
{
  boost::thread_group Tg;
  
  Tg.add_thread(new boost::thread(print, 1, 10)); 

//Tg.create_thread( boost::bind( print, 1, 10 ) );
  Tg.create_thread( boost::bind( print, 2, 20 ) );
  Tg.create_thread( boost::bind( print, 3, 31 ) );
  Tg.create_thread( boost::bind( print, 4, 42 ) );

  Tg.join_all();
  //std::cout << "Done! =)" << std::endl;
  //std::cin.get();
  return 0;
}
