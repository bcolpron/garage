#include <iostream>
#include <fstream>
#include <algorithm>
#include <iterator>
#include <regex>

std::string read(std::istream &stream, std::string::size_type count)
{
    std::string out;
    out.reserve(count);
    std::copy_n(std::istreambuf_iterator<char>(stream), count, std::back_inserter(out));
    return out;
}

int main(int, const char **)
{
    std::ifstream f("/dev/ttyACM0");
    f.exceptions(std::ios::failbit | std::ios::badbit | std::ios::eofbit);

    std::regex re(".*RESPONSE state=(0|1)");

    while (true)
    {
        auto s = read(f, 20);
        std::cout << "<" << s << ">" << std::endl;

        std::smatch matches;
        if (std::regex_search(s, matches, re)) {
            std::cout << matches[1] << std::endl;
        }
    }

    return 0;
}