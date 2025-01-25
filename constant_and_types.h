//
// Created by behra on 1/25/2025.
//
#include <iostream>
using namespace std;
#ifndef CONSTANT_AND_TYPES_H
#define CONSTANT_AND_TYPES_H
constexpr int BUFFER_SIZE = 1024;
constexpr int BANK_PORT = 9999;
constexpr string server_ip = "127.0.0.1";
struct EntityInfo {
    string name;
    string ip;
    int port;
};
#endif //CONSTANT_AND_TYPES_H
