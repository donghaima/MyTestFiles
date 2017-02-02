#include <iostream>
#include <unistd.h>

int main (void)
{
  int j = 0;
  while (j++ <= 4) {

    for (int i=0; i < 10; i++) {
      if (i == 8) {
        std::cout << "Exit when i is 8" << std::endl;
        break;
      }
    }
    std::cout << "In while loop: j=" << j << std::endl;
    usleep(1000000);

  }

  return 0;
}
