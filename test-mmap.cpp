//Compile like this: g++ -o mmap mmap.cpp -lboost_iostreams
#include <boost/iostreams/device/mapped_file.hpp>
#include <iostream>
#include <string>
using namespace std;
using namespace boost::iostreams;

int main(int argc, char** argv) {
    //Initialize the memory-mapped file
    mapped_file_source file(argv[1]);
    //Read the entire file into a string
    string fileContent(file.data(), file.size());
    //Print the string
    cout << fileContent;
    //Cleanup
    file.close();
}

