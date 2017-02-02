#include <limits>
#include <iostream>


int main(void)
{
    int64_t i64 = std::numeric_limits<int64_t>::max();

    std::cout << "int64_t max=" << i64 << std::endl;

    return 0;

}

