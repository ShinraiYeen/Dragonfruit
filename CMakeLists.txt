cmake_minimum_required(VERSION 3.14)
project(dragonfruit-player VERSION 0.1.1 LANGUAGES CXX)

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

# CPack configuration
include(InstallRequiredSystemLibraries)
set(CPACK_GENERATOR "DEB")

# Package information
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_CONTACT "shinrai.wolf@outlook.com")
set(CPACK_PACKAGE_DESCRIPTION "A CLI music player.")
set(CPACK_PACKAGE_VENDOR "ShinraiYeen")

# Debian-specific package information
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "ShinraiYeen")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libpulse0")
set(CPACK_DEBIAN_PACKAGE_SECTION "sound")

if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "arm64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "i.86")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "i386")
else()
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "${CMAKE_SYSTEM_PROCESSOR}")
endif()

set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")

include(CPack)