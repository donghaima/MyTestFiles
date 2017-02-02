// To build:
//
// g++ -I /nobackup/doma/SVN/bugfix/cme/build/include/boost-1_36 -g -o test-boost-thread test-boost-thread.cpp /nobackup/doma/SVN/bugfix/cme/build/lib/libboost_thread.so

// To run, need to export LD_LIBRARY_PATH first


#include <boost/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <iostream>
#include <string>

using namespace std;

// The function that will be run by the thread
void ThreadFunction(const string& name)
{
  std::cout << "   - Thread " << name << ": ";
  std::cout << boost::get_system_time() << 
    ": Thread will sleep for 15 seconds." << std::endl;

  // Never ending loop. Normally the thread will never finish
  while(true)
  {
    try
    {
      // Interrupt can only occur in wait/sleep or join operation.
      // If you don't do that, call interuption_point().
      // Remove this line, and the thread will never be interrupted.
      boost::this_thread::interruption_point();
 
      // Sleep for 15s
      boost::this_thread::sleep(boost::posix_time::seconds(15));
 
    }
    catch(const boost::thread_interrupted&)
    {
      // Thread interruption request received, break the loop
      std::cout << "   - Thread " << name << ": ";
      std::cout << boost::get_system_time() << 
        ": Thread interrupted. Exiting thread." << std::endl;
      break;
    }
  }

  std::cout << "   - Thread " << name << ": ";
  std::cout << boost::get_system_time() << 
    ": Broken from the thread loop." << std::endl;
}


int main()
{
  std::cout << boost::get_system_time() << 
    ": Creating two threads t and t2."<< std::endl;

  // Create and start the thread
  boost::thread t(ThreadFunction, "t");
  boost::thread *t2 = new boost::thread(ThreadFunction, "t2");

  // Wait 2 seconds for the thread to finish
  std::cout << boost::get_system_time() << 
    ": Wait for 2 seconds for the thread to stop."<< std::endl;

  while (t.timed_join(boost::posix_time::seconds(2))==false)
  {
    // Interupt the thread
    std::cout << boost::get_system_time() << 
      ": Thread not stopped, interrupt it now." << std::endl;

    t.interrupt();

    std::cout << boost::get_system_time() << 
      ": Thread interrupt request sent. " << std::endl;
    std::cout << boost::get_system_time() << 
      ": Wait up to 2 seconds for the thrad to finish.."<< std::endl;
  }
  
  // The thread t has been stopped
  std::cout << boost::get_system_time() << 
    ": Thread t stopped" << std::endl;

  // Interrupt Thread t2 and wait for up to 2 seconds for it to quit
  std::cout << std::endl;
  std::cout << boost::get_system_time() << 
    ": Interrupting Thread t2 now." << std::endl;
  t2->interrupt();  

  if (t2->timed_join(boost::posix_time::seconds(2))==false)
  {
    std::cout << boost::get_system_time() << 
      ": Thread t2 not stopped after 2 seconds."<< std::endl;
  }
  else
  {
    std::cout << boost::get_system_time() << 
      ": Thread t2 stopped."<< std::endl;
  }

  return 0;
}

 
#if 0  // output
doma@bxb-ads-095:/ws/doma-bxb/test]$ g++ -I /nobackup/doma/SVN/bugfix/cme/build/include/boost-1_36 -g -o test-thread_interrupt test-thread_interrupt.cpp /nobackup/doma/SVN/bugfix/cme/build/lib/libboost_thread.so
[doma@bxb-ads-095:/ws/doma-bxb/test]$ ./test-thread_interrupt
2012-Sep-26 10:50:52.659261: Creating two threads t and t2.
2012-Sep-26 10:50:52.659561: Wait for 2 seconds for the thread to stop.
   - Thread t: 2012-Sep-26 10:50:52.659630: Thread will sleep for 15 seconds.
   - Thread t2: 2012-Sep-26 10:50:52.659673: Thread will sleep for 15 seconds.
2012-Sep-26 10:50:54.677057: Thread not stopped, interrupt it now.
2012-Sep-26 10:50:54.677174: Thread interrupt request sent.
2012-Sep-26 10:50:54.677226: Wait up to 2 seconds for the thrad to finish..
   - Thread t: 2012-Sep-26 10:50:54.677474: Thread interrupted. Exiting thread.
   - Thread t: 2012-Sep-26 10:50:54.677523: Broken from the infinite while loop.
2012-Sep-26 10:50:54.677579: Thread t stopped

2012-Sep-26 10:50:54.677612: Interrupting Thread t2 now.
   - Thread t2: 2012-Sep-26 10:50:54.677654: Thread interrupted. Exiting thread.
   - Thread t2: 2012-Sep-26 10:50:54.677686: Broken from the infinite while loop.
2012-Sep-26 10:50:54.677717: Thread t2 stopped.
#endif
