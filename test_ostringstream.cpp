#include <iostream>
#include <sstream>

int main(void)
{
    //char buffer[256];

    std::ostringstream buffer;
    std::string m_UserName = "doma";

    buffer << "User name is: " << m_UserName << "\n";
    std::cout << buffer.str();

    return 0;
}
