//
// Created by behra on 1/25/2025.
//
#include <cstring>
#include <iostream>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <sstream>
#include "../constant_and_types.h"
#include "../ThreadSafeBuffer.h"
using namespace std;
unordered_map<int,ClientInfo> clients;
unordered_map<int,ExchangeInfo> exchanges;
ThreadSafeBuffer<string> input_buffer(1000,"bank_input_buffer",false);

[[noreturn]] void bank_reader() {
    int sock_fd;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in server_addr{}, client_addr{};

    if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Bank socket creation has failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(BANK_PORT);

    // Bind the socket
    if (bind(sock_fd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bank bind failed");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    std::cout << "Bank is listening on port " << BANK_PORT << " (UDP)\n";
    while(true) {
        socklen_t len = sizeof(client_addr);
        long long int n = recvfrom(sock_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &client_addr, &len);
        if (n < 0) {
            perror("Bank receive failed");
            continue;
        }

        buffer[n] = '\0';

        // Extract ip and port of client
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        int client_port = ntohs(client_addr.sin_port);

        string message(buffer);
        if (input_buffer.is_full()) {
            continue;
        }
        input_buffer.add_drop(message);
        // cout << client_ip << " " << client_port << " " << message << endl;


    }

    close(sock_fd);
}
void request_handler() {
    while (true) {
        if  (input_buffer.is_empty()) {
            continue;
        }
        string request = input_buffer.remove_no_wait();
        // cout << request << endl;
        istringstream iss(request);
        string data;
        iss >> data;
        if (data == "INIT_CLIENT") {
            ClientInfo client;
            iss >> data; // name
            client.name = data;
            iss >> data; // port
            client.port = stoi(data);
            clients[client.port] = client;
            cout << "Client " << client.name << " connected on port " << client.port << "\n";
        }
        else if (data == "INIT_EXCHANGE") {
            ExchangeInfo exchange;
            iss >> data; // name
            exchange.name = data;
            iss >> data; // port
            exchange.port = stoi(data);
            exchanges[exchange.port] = exchange;
            cout << "Exchange " << exchange.name << " connected on port " << exchange.port << "\n";
        }


    }
}
int main() {
    thread bank_reader_thread(bank_reader);
    thread bank_request_handler(request_handler);
    bank_reader_thread.join();
    bank_request_handler.join();
    /* stringstream s ;
    // string x;
    //
    // s <<"123 456" ;
    // istringstream ss(s.str());
    // ss >> x;
    // cout << x << endl;
    // ss >> x;
    // cout << x << endl;*/
    return 0;
}