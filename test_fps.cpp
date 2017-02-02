#include <iostream>
#include <cmath>

using namespace std;


int main (void)
{
  int detected_fps_num = 0, detected_fps_denom = 1001;

  double frame_rate_value[9] = 
    {12.5, 23.97, 24, 25, 29.97, 30, 50, 59.94, 60};

  int decimation[9] = 
    {2, 1, 1, 1, 1, 1, 1, 1, 1};

  for (int i = 0; i < 9; i++) {
    detected_fps_num = ceil(frame_rate_value[i] * decimation[i]);
    detected_fps_denom = 
      (int)((1000.0*(double)detected_fps_num/(decimation[i]*frame_rate_value[i])) + 0.5);
  
    cout << "i=" << i << 
      " frame_rate=" << frame_rate_value[i] << 
      " decimation=" << decimation[i] << 
      ": detected_fps_num=" << detected_fps_num << 
      " detected_fps_denom=" << detected_fps_denom << endl;
  }

  return 0;
}
