cmake_minimum_required(VERSION 3.12)
project(dragonfruit-engine CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PULSEAUDIO REQUIRED libpulse)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "src/*.cpp"
)

add_library(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC "include" PRIVATE ${PULSEAUDIO_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${PULSEAUDIO_LIBRARIES})
