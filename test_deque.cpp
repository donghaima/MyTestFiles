// deque::pop_front
#include <iostream>
#include <deque>
using namespace std;

int main ()
{
  deque<int> mydeque;
  int sum (0);
  mydeque.push_back (100);
  mydeque.push_back (200);
  mydeque.push_back (300);

  cout << "Popping out the elements in mydeque:";
  while (!mydeque.empty())
    {
      cout << " " << mydeque.front();
      mydeque.pop_front();
      cout << "\nCurrent size of mydeque is " << int(mydeque.size()) << endl;
    }
  
  cout << "\nFinal size of mydeque is " << int(mydeque.size()) << endl;
  
  return 0;
}
