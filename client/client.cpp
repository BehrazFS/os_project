//
// Created by behra on 1/25/2025.
//
#include <cstring>
#include <iostream>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#include "../constant_and_types.h"
using namespace std;
string menu[] = {
    "Get cryptocurrency price [0]" ,
    "Get exchanges list [1]" ,
    "Buy cryptocurrency [2]" ,
    "Sell cryptocurrency [3]",
    "View wallet balance [4]",
    "Requesting a balance increase [5]" ,
    "View transaction history [6]" ,
    "Exit [7]",
};

// void handleClient(int sock_fd, struct sockaddr_in sockaddr_in) {
//     bool Exited = false;
//     string choice;
//     while (!Exited) {
//         for (const auto & i : menu) {
//             cout << i << endl;
//         };
//         getline(cin, choice);
//         if (choice == "0") {
//             getcryptocurrencyprice(sock_fd, sockaddr_in);
//         } else if (choice == "1") {
//             getExchangesList(sock_fd, sockaddr_in);
//         } else if (choice == "2") {
//             buyCryptocurrency(sock_fd, sockaddr_in);
//         } else if (choice == "3") {
//             sellCryptocurrency(sock_fd, sockaddr_in);
//         } else if (choice == "4") {
//             viewWalletBallance(sock_fd, sockaddr_in);
//         } else if (choice == "5") {
//             increaseWalletBallance(sock_fd, sockaddr_in);
//         } else if (choice == "6") {
//             viewTransactionHistory(sock_fd, sockaddr_in);
//         } else if (choice == "7") {
//             Exited = true;
//         } else {
//             cout << "Invalid choice!\n";
//         }
//     }
// }
int main() {
    string name;
    cout << "Enter your name: ";
    cin >> name;
    int sock_fd;
    struct sockaddr_in addr{};
    // Create a UDP socket
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        perror("Client socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure the address structure with port 0 (system assigns a port)
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(0); // OS assigns an available port

    // Bind the socket
    if (bind(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("Client bind failed");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    // Retrieve the assigned port
    socklen_t addr_len = sizeof(addr);
    if (getsockname(sock_fd, (struct sockaddr *)&addr, &addr_len) < 0) {
        perror("Client retrieve port failed");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    int assigned_port = ntohs(addr.sin_port);

    int bank_socket_fd;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in bank_server_addr{};

    // Create a UDP socket
    if ((bank_socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Client bank socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    memset(&bank_server_addr, 0, sizeof(bank_server_addr));
    bank_server_addr.sin_family = AF_INET;
    bank_server_addr.sin_port = htons(BANK_PORT);
    inet_pton(AF_INET, server_ip.c_str(), &bank_server_addr.sin_addr);


    // Send registration message
    string message = "INIT_CLIENT " + name + " " + to_string(assigned_port);
    sendto(sock_fd, message.c_str(), message.size(), 0, (const struct sockaddr *)&bank_server_addr, sizeof(bank_server_addr));
    //cout << "Listening on assigned port: " << assigned_port << std::endl;
//     bool exit = false;
//     while (isRunning) {
//
//     }
//     // Receive acknowledgment
//     // socklen_t len = sizeof(server_addr);
//     // int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, &len);
//     // if (n < 0) {
//     //     perror("Receive failed");
//     // } else {
//     //     buffer[n] = '\0';
//     //     std::cout << "Bank response: " << buffer << "\n";
//     // }
//     //
//     // close(sockfd);
// }




    return 0;
}