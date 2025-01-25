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

int main() {
    string name;
    cout << "Enter your name: ";
    cin >> name;
    int sock_fd;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in server_addr{};

    if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("client socket creation has failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(BANK_PORT);
    inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr);

    // Send registration message
    sendto(sock_fd, name.c_str(), name.size(), 0, (const struct sockaddr *)&server_addr, sizeof(server_addr));

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