#include "htl_utility.hpp"
using namespace std;

string srs_string_replace(string str, string old_str, string new_str)
{
    std::string ret = str;

    if (old_str == new_str) {
        return ret;
    }

    size_t pos = 0;
    while ((pos = ret.find(old_str, pos)) != std::string::npos) {
        ret = ret.replace(pos, old_str.length(), new_str);
    }

    return ret;
}
