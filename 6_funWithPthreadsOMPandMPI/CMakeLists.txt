cmake_minimum_required(VERSION 3.6)
project(funWithLife)

include_directories ("${PROJECT_SOURCE_DIR}/liblife")
add_subdirectory(liblife)

add_executable(main main.c)
target_link_libraries(main liblife)
set_property(TARGET main PROPERTY C_STANDARD 99)

add_executable(tests tests.c)
target_link_libraries(tests liblife)
set_property(TARGET tests PROPERTY C_STANDARD 99)
