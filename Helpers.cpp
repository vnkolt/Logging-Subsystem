//
// Helpers.cpp
//
// helper functions
//
#include <random>
#include <cassert>
#include <fstream>
#include <string>
#include <chrono>

#include "Helpers.h"

/*

 Function: randoms

 Description: generates a vector with random integer values from specified range

 Parameters:
      count - count of numbers to be generated (vector size)
      from - minimum of the range
      to - maximum of the range
 
 Returns: vector of integers with random numbers from a specified range

*/
std::vector<int> randoms(int count, int from, int to) {
    assert(count > 0 && (to - from) > count);
    std::vector<int> result(count, 0);
    std::vector<int> rinds;

    for (auto i = from; i <= to; ++i) {
        rinds.push_back(i);
    }

    auto now = std::chrono::high_resolution_clock::now();
    std::mt19937 rgen{std::random_device{}()};
    rgen.seed((unsigned int)now.time_since_epoch().count());

    for (auto i = 0; i < count; ++i) {
        std::uniform_int_distribution<int> distr(0, (int)(rinds.size() - 1));

        int idx = distr(rgen);
        result[i] = rinds[idx];
        rinds.erase(rinds.begin() + idx);
    }

    return result;
}

// Function: replaceAll
//
// Description : generates a vector with random integer values from specified range
//
// Parameters :
// 
// s - reference to string to be modified
// search - search pattern
// replace - string to replace
//
void replaceAll(std::string& s, const std::string& search, const std::string& replace) {
    for (size_t pos = 0; ; pos += replace.length()) {
        pos = s.find(search, pos);
        if (std::string::npos == pos) {
            break;
        }
        s.erase(pos, search.length());
        s.insert(pos, replace);
    }
}
