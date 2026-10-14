cmake_minimum_required(VERSION 3.24..4.4)

project(lorodds
  VERSION 0.1
  DESCRIPTION "Odds calculator for Library of Ruina"
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_GUI "Build standalone application" ON)
option(BUILD_WASM "Build WASM module" ON)
option(BUILD_FFI "Build FFI wrapper" OFF)

add_subdirectory(core)

if(BUILD_GUI)
  add_subdirectory(gui)
endif()
