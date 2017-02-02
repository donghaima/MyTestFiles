/**
 *
 * vectest.cpp
 *
 */

#include <iostream>
#include <vector>

using namespace std;

int main()
{
  vector<int> vec1;
  //vector<int> vec2;

  vec1.push_back(1);
  vec1.push_back(2);
  vec1.push_back(3);
  vec1.push_back(4);
  vec1.push_back(5);
  vec1.push_back(6);
  vec1.push_back(7);
  vec1.push_back(5);
  vec1.push_back(4);
  vec1.push_back(5);


  //vec2.reserve( vec1.size() );
  //copy(vec1.begin(), vec1.end(), vec1.begin());

  //vec2 = vec1;
  vector<int> vec2(vec1);

  cout << "vec1.size()     = " << vec1.size() << endl;
  cout << "vec1.capacity() = " << vec1.capacity() << endl;
  
  cout << "vec1: ";
  for( vector<int>::const_iterator iter = vec1.begin(); iter < vec1.end(); ++iter ) {
    cout << *iter << " ";
  }
  cout << endl;
  
  cout << "vec2.size()     = " << vec2.size() << endl;
  cout << "vec2.capacity() = " << vec2.capacity() << endl;
  cout << "vec2: ";

  for( vector<int>::const_iterator iter = vec2.begin(); iter < vec2.end(); ++iter ) {
    cout << *iter << " ";
  }
  
  cout << endl;


  return 0;
}
