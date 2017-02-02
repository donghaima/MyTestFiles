#include <iostream>
#include <math.h>

using namespace std;

int main (void)
{
  //double framerate = 27.97700023;
  //double framerate = 14.985;
  double framerate = 27.97;

    std::cout << "framerate = " << framerate << ", rounded = " << 
        floorf(framerate * 100 + 0.5) / 100 << std::endl;

    std::cout << "framerate = " << framerate << ", rounded = " << 
        floorf(framerate * 1000 + 0.5) / 1000 << std::endl;

    char number[24]; // dummy size, you should take care of the size!
    sprintf(number, "%.2f", framerate);
    std::cout << "sprintf: " << number << std::endl;

    sprintf(number, "%.3f", framerate);
    std::cout << "sprintf: " << number << std::endl;
    return 0;
}
