cmake_minimum_required(VERSION 3.20)
project(OOP_Project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(OOP_Project
        src/main.cpp
        src/Account.cpp
        src/Account.h
        src/Atm.cpp
        src/Atm.h
        src/Bank.cpp
        src/Bank.h
        src/Card.cpp
        src/Card.h
        src/OurCin.cpp
        src/OurCin.h
        src/Session.cpp
        src/Session.h
        src/Transaction.cpp
        src/Transaction.h
        src/Utility.cpp
        src/Utility.h
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(OOP_Project PRIVATE -Wall -Wextra -pedantic)
endif()
