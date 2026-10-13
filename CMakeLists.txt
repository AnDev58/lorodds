cmake_minimum_required(VERSION 3.24..4.4)

project(lorodds
  VERSION 0.1
  DESCRIPTION "Odds calculator for Library of Ruina"
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(core)
