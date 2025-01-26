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
#include "../HashFunction.h"
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
int assigned_port;
string name;

void get_cryptocurrency_price(int c_socket_fd ,int sock_fd, sockaddr_in sock_in) {
    string cryptocurrency_name;
    cout << "Enter cryptocurrency name: ";
    cin >> cryptocurrency_name;

    string message = "GET_PRICE_CLIENT " + to_string(assigned_port) + " " + cryptocurrency_name;
    message = simpleHash(message) + " " + message;
    sendto(sock_fd, message.c_str(), message.size(), 0, (const struct sockaddr *)&sock_in, sizeof(sock_in));
    socklen_t len = sizeof(sock_in);
    char buffer[BUFFER_SIZE];
    long long int n = recvfrom(c_socket_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&sock_in, &len);
    if (n < 0) {
        cout << (message + " Receive failed")<<endl;
    } else {
        buffer[n] = '\0';
        cout << "Exchange response: " << buffer << "\n";
    }
}
void get_exchanges_list(int c_socket_fd ,int sock_fd, sockaddr_in sock_in) {
    string message = "VIEW_EXCHANGE_LIST_CLIENT " + to_string(assigned_port);
    message = simpleHash(message) + " " + message;
    sendto(sock_fd, message.c_str(), message.size(), 0, (const struct sockaddr *)&sock_in, sizeof(sock_in));
    socklen_t len = sizeof(sock_in);
    char buffer[BUFFER_SIZE];
    long long int n = recvfrom(c_socket_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&sock_in, &len);
    if (n < 0) {
        cout << (message + " Receive failed")<<endl;
    } else {
        buffer[n] = '\0';
        cout << "Bank response: " << buffer << "\n";
    }
}
void buy_cryptocurrency(int c_socket_fd ,int sock_fd, sockaddr_in sock_in) {
    string cryptocurrency_name;
    cout << "Enter cryptocurrency name: ";
    cin >> cryptocurrency_name;
    int amount;
    cout << "Enter amount : ";
    cin >> amount;
    string message = "BUY_CRYPTO_CLIENT " + to_string(assigned_port) + " " + cryptocurrency_name + " " + to_string(amount);
    message = simpleHash(message) + " " + message;
    sendto(sock_fd, message.c_str(), message.size(), 0, (const struct sockaddr *)&sock_in, sizeof(sock_in));
    socklen_t len = sizeof(sock_in);
    char buffer[BUFFER_SIZE];
    long long int n = recvfrom(c_socket_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&sock_in, &len);
    if (n < 0) {
        cout << (message + " Receive failed")<<endl;
    } else {
        buffer[n] = '\0';
        cout << "Exchange response: " << buffer << "\n";
    }
}
void sell_cryptocurrency(int sock_fd, sockaddr_in sock_in) {

}
void view_wallet_balance(int c_socket_fd ,int sock_fd, sockaddr_in sock_in) {
    string message = "VIEW_BALANCE_CLIENT " + to_string(assigned_port);
    message = simpleHash(message) + " " + message;
    sendto(sock_fd, message.c_str(), message.size(), 0, (const struct sockaddr *)&sock_in, sizeof(sock_in));
    socklen_t len = sizeof(sock_in);
    char buffer[BUFFER_SIZE];
    long long int n = recvfrom(c_socket_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&sock_in, &len);
    if (n < 0) {
        cout << (message + " Receive failed")<<endl;
    } else {
        buffer[n] = '\0';
        cout << "Bank response: " << buffer << "\n";
    }
}
void increase_wallet_balance(int c_socket_fd ,int sock_fd, struct sockaddr_in sock_in) {
    int amount;
    cout << "Enter increase wallet balance amount:"<<endl;
    cin >> amount;
    string message = "INCREASE_BALANCE_CLIENT " + to_string(assigned_port) +" "+to_string(amount);
    message = simpleHash(message) + " " + message;
    sendto(sock_fd, message.c_str(), message.size(), 0, (const struct sockaddr *)&sock_in, sizeof(sock_in));
    socklen_t len = sizeof(sock_in);
    char buffer[BUFFER_SIZE];
    long long int n = recvfrom(c_socket_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&sock_in, &len);
    if (n < 0) {
        cout << (message + " Receive failed")<<endl;
    } else {
        buffer[n] = '\0';
        cout << "Bank response: " << buffer << "\n";
    }
}
void view_transaction_history(int c_socket_fd,int sock_fd, sockaddr_in sock_in) {
    int page;
    cout << "Enter transaction history page:"<<endl;
    cin >> page;
    string message = "VIEW_HISTORY_CLIENT " + to_string(assigned_port) +" "+to_string(page);
    message = simpleHash(message) + " " + message;
    sendto(sock_fd, message.c_str(), message.size(), 0, (const struct sockaddr *)&sock_in, sizeof(sock_in));
    socklen_t len = sizeof(sock_in);
    char buffer[BUFFER_SIZE];
    long long int n = recvfrom(c_socket_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&sock_in, &len);
    if (n < 0) {
        cout << (message + " Receive failed")<<endl;
    } else {
        buffer[n] = '\0';
        cout << "Bank response: " << buffer << "\n";
    }
}

int main() {
    cout << "Enter your name: ";
    cin >> name;
    int sock_fd;
    struct sockaddr_in addr{};
    // Create a UDP socket
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        cout << "Client socket creation failed"<<endl;
        exit(EXIT_FAILURE);
    }

    // Configure the address structure with port 0 (system assigns a port)
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(0); // OS assigns an available port

    // Bind the socket
    if (bind(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        cout << "Client bind failed"<<endl;
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    // Retrieve the assigned port
    socklen_t addr_len = sizeof(addr);
    if (getsockname(sock_fd, (struct sockaddr *)&addr, &addr_len) < 0) {
        cout << "Client retrieve port failed"<<endl;
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    assigned_port = ntohs(addr.sin_port);

    int bank_socket_fd;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in bank_server_addr{};

    // Create a UDP socket
    if ((bank_socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        cout << "Client bank socket creation failed"<<endl;
        exit(EXIT_FAILURE);
    }

    // Configure server address
    memset(&bank_server_addr, 0, sizeof(bank_server_addr));
    bank_server_addr.sin_family = AF_INET;
    bank_server_addr.sin_port = htons(BANK_PORT);
    inet_pton(AF_INET, server_ip.c_str(), &bank_server_addr.sin_addr);


    // Send registration message
    string message = "INIT_CLIENT " + name + " " + to_string(assigned_port);
    message = simpleHash(message) + " " + message;
    sendto(sock_fd, message.c_str(), message.size(), 0, (const struct sockaddr *)&bank_server_addr, sizeof(bank_server_addr));
    cout << "Client is listening on assigned port: " << assigned_port << std::endl;
    bool Exited = false;
    string choice;
    while (!Exited) {
        cout <<"\n\n\n";
        for (const auto & i : menu) {
            cout << i << endl;
        };
        cin>>choice;
        if (choice == "0") {
            int exchange_port;
            cout << "Enter exchange port number: ";
            cin >> exchange_port;
            struct sockaddr_in exchange_server_addr{};
            int exchange_sock_fd;
            if ((exchange_sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                cout << "Client exchange socket creation failed"<<endl;
                exit(EXIT_FAILURE);
            }
            memset(&exchange_server_addr, 0, sizeof(exchange_server_addr));
            exchange_server_addr.sin_family = AF_INET;
            exchange_server_addr.sin_port = htons(exchange_port);
            inet_pton(AF_INET, server_ip.c_str(), &exchange_server_addr.sin_addr);
            get_cryptocurrency_price(sock_fd,exchange_sock_fd, exchange_server_addr);
            close(exchange_sock_fd);
        } else if (choice == "1") {
            get_exchanges_list(sock_fd,bank_socket_fd, bank_server_addr);
        } else if (choice == "2") {
            int exchange_port;
            cout << "Enter exchange port number: ";
            cin >> exchange_port;
            struct sockaddr_in exchange_server_addr{};
            int exchange_sock_fd;
            if ((exchange_sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                cout << "Client exchange socket creation failed"<<endl;
                exit(EXIT_FAILURE);
            }
            memset(&exchange_server_addr, 0, sizeof(exchange_server_addr));
            exchange_server_addr.sin_family = AF_INET;
            exchange_server_addr.sin_port = htons(exchange_port);
            inet_pton(AF_INET, server_ip.c_str(), &exchange_server_addr.sin_addr);
            buy_cryptocurrency(sock_fd,exchange_sock_fd, exchange_server_addr);
            close(exchange_sock_fd);
        } else if (choice == "3") {
            // int exchange_port;
            // cout << "Enter exchange port number: ";
            // cin >> exchange_port;
            // struct sockaddr_in exchange_server_addr{};
            // int exchange_sock_fd;
            // if ((exchange_sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            //     cout << "Client exchange socket creation failed"<<endl;
            //     exit(EXIT_FAILURE);
            // }
            // memset(&exchange_server_addr, 0, sizeof(exchange_server_addr));
            // exchange_server_addr.sin_family = AF_INET;
            // exchange_server_addr.sin_port = htons(exchange_port);
            // inet_pton(AF_INET, server_ip.c_str(), &exchange_server_addr.sin_addr);
            // sell_cryptocurrency(exchange_sock_fd, exchange_server_addr);
            // close(exchange_sock_fd);
        } else if (choice == "4") {
            view_wallet_balance(sock_fd,bank_socket_fd, bank_server_addr);
        } else if (choice == "5") {
            increase_wallet_balance(sock_fd,bank_socket_fd, bank_server_addr);
        } else if (choice == "6") {
            view_transaction_history(sock_fd,bank_socket_fd, bank_server_addr);
        } else if (choice == "7") {
            Exited = true;
        } else {
            cout << "Invalid choice!\n";
        }
    }
    close(sock_fd);
    close(bank_socket_fd);
    return 0;
}