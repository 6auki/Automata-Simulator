cmake_minimum_required(VERSION 3.20)
project(MyApp LANGUAGES CXX)

add_executable(MyApp
    main.cpp
    thompsons_construction.cpp
    postfix.cpp
    check.cpp
    nfa2dfa.cpp
    minimized_dfa.cpp
)
