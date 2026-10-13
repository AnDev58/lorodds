find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(lorodds_gui
  # --- Sources ---
  src/main.cpp
  src/clash_view.cpp
  src/clash_model.cpp
  src/page_selector.cpp

  # --- Headers ---
  include/clash_view.h
  include/clash_model.h
  include/page_selector.h
)

target_compile_features(lorodds_gui PRIVATE cxx_std_20)

target_include_directories(lorodds_gui PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(lorodds_gui PRIVATE
  core
  Qt6::Widgets
)

set_target_properties(lorodds_gui PROPERTIES WIN32_EXECUTABLE TRUE)
