cmake_minimum_required(VERSION 3.28)
project(os_project)

set(CMAKE_CXX_STANDARD 20)
add_subdirectory(bank)
add_subdirectory(exchange)
add_subdirectory(client)
#add_executable(os_project
#        bank/bank.cpp
#        client/client.cpp
#        exchange/exchange.cpp)
