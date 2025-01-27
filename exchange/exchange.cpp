//
// Created by behra on 1/25/2025.
//
#include <iostream>
#include <cstring>
#include <iostream>
#include <sstream>
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
ThreadSafeBuffer<string> input_buffer(10000, "bank_input_buffer", false);
Safe safe;
constexpr double init_price = 100.0;

void exchange_reader() {
    int sock_fd;
    struct sockaddr_in addr{}, client_addr{};
    // Create a UDP socket
    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        cout << "Exchange socket creation failed" << endl;
        exit(EXIT_FAILURE);
    }

    // Configure the address structure with port 0 (system assigns a port)
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(0); // OS assigns an available port

    // Bind the socket
    if (bind(sock_fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        cout << "Exchange bind failed" << endl;
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    // Retrieve the assigned port
    socklen_t addr_len = sizeof(addr);
    if (getsockname(sock_fd, (struct sockaddr *) &addr, &addr_len) < 0) {
        cout << "Exchange retrieve port failed" << endl;
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    assigned_port = ntohs(addr.sin_port);

    int bank_socket_fd;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in bank_server_addr{};

    // Create a UDP socket
    if ((bank_socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        cout << "Exchange bank socket creation failed" << endl;
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
    sendto(sock_fd, message.c_str(), message.size(), 0, (const struct sockaddr *) &bank_server_addr,
           sizeof(bank_server_addr));
    cout << "Exchange is listening on assigned port: " << assigned_port << endl;
    while (true) {
        socklen_t len = sizeof(client_addr);
        long long int n = recvfrom(sock_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &client_addr, &len);
        if (n < 0) {
            cout << "EXchange receive failed" << endl;
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
    while (true) {
        if (input_buffer.is_empty()) {
            continue;
        }
        string request = input_buffer.remove_no_wait();
        cout << request << endl;
        istringstream iss(request);
        string data;
        iss >> data;
        if (data != simpleHash(iss.str().substr(21))) {
            cout << "Authorization failed session must be terminated" << endl;
            continue;
        }
        iss >> data;
        if (data == "GET_PRICE_CLIENT") {
            iss >> data; // port
            int client_port = stoi(data);
            string cyripto_name;
            iss >> cyripto_name;
            if (!safe.cryptocurrencies.contains(cyripto_name)) {
            }
            struct sockaddr_in client_server_addr{};
            int client_sock_fd;
            if ((client_sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                cout << "Exchange client socket creation failed" << endl;
                exit(EXIT_FAILURE);
            }
            memset(&client_server_addr, 0, sizeof(client_server_addr));
            client_server_addr.sin_family = AF_INET;
            client_server_addr.sin_port = htons(client_port);
            inet_pton(AF_INET, server_ip.c_str(), &client_server_addr.sin_addr);

            cout << "A client viewed price of " + cyripto_name + " on port " << client_port << endl;
            string message = "GET_CRYPTO_PRICE " + cyripto_name + " : " + to_string(
                                 safe.cryptocurrencies[cyripto_name].price);
            sendto(client_sock_fd, message.c_str(), message.size(), 0, (const struct sockaddr *) &client_server_addr,
                   sizeof(client_server_addr));
            close(client_sock_fd);
        } else if (data == "ADDED_NEW_CRYPTO") {
            iss >> data; // Failed/Sucsess
            if (data == "Failed") {
                cout << "exchange failed to create crypto" << endl;
            } else {
                string crypto_name;
                iss >> crypto_name;
                SafeItem si;
                si.creation_time = chrono::system_clock::now();
                si.count = 100;
                si.init_count = 100;
                si.cryptocurrency = crypto_name;
                si.price = init_price;
                si.state = "preorder";
                safe.cryptocurrencies[crypto_name] = si;
                cout << "exchange created crypto : " + crypto_name << endl;
            }
        } else if (data == "BUY_CRYPTO_CLIENT") {
            iss >> data; // port
            int client_port = stoi(data);
            string cyripto_name;
            iss >> cyripto_name;
            int amount;
            iss >> data;
            amount = stoi(data);
            string message = "BUY_CRYPTO_CLIENT " + cyripto_name + " ";
            bool send = true;
            if (!safe.cryptocurrencies.contains(cyripto_name)) {
                cout << "exchange failed to find crypto" << endl;
                message += "Not Found";
            } else {
                SafeItem si = safe.cryptocurrencies[cyripto_name];
                // cout<<si.count<<endl;
                if (((si.creation_time + chrono::seconds(30) <= chrono::system_clock::now()) && si.state == "preorder")
                    || (si.state == "preorder" && si.price >= 900)) {
                    safe.cryptocurrencies[cyripto_name].state = "released";
                    struct sockaddr_in bank_server_addr{};
                    int bank_sock_fd;
                    if ((bank_sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                        cout << "Exchange bank socket creation failed" << endl;
                        exit(EXIT_FAILURE);
                    }
                    memset(&bank_server_addr, 0, sizeof(bank_server_addr));
                    bank_server_addr.sin_family = AF_INET;
                    bank_server_addr.sin_port = htons(BANK_PORT);
                    inet_pton(AF_INET, server_ip.c_str(), &bank_server_addr.sin_addr);
                    string message2 = "RELEASE_CRYPTO " + cyripto_name + " " + to_string(si.price) + " " + to_string(
                                          si.init_count);
                    message2 = simpleHash(message2) + " " + message2;
                    sendto(bank_sock_fd, message2.c_str(), message2.size(), 0,
                           (const struct sockaddr *) &bank_server_addr, sizeof(bank_server_addr));
                    close(bank_sock_fd);
                    cout << "exchange sent to bank for release" << endl;

                    message += "Not released please try later";
                    cout << "exchange failed due to pending release crypto" << endl;
                } else {
                    if (amount > si.count && si.state == "released") {
                        message += "Not enough available";
                        struct sockaddr_in bank_server_addr{};
                        int bank_sock_fd;
                        if ((bank_sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                            cout << "Exchange bank socket creation failed" << endl;
                            exit(EXIT_FAILURE);
                        }
                        memset(&bank_server_addr, 0, sizeof(bank_server_addr));
                        bank_server_addr.sin_family = AF_INET;
                        bank_server_addr.sin_port = htons(BANK_PORT);
                        inet_pton(AF_INET, server_ip.c_str(), &bank_server_addr.sin_addr);
                        string message2 = "BUY_CRYPTO_EXCHANGE_TO_BANK " + cyripto_name + " " +
                                          to_string(assigned_port);
                        message2 = simpleHash(message2) + " " + message2;
                        sendto(bank_sock_fd, message2.c_str(), message2.size(), 0,
                               (const struct sockaddr *) &bank_server_addr, sizeof(bank_server_addr));
                        close(bank_sock_fd);
                        safe.cryptocurrencies[cyripto_name].buying = true;
                        cout << "exchange sent to bank for buying more" << endl;
                        cout << "exchange failed due to low crypto inventory(start buying)" << endl;
                    } else if (amount > si.count && si.state == "preorder") {
                        message += "Not enough available";
                        cout << "exchange failed due to low crypto inventory(error)" << endl;
                    } else {
                        send = false;
                        struct sockaddr_in bank_server_addr{};
                        int bank_sock_fd;
                        if ((bank_sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                            cout << "Exchange bank socket creation failed" << endl;
                            exit(EXIT_FAILURE);
                        }
                        memset(&bank_server_addr, 0, sizeof(bank_server_addr));
                        bank_server_addr.sin_family = AF_INET;
                        bank_server_addr.sin_port = htons(BANK_PORT);
                        inet_pton(AF_INET, server_ip.c_str(), &bank_server_addr.sin_addr);
                        string message2 = "BUY_CRYPTO_CLIENT_CHECK " + cyripto_name + " " + to_string(amount) + " " +
                                          to_string(si.price) + " " + to_string(client_port) + " " + to_string(
                                              assigned_port);
                        message2 = simpleHash(message2) + " " + message2;
                        sendto(bank_sock_fd, message2.c_str(), message2.size(), 0,
                               (const struct sockaddr *) &bank_server_addr, sizeof(bank_server_addr));
                        close(bank_sock_fd);
                        cout << "exchange sent to bank for check(buy)" << endl;
                    }
                }
            }
            if (send) {
                struct sockaddr_in client_server_addr{};
                int client_sock_fd;
                if ((client_sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                    cout << "Exchange client socket creation failed" << endl;
                    exit(EXIT_FAILURE);
                }
                memset(&client_server_addr, 0, sizeof(client_server_addr));
                client_server_addr.sin_family = AF_INET;
                client_server_addr.sin_port = htons(client_port);
                inet_pton(AF_INET, server_ip.c_str(), &client_server_addr.sin_addr);


                sendto(client_sock_fd, message.c_str(), message.size(), 0,
                       (const struct sockaddr *) &client_server_addr, sizeof(client_server_addr));
                close(client_sock_fd);
            }
        } else if (data == "BUY_CRYPTO_CLIENT_CHECKED") {
            string message = "BUY_CRYPTO_CLIENT_CHECKED ";
            string crypto_name;
            string check;
            double money;
            int amount;
            int client_port;
            iss >> data; // Failed/Sucsess
            check = data;
            message += (check + " ");
            iss >> data;
            crypto_name = data;
            iss >> data;
            money = stod(data);
            iss >> data;
            amount = stoi(data);
            iss >> data;
            client_port = stoi(data);
            if (check == "Failed") {
                cout << "exchange: client failed to buy crypto" << endl;
            } else {
                safe.cryptocurrencies[crypto_name].count -= amount;
                safe.balance += money;
                message += "Bought " + crypto_name + " : " + to_string(amount);
                //handel price change
                if (safe.cryptocurrencies[crypto_name].state == "preorder") {
                    safe.cryptocurrencies[crypto_name].count += 100;
                    safe.cryptocurrencies[crypto_name].init_count += 100;
                    safe.cryptocurrencies[crypto_name].price += 10.0 * amount;
                } else {
                    if (0.05 * safe.cryptocurrencies[crypto_name].init_count <= amount) {
                        safe.cryptocurrencies[crypto_name].price *= 1.03;
                    }
                }
                cout << "exchange: client bought  crypto " + crypto_name + " : " + to_string(amount) << endl;
            }
            struct sockaddr_in client_server_addr{};
            int client_sock_fd;
            if ((client_sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                cout << "Exchange client socket creation failed" << endl;
                exit(EXIT_FAILURE);
            }
            memset(&client_server_addr, 0, sizeof(client_server_addr));
            client_server_addr.sin_family = AF_INET;
            client_server_addr.sin_port = htons(client_port);
            inet_pton(AF_INET, server_ip.c_str(), &client_server_addr.sin_addr);

            sendto(client_sock_fd, message.c_str(), message.size(), 0, (const struct sockaddr *) &client_server_addr,
                   sizeof(client_server_addr));
            close(client_sock_fd);
            cout << "exchange sent to client(bought)" << endl;
        } else if (data == "DO_RELEASE_CRYPTO") {
            iss >> data;
            string crypto_name;
            crypto_name = data;
            double price;
            iss >> data;
            price = stod(data);
            int init_count;
            iss >> data;
            init_count = stoi(data);
            if (!safe.cryptocurrencies.contains(crypto_name)) {
                SafeItem si;
                si.creation_time = chrono::system_clock::now();
                si.count = 0;
                si.init_count = init_count;
                si.cryptocurrency = crypto_name;
                si.price = price;
                si.state = "released";
                safe.cryptocurrencies[crypto_name] = si;
            }
        } else if (data == "SELL_CRYPTO_CLIENT") {
            iss >> data; // port
            int client_port = stoi(data);
            string cyripto_name;
            iss >> cyripto_name;
            int amount;
            iss >> data;
            amount = stoi(data);
            string message = "SELL_CRYPTO_CLIENT " + cyripto_name + " ";
            bool send = true;
            if (!safe.cryptocurrencies.contains(cyripto_name) || safe.cryptocurrencies[cyripto_name].state ==
                "preorder") {
                cout << "exchange failed to find crypto" << endl;
                message += "Not Found";
            } else {
                SafeItem si = safe.cryptocurrencies[cyripto_name];
                if (si.price * amount > safe.balance) {
                    cout << "exchange not enough money to buy crypto" << endl;
                    message += "Not Bought";
                } else {
                    send = false;
                    struct sockaddr_in bank_server_addr{};
                    int bank_sock_fd;
                    if ((bank_sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                        cout << "Exchange bank socket creation failed" << endl;
                        exit(EXIT_FAILURE);
                    }
                    memset(&bank_server_addr, 0, sizeof(bank_server_addr));
                    bank_server_addr.sin_family = AF_INET;
                    bank_server_addr.sin_port = htons(BANK_PORT);
                    inet_pton(AF_INET, server_ip.c_str(), &bank_server_addr.sin_addr);
                    string message2 = "SELL_CRYPTO_CLIENT_CHECK " + cyripto_name + " " + to_string(amount) + " " +
                                      to_string(si.price) + " " + to_string(client_port) + " " + to_string(
                                          assigned_port);
                    message2 = simpleHash(message2) + " " + message2;
                    sendto(bank_sock_fd, message2.c_str(), message2.size(), 0,
                           (const struct sockaddr *) &bank_server_addr, sizeof(bank_server_addr));
                    close(bank_sock_fd);
                    cout << "exchange sent to bank for check(sell)" << endl;
                }
            }
            if (send) {
                struct sockaddr_in client_server_addr{};
                int client_sock_fd;
                if ((client_sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                    cout << "Exchange client socket creation failed" << endl;
                    exit(EXIT_FAILURE);
                }
                memset(&client_server_addr, 0, sizeof(client_server_addr));
                client_server_addr.sin_family = AF_INET;
                client_server_addr.sin_port = htons(client_port);
                inet_pton(AF_INET, server_ip.c_str(), &client_server_addr.sin_addr);


                sendto(client_sock_fd, message.c_str(), message.size(), 0,
                       (const struct sockaddr *) &client_server_addr, sizeof(client_server_addr));
                close(client_sock_fd);
            }
        } else if (data == "SELL_CRYPTO_CLIENT_CHECKED") {
            string message = "SELL_CRYPTO_CLIENT_CHECKED ";
            string crypto_name;
            string check;
            double money;
            int amount;
            int client_port;
            iss >> data; // Failed/Sucsess
            check = data;
            message += (check + " ");
            iss >> data;
            crypto_name = data;
            iss >> data;
            money = stod(data);
            iss >> data;
            amount = stoi(data);
            iss >> data;
            client_port = stoi(data);
            if (check == "Failed") {
                cout << "exchange: client failed to sell crypto" << endl;
            } else {
                //handel price change
                if (0.03 * safe.cryptocurrencies[crypto_name].count <= amount) {
                    safe.cryptocurrencies[crypto_name].price *= 0.98;
                }
                safe.cryptocurrencies[crypto_name].count += amount;
                safe.balance -= money;
                message += "Sold " + crypto_name + " : " + to_string(amount);
                cout << "exchange: client sold  crypto " + crypto_name + " : " + to_string(amount) << endl;
            }
            struct sockaddr_in client_server_addr{};
            int client_sock_fd;
            if ((client_sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                cout << "Exchange client socket creation failed" << endl;
                exit(EXIT_FAILURE);
            }
            memset(&client_server_addr, 0, sizeof(client_server_addr));
            client_server_addr.sin_family = AF_INET;
            client_server_addr.sin_port = htons(client_port);
            inet_pton(AF_INET, server_ip.c_str(), &client_server_addr.sin_addr);

            sendto(client_sock_fd, message.c_str(), message.size(), 0, (const struct sockaddr *) &client_server_addr,
                   sizeof(client_server_addr));
            close(client_sock_fd);
            cout << "exchange sent to client(sold)" << endl;
        } else if (data == "SYNC") {
            string crypto_name;
            iss >> data;
            crypto_name = data;
            double price;
            iss >> data;
            price = stod(data);
            safe.cryptocurrencies[crypto_name].price = price;
            cout << "Exchange crypto : " + crypto_name + " synced : " + to_string(price) << endl;
        } else if (data == "BUY_CRYPTO_EXCHANGE_TO_BANK_RESPONSE") {
            string crypto_name;
            iss >> data;
            crypto_name = data;
            int size;
            iss >> data;
            size = stoi(data);
            double max_price = safe.balance * 0.1;
            for (int i = 0; i < size; i++) {
                int port;
                iss >> data;
                port = stoi(data);
                //sent to all exchanges
                struct sockaddr_in exchange_server_addr{};
                int exchange_sock_fd;
                if ((exchange_sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                    cout << "Exchange exchange socket creation failed" << endl;
                    exit(EXIT_FAILURE);
                }
                memset(&exchange_server_addr, 0, sizeof(exchange_server_addr));
                exchange_server_addr.sin_family = AF_INET;
                exchange_server_addr.sin_port = htons(port);
                inet_pton(AF_INET, server_ip.c_str(), &exchange_server_addr.sin_addr);
                string message = "EXCHANGE_BUY_REQUEST " + crypto_name + " " + to_string(max_price) + " " + to_string(
                                     assigned_port);
                message = simpleHash(message) + " " + message;
                sendto(exchange_sock_fd, message.c_str(), message.size(), 0,
                       (const struct sockaddr *) &exchange_server_addr, sizeof(exchange_server_addr));
                close(exchange_sock_fd);
                cout << "exchange sent to exchanges a buy request on " + crypto_name<<endl;
            }
        } else if (data == "EXCHANGE_BUY_REQUEST") {
            string crypto_name;
            iss >> data;
            crypto_name = data;
            double max_price;
            iss >> data;
            max_price = stod(data);
            int exchange_port;
            iss >> data;
            exchange_port = stoi(data);
            int amount = 0;
            while (safe.cryptocurrencies[crypto_name].count > amount && safe.cryptocurrencies[crypto_name].price *
                   amount < max_price) {
                amount++;
            }
            if (amount > 0) {
                struct sockaddr_in exchange_server_addr{};
                int exchange_sock_fd;
                if ((exchange_sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                    cout << "Exchange exchange socket creation failed" << endl;
                    exit(EXIT_FAILURE);
                }
                memset(&exchange_server_addr, 0, sizeof(exchange_server_addr));
                exchange_server_addr.sin_family = AF_INET;
                exchange_server_addr.sin_port = htons(exchange_port);
                inet_pton(AF_INET, server_ip.c_str(), &exchange_server_addr.sin_addr);
                string message = "EXCHANGE_BUY_RESPONSE " + crypto_name + " " + to_string(
                                     safe.cryptocurrencies[crypto_name].price * amount) + " " + to_string(amount) + " "
                                 + to_string(assigned_port);
                message = simpleHash(message) + " " + message;
                sendto(exchange_sock_fd, message.c_str(), message.size(), 0,
                       (const struct sockaddr *) &exchange_server_addr, sizeof(exchange_server_addr));
                close(exchange_sock_fd);
                cout << "exchange sent to exchanges a buy response on " + crypto_name<<endl;
            }
        } else if (data == "EXCHANGE_BUY_RESPONSE") {
            string crypto_name;
            iss >> data;
            crypto_name = data;
            double money;
            iss >> data;
            money = stod(data);
            int amount;
            iss >> data;
            amount = stoi(data);
            int exchange_port;
            iss >> data;
            exchange_port = stoi(data);
            if (safe.cryptocurrencies[crypto_name].buying) {
                safe.cryptocurrencies[crypto_name].buying = false;
                if (0.05 * safe.cryptocurrencies[crypto_name].init_count <= amount) {
                    safe.cryptocurrencies[crypto_name].price *= 1.03;
                }
                safe.balance -= money;
                safe.cryptocurrencies[crypto_name].count += amount;
                struct sockaddr_in exchange_server_addr{};
                int exchange_sock_fd;
                if ((exchange_sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                    cout << "Exchange exchange socket creation failed" << endl;
                    exit(EXIT_FAILURE);
                }
                memset(&exchange_server_addr, 0, sizeof(exchange_server_addr));
                exchange_server_addr.sin_family = AF_INET;
                exchange_server_addr.sin_port = htons(exchange_port);
                inet_pton(AF_INET, server_ip.c_str(), &exchange_server_addr.sin_addr);
                string message = "EXCHANGE_BOUGHT_RESPONSE " + crypto_name + " " + to_string(
                                     safe.cryptocurrencies[crypto_name].price * amount) + " " + to_string(amount) + " "
                                 + to_string(assigned_port);
                message = simpleHash(message) + " " + message;
                sendto(exchange_sock_fd, message.c_str(), message.size(), 0,
                       (const struct sockaddr *) &exchange_server_addr, sizeof(exchange_server_addr));
                close(exchange_sock_fd);
                cout << "exchange sent to exchanges a bought response on " + crypto_name<<endl;
            }
        } else if (data == "EXCHANGE_BOUGHT_RESPONSE") {
            string crypto_name;
            iss >> data;
            crypto_name = data;
            double money;
            iss >> data;
            money = stod(data);
            int amount;
            iss >> data;
            amount = stoi(data);
            int exchange_port;
            iss >> data;
            exchange_port = stoi(data);
            if (0.03 * safe.cryptocurrencies[crypto_name].count <= amount) {
                safe.cryptocurrencies[crypto_name].price *= 0.98;
            }
            safe.balance += money;
            safe.cryptocurrencies[crypto_name].count -= amount;
            cout << "exchange sent to exchanges a transaction complited on " + crypto_name<<endl;
        }
    }
}

void new_crypto_handler() {
    while (true) {
        // so you can do it on one pc (time delay)
        this_thread::sleep_for(chrono::seconds(2));
        cout << "Enter new crypto name: \n";
        cin >> name;
        struct sockaddr_in bank_server_addr{};
        int bank_sock_fd;
        if ((bank_sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
            cout << "Client exchange socket creation failed" << endl;
            exit(EXIT_FAILURE);
        }
        memset(&bank_server_addr, 0, sizeof(bank_server_addr));
        bank_server_addr.sin_family = AF_INET;
        bank_server_addr.sin_port = htons(BANK_PORT);
        inet_pton(AF_INET, server_ip.c_str(), &bank_server_addr.sin_addr);
        string message = "ADD_NEW_CRYPTO " + name + " " + to_string(assigned_port);
        message = simpleHash(message) + " " + message;
        sendto(bank_sock_fd, message.c_str(), message.size(), 0, (const struct sockaddr *) &bank_server_addr,
               sizeof(bank_server_addr));
        close(bank_sock_fd);
        this_thread::sleep_for(chrono::seconds(2));
    }
}

int main() {
    cout << "Enter your exchange name: \n";
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
