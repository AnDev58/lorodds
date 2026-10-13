include(db/generator.cmake)

add_library(core
  # --- Sources ---
  src/clash_calculator.cpp

  # --- Headers ---
  include/clash_calculator.h

  # --- Generated ---
  ${CMAKE_CURRENT_BINARY_DIR}/core/combat_pages_db.h
)

target_compile_features(core PUBLIC cxx_std_20)

target_include_directories(core PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_BINARY_DIR}/core
)
