cmake_minimum_required(VERSION 3.12)
project(dragonfruit-player CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "src/dragonfruit_player/src/*.cpp"
)

include(FetchContent)
FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
  GIT_TAG v6.1.9
)
FetchContent_MakeAvailable(ftxui)

add_subdirectory("src/dragonfruit_engine")

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE "src/dragonfruit_player/include")
target_link_libraries(${PROJECT_NAME} PRIVATE dragonfruit-engine ftxui::component ftxui::screen ftxui::dom)