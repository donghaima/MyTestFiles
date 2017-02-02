// To build with boost library:
//   g++ -I /nobackup/doma/TRUNK/cme-utest/cme/externalLibs/boost/boost_1_53_0_built/ -g -o test-map test-map.cpp


// string::empty
#include <stdlib.h>
#include <iostream>
#include <string>
#include <sstream>
#include <boost/lexical_cast.hpp>


using namespace std;

int main ()
{
  stringstream ss;
  string m_streamname = "stream1";
  string language = "eng";
  string streamName = "CC1";
  uint8_t streamNum = 2;
  int streamNumInt = 2;

  ss << m_streamname << "_caption_CC" << int(streamNum) << "_" << language;
  string captionStreamName = ss.str();

  // XXX DEBUG
  cout << "XXXConfigureSubtitles: streamNum=" << streamNum << 
    ", caption streamName=" << captionStreamName << endl;
  
  string line = "test line";
  cout << "The string is: " << line 
       << "; empty() check: " << line.empty() << endl;

  cout << "Now clear the string" << endl;
  line.clear();

  cout << "The string now is: " << line 
       << "; empty() check: " << line.empty() << endl;

  char char_array[] = "string array";
  line = "string object: ";
  int i = 1500000;
  char bi[16];
  snprintf(bi, sizeof(bi), "%d", i);   
  cout << "Test concatenation: " << line + std::string(char_array) + std::string(bi) << "." << endl;

  char char_str[100] = "This is a test string";
  cout << "char_str: strlen=" << strlen(char_str) << ", str[]=" << char_str << endl;

  char_str[0] = '\0';
  cout << "After reset, char_str: strlen=" << strlen(char_str) << ", str[]=" << char_str << endl;

  char typeStr[4], type[]="ec-3";
  strncpy(typeStr, type, 4);
  cout << "typeStr=" << typeStr << ", typeStr len=" << strlen(typeStr) << ", type[]=" << type << endl;
  return 0;
}
