cmake_minimum_required(VERSION 3.14)
project(FreePLC VERSION 2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ncurses
find_package(Curses REQUIRED)

include_directories(include ${CURSES_INCLUDE_DIR})

set(SOURCES
    main.cpp
    src/plcio.cpp
    src/gates.cpp
    src/ld_program.cpp
    src/relay_manager.cpp
    src/gui.cpp
)

add_executable(freeplc ${SOURCES})

target_link_libraries(freeplc
    ${CURSES_LIBRARIES}
    pthread
)
