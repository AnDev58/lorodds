find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

qt_add_executable(gui
  src/main.cpp

  # --- View ---
  src/view/rotator.h
  src/view/rotator.cpp
  src/view/main_window.cpp
  src/view/main_window.h
  src/view/main_window.ui

  # --- ViewModel ---
  src/viewmodel/page_viewmodel.cpp
  src/viewmodel/clash_viewmodel.cpp

  # --- Resources ---
  assets/dice.qrc
  assets/pages.qrc
)

target_link_libraries(gui PRIVATE core Qt6::Widgets Qt6::Gui Qt6::Core)
