#include <iostream>

using namespace std;

class Test
{
public:
    Test()
    {
        cout << "In constructor Test():" << endl;
    };

    ~Test()
    {
        cout << "In destructor Test():" << endl;
    };
    
private:
};



int main (void)
{
    cout << "--> Test t1" << endl;
    Test t1;
    
    cout << "--> Test* tp = &t1" << endl;
    Test* tp = &t1;
        
    cout << "--> Test& t2 = *tp" << endl;
    Test& t2 = *tp;

    cout << "--> return 0;" << endl;
    return 0;
}

