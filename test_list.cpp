// Create a random integer list and sort the list
#include <iostream>
#include <list>
#include <vector>
#include <string>

using namespace std;

int main()
{
  list<string> lst;
  int i;
  
  //create a list of random integers
  lst.push_back("string1");
  lst.push_back("string2");
  lst.push_back("string3");
  lst.push_back("string4");
  lst.push_back("string5");
  lst.push_back("string6");
  lst.push_back("string7");
  lst.push_back("string8");
  
  cout << "Original list: " << endl;

  list<string>::iterator p = lst.begin();
  while(p != lst.end()){
    cout << *p << " ";
    p++;
  }
  
  cout << endl << endl;
  
  // vector
  vector<string> vecString;
  vecString.push_back("string1");
  vecString.push_back("string2");
  vecString.push_back("string3");

  return (0);
}

