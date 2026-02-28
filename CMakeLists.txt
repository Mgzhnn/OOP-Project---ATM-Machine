cmake_minimum_required(VERSION 4.0)
project(OOP_Project)

set(CMAKE_CXX_STANDARD 20)

add_executable(OOP_Project main.cpp
        Bank.cpp
        Bank.h
        Card.cpp
        Card.h
        Account.cpp
        Account.h
        OurCin.cpp
        OurCin.h
        Utility.cpp
        Utility.h
        Atm.cpp
        Atm.h
        Transaction.cpp
        Transaction.h
        Session.cpp
        Session.h
)
