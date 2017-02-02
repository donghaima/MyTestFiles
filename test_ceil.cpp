#include <iostream>
#include <cmath>

using namespace std;


int main (void)
{

  uint32_t segNumA, segNumC, fragNum, fragsPerSeg=5;
  
  for (fragNum=1; fragNum < 120; fragNum++) {
    segNumA = static_cast<uint32_t>(std::ceil(double(fragNum)/fragsPerSeg));
    segNumC = fragNum/fragsPerSeg + 1;

    cout << "fragNum=" << fragNum << ": segNumA=" << segNumA 
         << ", segNumC=" << segNumC << endl;
  }

  return 0;
}
