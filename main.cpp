#include <iostream>
#include <fstream>
#include <algorithm>
#include <iterator>


std::string read(std::istream &stream, std::string::size_type count)
{
    std::string out;
    out.reserve(count);
    std::copy_n(std::istreambuf_iterator<char>(stream), count, std::back_inserter(out));
    return out;
}

int main(int, const char**)
{
    std::ifstream f("/dev/ttyACM0");
    
    auto s = read(f, 100);
    std::cout << s << std::endl;

    return 0;
}