//
// Created by behra on 1/25/2025.
//
#include <iostream>
#include <unordered_map>
using namespace std;
#ifndef CONSTANT_AND_TYPES_H
#define CONSTANT_AND_TYPES_H
constexpr int BUFFER_SIZE = 4096;
constexpr int BANK_PORT = 9999;
constexpr string server_ip = "127.0.0.1";
struct Wallet {
    double balance = 0;
    unordered_map<string, int> cryptocurrencies;
    string pars_string() {
        string ret = "";
        ret += to_string(balance) + " ";
        for (auto &it : cryptocurrencies) {
            ret += it.first + " " + to_string(it.second) + " ";
        }
        return ret;
    }
};

struct SafeItem {
    int count = 0;
    int init_count = 0;
    string cryptocurrency;
    double price = 0;
    string state = "preorder";
    chrono::time_point<chrono::system_clock> creation_time;;
};
struct Safe {
    double balance = 0;
    unordered_map<string, SafeItem> cryptocurrencies;
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
