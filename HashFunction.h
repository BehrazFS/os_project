//
// Created by behra on 1/26/2025.
//

#ifndef HASHFUNCTION_H
#define HASHFUNCTION_H


#include <string>
using namespace std;
const string SECRET = "SecretKey:)";

inline string simpleHash(const string& message) {
    const int size = 20;
    unsigned long hash = 4782;
    string input = message + SECRET;
    for (char c : input) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }

    string hashStr = to_string(hash);
    if (hashStr.size() > size) {
        return hashStr.substr(0, size);
    }
    while (hashStr.size() < size) {
        hashStr += 'P';//pad
    }

    return hashStr;
}

#endif //HASHFUNCTION_H
