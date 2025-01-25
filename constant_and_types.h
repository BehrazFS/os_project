//
// Created by behra on 1/25/2025.
//
#include <iostream>
#include <unordered_map>
using namespace std;
#ifndef CONSTANT_AND_TYPES_H
#define CONSTANT_AND_TYPES_H
constexpr int BUFFER_SIZE = 1024;
constexpr int BANK_PORT = 9999;
constexpr string server_ip = "127.0.0.1";
struct Wallet {
    long long int balance = 0;
    unordered_map<string, int> cryptocurrencies;
};
struct ClientInfo {
    string name;
    int port;
    Wallet wallet;
};
struct ExchangeInfo {
    string name;
    int port;
};
#endif //CONSTANT_AND_TYPES_H
