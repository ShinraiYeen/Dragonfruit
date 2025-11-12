project(dragonfruit-engine VERSION 0.1 LANGUAGES CXX)

find_package(PkgConfig REQUIRED)
pkg_check_modules(PULSEAUDIO REQUIRED libpulse)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "src/*.cpp"
)

add_library(${PROJECT_NAME} ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC "include" PRIVATE ${PULSEAUDIO_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${PULSEAUDIO_LIBRARIES} spdlog::spdlog)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE ENABLE_DEBUG_LOGGING)
endif()
