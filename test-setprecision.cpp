#include <iostream>
#include <sstream>
#include <iomanip>
#include <cmath>

using namespace std;

bool AlmostEquals(double a, double b, double epsilon)
{
    return a==b || std::abs(a-b) < epsilon;
}
int main(void)
{
    double f = 3.14159;

    std::cout << std::setprecision(4) << f << std::endl;
    std::cout << std::setprecision(9) << f << std::endl;
    std::cout << std::fixed;
    std::cout << std::setprecision(4) << f << std::endl;
    std::cout << std::setprecision(9) << f << std::endl;

    double t;
    stringstream convert("26026.7804777");
    convert.precision(3);
    convert >> std::fixed >> t;
    std::cout << "t=" << t << std::endl;

    double a = 26026.7804777;
    double b = 26026.7808777;
    cout << "Compare a= " << a << " and b=" << b << ": " << AlmostEquals(a,b,0.001) << endl;

    return 0;
}
