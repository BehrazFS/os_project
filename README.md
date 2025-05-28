# Operating-Systems-Digital-Currency-Exchange-System-Simulation–C++-UDP-Project

This project simulates the interaction between clients, digital currency exchanges, and a bank over the UDP protocol using C++. It is implemented without using external libraries and focuses on low-level system calls. This was the final project of Operating Systems course.

# 📌 Overview
The system consists of three main entities:

Client: Requests cryptocurrency prices, buys/sells coins, views balances, and adds funds.

Exchange: Offers cryptocurrency trading services, sets dynamic prices, and manages inventory.

Bank: Acts as a centralized fund manager, validating and processing deposit requests.

All communications are handled using UDP sockets, with each entity bound to a unique port and able to send/receive plain-text messages following a defined protocol.

#Team members:

    https://github.com/BehrazFS <br>
    https://github.com/RozhinaLatifi
