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

using namespace std;
int main() {
    string name;
    cout << "Enter your exchange name: ";
    cin >> name;
    int sock_fd;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in server_addr{};

    // Create a UDP socket
    if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Exchange socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(BANK_PORT);
    inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr);

    // Send registration message
    sendto(sock_fd, name.c_str(), name.size(), 0, (const struct sockaddr *)&server_addr, sizeof(server_addr));

    // Receive acknowledgment
    socklen_t len = sizeof(server_addr);
    long long int n = recvfrom(sock_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&server_addr, &len);
    if (n < 0) {
        perror("Exchange receive failed");
    } else {
        buffer[n] = '\0';
        std::cout << "Bank response: " << buffer << "\n";
    }

    close(sock_fd);



    return 0;
}