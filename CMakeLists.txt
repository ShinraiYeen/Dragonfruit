cmake_minimum_required(VERSION 3.14)
project(dragonfruit-player VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

# Display build configuration
message(STATUS "${PROJECT_NAME} build settings:")
message(STATUS "    Project version: ${PROJECT_VERSION}")
message(STATUS "    Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "    Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "    Cmake Version: ${CMAKE_VERSION}")
message(STATUS "    System: ${CMAKE_SYSTEM_NAME}")
message(STATUS "    Processor: ${CMAKE_SYSTEM_PROCESSOR}")

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "src/dragonfruit_player/src/*.cpp"
)

include(FetchContent)
FetchContent_Declare(ftxui
  GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
  GIT_TAG v6.1.9
)

# We do not want to install FTXUI headers/libraries. Disabling that here.
set(FTXUI_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(ftxui)

add_subdirectory("src/dragonfruit_engine")

add_executable(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE "src/dragonfruit_player/include")
target_link_libraries(${PROJECT_NAME} PRIVATE dragonfruit-engine ftxui::component ftxui::screen ftxui::dom)

# Enable all warnings
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)

# Set installation rules
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "bin")