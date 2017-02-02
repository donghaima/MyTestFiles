#include <string>
#include <iostream>

using namespace std;

int main(void)
{
    // Generate long filler string
    std::string padding = "0123456789abcdefghijklmnopqrstuv";
    int iterations = 10;
    for (int i = 0; i <= iterations; i++) {
      padding += padding;
      //cout << "i = " << i << ", size = " << padding.size() << ", padding: " << padding << endl;
      cout << "i = " << i << ", size = " << padding.size() << endl;
    }

    return 0;
}
