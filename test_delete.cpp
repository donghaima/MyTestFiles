#include <iostream>
#include <string>

using namespace std;

int main(void)
{
    string *str = new string("This is a string");
    cout << "str ptr=" << str << ", string=" << *str << "." << endl;

    cout << "now delete ptr str.." << endl;
    delete str;

    cout << "str ptr=" << str << "." << endl;

    return 0;

}
