#include <wchar.h>
#include <cstring>
#include <iostream>

using namespace std;

wchar_t* A2Wconversion( const string& str, wchar_t* wbuf, size_t bufsize)
{
  memset(wbuf, 0, bufsize);
  size_t size = str.size();
  if (size >= bufsize)
    size = bufsize-1;

  printf("In A2Wconversion(): size=%d\n", size);

  for( size_t i=0 ; i<size ; ++i ) {
    wbuf[i] = str.c_str()[i];
    printf("In A2Wconversion(): i=%d, wbuf[%d]=%d, str[%d]=%d\n", 
           i, i, wbuf[i], i, str[i]);
  }
  return wbuf;
}

wchar_t *myA2Wconv(const string& str)
{
  std::wstring stemp = std::wstring(str.begin(), str.end());
  return  (wchar_t *)stemp.c_str();
}


int main(int argc, char* argv[])
{
  const string label="Espanol";

  const wstring wlabel=L"Eùùeùmplmkhùm";

  wchar_t wsAudioLabel[256];

  A2Wconversion(label, wsAudioLabel, 256);

  std::cout << "label string=" << label << std::endl;
  std::wcout << "label wstring=" << wlabel << std::endl;

  //std::cout << "After conversion: wsAudioLabel=" << wsAudioLabel << std::endl;
  printf("After conversion: wsAudioLabel=%ls\n", wsAudioLabel);

  printf("After my conversion: %ls\n", myA2Wconv(label));

  printf("Printf wstring: %ls\n", wlabel.c_str());
  wsprintf(wsAudioLabel, wlabel.c_str(), sizeof(wsAudioLabel));
  printf("Printf wsAudioLabel: %ls\n", wsAudioLabel);

  return 0;
}
