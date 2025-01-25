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
#include "../constant_and_types.h"
using namespace std;
unordered_map<int,EntityInfo> clients;
unordered_map<int,EntityInfo> exchanges;

int main() {
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

        std::string message(buffer);

        // extract message -->
    }

    close(sock_fd);





    return 0;
}