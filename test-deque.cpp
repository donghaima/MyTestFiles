/**
 *
 * test-deque
 *
 */

#include <iostream>
#include <deque>
#include <stdint.h>

using namespace std;


typedef bool  BOOL;
typedef int64_t  INT64;
typedef unsigned long DWORD;

struct tDashMediaFileInfo
{
    INT64 timestamp;
    INT64 duration;
    INT64 timescale;
    std::string fileName;
    std::string subDir;
    DWORD asIndex;
  BOOL removedInMPD;

    tDashMediaFileInfo() :
      timestamp(0),
      duration(0),
      timescale(0),
      asIndex(0),
      removedInMPD(false)
    {
    }
};


deque<string> slist;

void checkAndRemove(string name)
{
  deque<string>::const_iterator itr;
  itr = slist.begin();

  while (itr != slist.end())
  {  
    if (*itr == name)
    {
      cout << "Found " << name << " on the string list" << endl;
    }
    itr++;
  }
}


void dumpList()
{
  const std::deque<string>& d = slist;
  cout << "String List: ";
  for( deque<string>::const_iterator iter = d.begin(); iter < d.end(); ++iter ) {
    cout << *iter << " ";
  }
  cout << endl;
}

// std::remove algorithm returns an iterator to the beginning of the range 
// of "unused" elements. It does not change the end() iterator of 
// the container nor does the size of it. Member erase function can be used 
// to really eliminate the members from the container in the following 
// idiomatic way:
//    std::vector<int> v; 
//    // fill it up somehow
//    v.erase(std::remove(v.begin(), v.end(), 99), v.end());

// Delete all the elements matching string s on the list
void deleteFromList(const string& s)
{
  std::deque<string>& d = slist;
  // really remove all elements with value string s
  d.erase(std::remove(d.begin(), d.end(), s), d.end());
}

int main()
{
  slist.push_back("david");
  slist.push_back("jason");
  slist.push_back("david");
  slist.push_back("amy");
  slist.push_back("david");
  slist.push_back("eric");
  slist.push_back("david");
  slist.push_back("david");
  slist.push_back("andrew");

  dumpList();
  
  cout << "Search for david..." << endl;
  string s = "david";
  checkAndRemove(s);

  cout << "Remove david from the list..." << endl;
  deleteFromList(s);

  dumpList();
  

  // 
  std::deque<tDashMediaFileInfo> hist;

  tDashMediaFileInfo segment;
  segment.fileName = "stream1-1.mp4";
  segment.timestamp = 12121212112;
  segment.duration = 20020000;
  segment.asIndex = 11;
  segment.timescale = 10000000;
  segment.removedInMPD = false;
  hist.push_back(segment);

  segment.fileName = "stream1-2.mp4";
  segment.timestamp = 24121212112;
  segment.duration = 20020000;
  segment.asIndex = 12;
  segment.timescale = 10000000;
  segment.removedInMPD = false;
  hist.push_back(segment);

  segment.fileName = "stream1-3.mp4";
  segment.timestamp = 36121212112;
  segment.duration = 20020000;
  segment.asIndex = 13;
  segment.timescale = 10000000;
  segment.removedInMPD = false;
  hist.push_back(segment);

  for(deque<tDashMediaFileInfo>::const_iterator iter = hist.begin();
      iter < hist.end();
      ++iter ) {
    tDashMediaFileInfo mfile = *iter;
    cout << "fileName=" << mfile.fileName << ", " <<
      "timestamp=" << mfile.timestamp << ", " <<
      "asIndex=" << mfile.asIndex << ", " <<
      "removedInMPD flag=" << mfile.removedInMPD << endl;
  }
  cout << endl;

  for(deque<tDashMediaFileInfo>::iterator iter = hist.begin();
      iter < hist.end();
      ++iter ) {
    tDashMediaFileInfo mfile = *iter;
    //mfile.removedInMPD = true;
    iter->removedInMPD = true;
  }

  for(deque<tDashMediaFileInfo>::const_iterator iter = hist.begin();
      iter < hist.end();
      ++iter ) {
    tDashMediaFileInfo mfile = *iter;
    cout << "fileName=" << mfile.fileName << ", " <<
      "timestamp=" << mfile.timestamp << ", " <<
      "asIndex=" << mfile.asIndex << ", " <<
      "removedInMPD flag=" << mfile.removedInMPD << endl;
  }
  cout << endl;

  class Test {
  public:
    std::deque <int> samples;
  };

  Test *testObj = new Test();

  testObj->samples.push_back(32323);
  testObj->samples.push_back(32323);
  testObj->samples.push_back(32323);
  testObj->samples.push_back(32323);

  cout << "After push_back(), deque samples size=" << 
    testObj->samples.size() << endl;

  int siz = testObj->samples.size();
  for (int i=0; i<siz; i++) {
    testObj->samples.pop_front();
  }

  cout << "After pop_front(), deque samples size=" << 
    testObj->samples.size() << endl;

  cout << "Now trying to clean deque samples" << endl;

  testObj->samples.clear();
  cout << "deque samples size =" << testObj->samples.size() << endl;

  delete testObj;

  cout << "Clearing deque hist again..." << endl;
  testObj->samples.clear();

  return 0;
}
