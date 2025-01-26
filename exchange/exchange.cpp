//
// Created by behra on 1/25/2025.
//
#include <iostream>
#include <cstring>
#include <iostream>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#include "../constant_and_types.h"
#include "../ThreadSafeBuffer.h"
#include "../HashFunction.h"
using namespace std;
string name;
int assigned_port;
ThreadSafeBuffer<string> input_buffer(1000,"bank_input_buffer",false);
Safe safe;
void exchange_reader() {
    int sock_fd;
    struct sockaddr_in addr{}, client_addr{};
    // Create a UDP socket
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        cout << "Exchange socket creation failed"<<endl;
        exit(EXIT_FAILURE);
    }

    // Configure the address structure with port 0 (system assigns a port)
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(0); // OS assigns an available port

    // Bind the socket
    if (bind(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        cout << "Exchange bind failed"<<endl;
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    // Retrieve the assigned port
    socklen_t addr_len = sizeof(addr);
    if (getsockname(sock_fd, (struct sockaddr *)&addr, &addr_len) < 0) {
        cout << "Exchange retrieve port failed"<<endl;
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    assigned_port = ntohs(addr.sin_port);

    int bank_socket_fd;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in bank_server_addr{};

    // Create a UDP socket
    if ((bank_socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        cout << "Exchange bank socket creation failed"<<endl;
        exit(EXIT_FAILURE);
    }

    // Configure server address
    memset(&bank_server_addr, 0, sizeof(bank_server_addr));
    bank_server_addr.sin_family = AF_INET;
    bank_server_addr.sin_port = htons(BANK_PORT);
    inet_pton(AF_INET, server_ip.c_str(), &bank_server_addr.sin_addr);


    // Send registration message
    string message = "INIT_EXCHANGE " + name + " " + to_string(assigned_port);
    message = simpleHash(message) + " " + message;
    sendto(sock_fd, message.c_str(), message.size(), 0, (const struct sockaddr *)&bank_server_addr, sizeof(bank_server_addr));
    cout << "Exchange is listening on assigned port: " << assigned_port << endl;
    while(true) {
        socklen_t len = sizeof(client_addr);
        long long int n = recvfrom(sock_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &client_addr, &len);
        if (n < 0) {
            cout << "EXchange receive failed"<<endl;
            continue;
        }

        buffer[n] = '\0';

        // Extract ip and port of client
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        int client_port = ntohs(client_addr.sin_port);

        string message2(buffer);
        if (input_buffer.is_full()) {
            continue;
        }
        input_buffer.add_drop(message2);
        // cout << client_ip << " " << client_port << " " << message << endl;


    }

    close(sock_fd);
}
void request_handler() {

}
void new_crypto_handler() {
    while (true) {
        cout << "Enter new crypto name: ";
        cin >> name;

    }
}
int main() {

    cout << "Enter your exchange name: ";
    cin >> name;
    thread exchange_reader_thread(exchange_reader);
    thread exchange_request_handler(request_handler);
    thread new_crypto_thread(new_crypto_handler);
    exchange_reader_thread.join();
    exchange_request_handler.join();
    exchange_request_handler.join();
    /*
    // Receive acknowledgment
    // socklen_t len = sizeof(bank_server_addr);
    // long long int n = recvfrom(sock_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&bank_server_addr, &len);
    // if (n < 0) {
    //     cout << "Exchange receive failed"<<endl;
    // } else {
    //     buffer[n] = '\0';
    //     cout << "Bank response: " << buffer << "\n";
    // }
    //
    // close(sock_fd);
    */



    return 0;
}