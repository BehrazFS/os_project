//
// Created by behra on 1/25/2025.
//
#include <cstring>
#include <iostream>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <set>
#include <unordered_map>
#include <vector>
#include <sstream>
#include "../constant_and_types.h"
#include "../ThreadSafeBuffer.h"
#include "../HashFunction.h"
using namespace std;
unordered_map<int,ClientInfo> clients;
unordered_map<int,ExchangeInfo> exchanges;
ThreadSafeBuffer<string> input_buffer(10000,"bank_input_buffer",false);
constexpr int max_increase_amount = 1000;
constexpr int page_size = 10;
auto timer = chrono::system_clock::now();
vector<string> history;
unordered_map<string,SafeItem> cryptocurrencies;
[[noreturn]] void bank_reader() {
    int sock_fd;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in server_addr{}, client_addr{};

    if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        cout << "Bank socket creation has failed"<<endl;
        exit(EXIT_FAILURE);
    }

    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(BANK_PORT);

    // Bind the socket
    if (bind(sock_fd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        cout << "Bank bind failed"<<endl;
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    std::cout << "Bank is listening on port " << BANK_PORT << " (UDP)\n";
    while(true) {
        socklen_t len = sizeof(client_addr);
        long long int n = recvfrom(sock_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &client_addr, &len);
        if (n < 0) {
            cout << "Bank receive failed"<<endl;
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
        if (chrono::system_clock::now() - timer >= chrono::seconds(120)) {
            unordered_map<string,int> count;
            unordered_map<string,double> new_price;
            for (auto & it : clients) {
                for (auto & it2 : it.second.wallet.cryptocurrencies) {
                    count[it2.first]+=it2.second;
                }
            }
            for (auto & it : cryptocurrencies) {
                if (it.second.state == "released") {
                    new_price[it.first] = it.second.price + ((double)count[it.first]/it.second.init_count) * 100;
                }
            }
            for (auto & it : cryptocurrencies) {
                if (it.second.state == "released") {
                    string message = "SYNC " + it.first + " " + to_string(new_price[it.first]);
                    for ( auto & exchange1 : exchanges) {
                        struct sockaddr_in exchange_server_addr{};
                        int exchange_sock_fd;
                        if ((exchange_sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                            cout << "Bank exchange socket creation failed"<<endl;
                            exit(EXIT_FAILURE);
                        }
                        memset(&exchange_server_addr, 0, sizeof(exchange_server_addr));
                        exchange_server_addr.sin_family = AF_INET;
                        exchange_server_addr.sin_port = htons(exchange1.first);
                        inet_pton(AF_INET, server_ip.c_str(), &exchange_server_addr.sin_addr);
                        message = simpleHash(message) + " " + message;
                        sendto(exchange_sock_fd, message.c_str(), message.size(), 0, (const struct sockaddr *)&exchange_server_addr, sizeof(exchange_server_addr));
                        close(exchange_sock_fd);
                    }
                }
            }
            timer = chrono::system_clock::now();
        }
        if  (input_buffer.is_empty()) {
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
            for (auto &crypto_name : cryptocurrencies) {
            string message = "DO_RELEASE_CRYPTO " + crypto_name.first +" "+ to_string(crypto_name.second.price) + " "  +to_string(crypto_name.second.init_count);
                if (crypto_name.second.state == "released") {
                    for ( auto & exchange1 : exchanges) {
                        struct sockaddr_in exchange_server_addr{};
                        int exchange_sock_fd;
                        if ((exchange_sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                            cout << "Bank exchange socket creation failed"<<endl;
                            exit(EXIT_FAILURE);
                        }
                        memset(&exchange_server_addr, 0, sizeof(exchange_server_addr));
                        exchange_server_addr.sin_family = AF_INET;
                        exchange_server_addr.sin_port = htons(exchange1.first);
                        inet_pton(AF_INET, server_ip.c_str(), &exchange_server_addr.sin_addr);
                        message = simpleHash(message) + " " + message;
                        sendto(exchange_sock_fd, message.c_str(), message.size(), 0, (const struct sockaddr *)&exchange_server_addr, sizeof(exchange_server_addr));
                        close(exchange_sock_fd);
                    }
                }
            }
        }
        else if (data == "VIEW_BALANCE_CLIENT") {
            iss >> data; // port
            int client_port = stoi(data);
            ClientInfo client = clients[client_port];
            struct sockaddr_in client_server_addr{};
            int client_sock_fd;
            if ((client_sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                cout << "Bank client socket creation failed"<<endl;
                exit(EXIT_FAILURE);
            }
            memset(&client_server_addr, 0, sizeof(client_server_addr));
            client_server_addr.sin_family = AF_INET;
            client_server_addr.sin_port = htons(client_port);
            inet_pton(AF_INET, server_ip.c_str(), &client_server_addr.sin_addr);

            cout << "Client " << client.name << " viewed balance on port " << client_port << endl;
            string message = "BALANCE_CLIENT " + client.wallet.pars_string();
            sendto(client_sock_fd, message.c_str(), message.size(), 0, (const struct sockaddr *)&client_server_addr, sizeof(client_server_addr));
            close(client_sock_fd);
        }
        else if (data == "INCREASE_BALANCE_CLIENT") {
            iss >> data; // port
            int client_port = stoi(data);
            iss >> data;
            int amount = stoi(data);
            string message;
            if (amount <= max_increase_amount) {
                clients[client_port].wallet.balance += amount;
                ClientInfo client = clients[client_port];

                cout << "Client " << client.name << " increased balance on port " << to_string(client_port) +" amount : " + to_string(amount)<< endl;
                message = "INCREASE_BALANCE_CLIENT new balance : " + to_string(client.wallet.balance);
                history.push_back(message);
            }
            else {
                message = "INCREASE_BALANCE_CLIENT failed";
            }
            struct sockaddr_in client_server_addr{};
            int client_sock_fd;
            if ((client_sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                cout << "Bank client socket creation failed"<<endl;
                exit(EXIT_FAILURE);
            }
            memset(&client_server_addr, 0, sizeof(client_server_addr));
            client_server_addr.sin_family = AF_INET;
            client_server_addr.sin_port = htons(client_port);
            inet_pton(AF_INET, server_ip.c_str(), &client_server_addr.sin_addr);

            sendto(client_sock_fd, message.c_str(), message.size(), 0, (const struct sockaddr *)&client_server_addr, sizeof(client_server_addr));
            close(client_sock_fd);
        }
        else if (data == "VIEW_EXCHANGE_LIST_CLIENT") {
            iss >> data; // port
            int client_port = stoi(data);
            ClientInfo client = clients[client_port];
            struct sockaddr_in client_server_addr{};
            int client_sock_fd;
            if ((client_sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                cout << "Bank client socket creation failed"<<endl;
                exit(EXIT_FAILURE);
            }
            memset(&client_server_addr, 0, sizeof(client_server_addr));
            client_server_addr.sin_family = AF_INET;
            client_server_addr.sin_port = htons(client_port);
            inet_pton(AF_INET, server_ip.c_str(), &client_server_addr.sin_addr);

            cout << "Client " << client.name << " viewed exchange list on port " << client_port << endl;
            string message = "EXCHANGE_LIST_CLIENT " ;
            for (auto &it : exchanges) {
                message += "(" + it.second.name + " : " + to_string(it.second.port) + ") ";
            }
            sendto(client_sock_fd, message.c_str(), message.size(), 0, (const struct sockaddr *)&client_server_addr, sizeof(client_server_addr));
            close(client_sock_fd);
        }
        else if (data == "VIEW_HISTORY_CLIENT") {
            iss >> data; // port
            int client_port = stoi(data);
            iss >> data;
            int page = stoi(data);
            string message = "VIEW_HISTORY_CLIENT ";
            if (page <= history.size()/page_size) {
                ClientInfo client = clients[client_port];

                cout << "Client " << client.name << "viewed history on port " << to_string(client_port) +" page : " + to_string(page)<< endl;
                for (int i = page*page_size; i < min(page_size*(page+1),(int)history.size()); i++) {
                    message += "(" + history[i] + ") ";
                }
            }
            else {
                message = "VIEW_HISTORY_CLIENT failed";
            }
            struct sockaddr_in client_server_addr{};
            int client_sock_fd;
            if ((client_sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                cout << "Bank client socket creation failed"<<endl;
                exit(EXIT_FAILURE);
            }
            memset(&client_server_addr, 0, sizeof(client_server_addr));
            client_server_addr.sin_family = AF_INET;
            client_server_addr.sin_port = htons(client_port);
            inet_pton(AF_INET, server_ip.c_str(), &client_server_addr.sin_addr);

            sendto(client_sock_fd, message.c_str(), message.size(), 0, (const struct sockaddr *)&client_server_addr, sizeof(client_server_addr));
            close(client_sock_fd);
        }
        else if (data == "ADD_NEW_CRYPTO") {
            ExchangeInfo exchange;
            iss >> data; // name
            string crypto_name = data;
            iss >> data; // port
            int exchange_port = stoi(data);
            string message = "ADDED_NEW_CRYPTO ";
            if (cryptocurrencies.contains(crypto_name)) {
                message += "Failed ";
                cout << "An exchange failed to create crypto : "+ crypto_name + " on port " << exchange_port << "\n";
            }else {
                message += "Success " + crypto_name;
                cout << "An exchange created crypto : "+ crypto_name + " on port " << exchange_port << "\n";
                SafeItem item;
                item.state = "preorder";
                cryptocurrencies[crypto_name] = item;
            }
            struct sockaddr_in exchange_server_addr{};
            int exchange_sock_fd;
            if ((exchange_sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                cout << "Bank exchange socket creation failed"<<endl;
                exit(EXIT_FAILURE);
            }
            memset(&exchange_server_addr, 0, sizeof(exchange_server_addr));
            exchange_server_addr.sin_family = AF_INET;
            exchange_server_addr.sin_port = htons(exchange_port);
            inet_pton(AF_INET, server_ip.c_str(), &exchange_server_addr.sin_addr);
            message = simpleHash(message) + " " + message;
            sendto(exchange_sock_fd, message.c_str(), message.size(), 0, (const struct sockaddr *)&exchange_server_addr, sizeof(exchange_server_addr));
            close(exchange_sock_fd);
        }
        else if (data == "BUY_CRYPTO_CLIENT_CHECK") {
            iss >> data; // name
            string crypto_name = data;
            iss >> data; // amount
            int amount = stoi(data);
            iss >> data;
            double price = stod(data);
            iss >> data;
            int client_port = stoi(data);
            iss >> data;
            int exchange_port = stoi(data);
            ClientInfo client = clients[client_port];
            string message = "BUY_CRYPTO_CLIENT_CHECKED ";
            if (client.wallet.balance >= price*amount) {
                clients[client_port].wallet.balance -= price*amount;
                clients[client_port].wallet.cryptocurrencies[crypto_name] += amount;
                message += "Success ";
            }
            else {
                message += "Failed ";
            }

            struct sockaddr_in exchange_server_addr{};
            int exchange_sock_fd;
            if ((exchange_sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                cout << "Bank exchange socket creation failed"<<endl;
                exit(EXIT_FAILURE);
            }
            message += crypto_name + " " + to_string(price*amount) + " " + to_string(amount) + " " + to_string(client_port);
            memset(&exchange_server_addr, 0, sizeof(exchange_server_addr));
            exchange_server_addr.sin_family = AF_INET;
            exchange_server_addr.sin_port = htons(exchange_port);
            inet_pton(AF_INET, server_ip.c_str(), &exchange_server_addr.sin_addr);
            history.push_back("Bought " + crypto_name + " : " + to_string(amount) + " used : " + to_string(price*amount));
            message = simpleHash(message) + " " + message;
            sendto(exchange_sock_fd, message.c_str(), message.size(), 0, (const struct sockaddr *)&exchange_server_addr, sizeof(exchange_server_addr));
            close(exchange_sock_fd);
        }
        else if (data == "RELEASE_CRYPTO") {
            iss >> data;
            string crypto_name = data;
            iss >> data;
            double price = stod(data);
            iss >> data;
            int init_count = stoi(data);
            string message = "DO_RELEASE_CRYPTO " + crypto_name + " " + to_string(price) + " " + to_string(init_count);
            cryptocurrencies[crypto_name].state = "released";
            cryptocurrencies[crypto_name].price = price;
            cryptocurrencies[crypto_name].init_count = init_count;
            for ( auto & exchange : exchanges) {
                struct sockaddr_in exchange_server_addr{};
                int exchange_sock_fd;
                if ((exchange_sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                    cout << "Bank exchange socket creation failed"<<endl;
                    exit(EXIT_FAILURE);
                }
                memset(&exchange_server_addr, 0, sizeof(exchange_server_addr));
                exchange_server_addr.sin_family = AF_INET;
                exchange_server_addr.sin_port = htons(exchange.first);
                inet_pton(AF_INET, server_ip.c_str(), &exchange_server_addr.sin_addr);
                message = simpleHash(message) + " " + message;
                sendto(exchange_sock_fd, message.c_str(), message.size(), 0, (const struct sockaddr *)&exchange_server_addr, sizeof(exchange_server_addr));
                close(exchange_sock_fd);

            }

        }else if (data == "SELL_CRYPTO_CLIENT_CHECK") {
            iss >> data; // name
            string crypto_name = data;
            iss >> data; // amount
            int amount = stoi(data);
            iss >> data;
            double price = stod(data);
            iss >> data;
            int client_port = stoi(data);
            iss >> data;
            int exchange_port = stoi(data);
            ClientInfo client = clients[client_port];
            string message = "SELL_CRYPTO_CLIENT_CHECKED ";
            if (client.wallet.cryptocurrencies[crypto_name] >= amount) {
                clients[client_port].wallet.cryptocurrencies[crypto_name] -= amount;
                clients[client_port].wallet.balance += price*amount;
                message += "Success ";
            }
            else {
                message += "Failed ";
            }

            struct sockaddr_in exchange_server_addr{};
            int exchange_sock_fd;
            if ((exchange_sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                cout << "Bank exchange socket creation failed"<<endl;
                exit(EXIT_FAILURE);
            }
            message += crypto_name + " " + to_string(price*amount) + " " + to_string(amount) + " " + to_string(client_port);
            memset(&exchange_server_addr, 0, sizeof(exchange_server_addr));
            exchange_server_addr.sin_family = AF_INET;
            exchange_server_addr.sin_port = htons(exchange_port);
            inet_pton(AF_INET, server_ip.c_str(), &exchange_server_addr.sin_addr);
            history.push_back("Sold " + crypto_name + " : " + to_string(amount) + " gained : " + to_string(price*amount) );
            message = simpleHash(message) + " " + message;
            sendto(exchange_sock_fd, message.c_str(), message.size(), 0, (const struct sockaddr *)&exchange_server_addr, sizeof(exchange_server_addr));
            close(exchange_sock_fd);
        }
        else if (data == "BUY_CRYPTO_EXCHANGE_TO_BANK") {
            iss >> data;
            string crypto_name = data;
            iss >> data; // port
            int exchange_port = stoi(data);

            struct sockaddr_in exchange_server_addr{};
            int exchange_sock_fd;
            if ((exchange_sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                cout << "Bank client socket creation failed"<<endl;
                exit(EXIT_FAILURE);
            }
            memset(&exchange_server_addr, 0, sizeof(exchange_server_addr));
            exchange_server_addr.sin_family = AF_INET;
            exchange_server_addr.sin_port = htons(exchange_port);
            inet_pton(AF_INET, server_ip.c_str(), &exchange_server_addr.sin_addr);

            cout << "An exchange viewed exchange list on port " << exchange_port << endl;
            string message = "BUY_CRYPTO_EXCHANGE_TO_BANK_RESPONSE "  +crypto_name + " " + to_string(exchanges.size()) + " ";
            for (auto &it : exchanges) {
                if (exchange_port != it.first) {
                    message += to_string(it.second.port) + " ";
                }
            }
            sendto(exchange_sock_fd, message.c_str(), message.size(), 0, (const struct sockaddr *)&exchange_server_addr, sizeof(exchange_server_addr));
            close(exchange_sock_fd);
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