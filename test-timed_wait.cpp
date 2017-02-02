// To build:
//
// g++ -I /nobackup/doma/SVN/bugfix/cme/build/include/boost-1_36 -g -o test-boost-thread test-boost-thread.cpp /nobackup/doma/SVN/bugfix/cme/build/lib/libboost_thread.so

// To run, need to export LD_LIBRARY_PATH first

#include <boost/thread.hpp>
#include <iostream>
#include <boost/date_time/posix_time/posix_time.hpp>

int main(){
  // Determine the absolute time for this timer.
  boost::system_time tAbsoluteTime = 
    boost::get_system_time() + boost::posix_time::milliseconds(10000);
  
  bool done;
  boost::mutex m;
  boost::condition_variable cond;
  
  boost::unique_lock<boost::mutex> lk(m);
  while(!done) {
    boost::system_time tStartTime = boost::get_system_time();
    if(! cond.timed_wait(lk,tAbsoluteTime)) {
      done = true;
      std::cout << "Timed out after " << 
        (boost::get_system_time() - tStartTime) << " seconds." << std::endl;
    }
  }
  return 1;
}

