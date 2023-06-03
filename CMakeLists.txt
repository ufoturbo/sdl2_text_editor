cmake_minimum_required(VERSION 3.0.0)
project(text_editor VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(text_editor main.cpp)
target_link_libraries(text_editor SDL2 SDL2main SDL2_ttf GL)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)