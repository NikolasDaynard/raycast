cmake_minimum_required(VERSION 3.21) # SDL3 requires CMake >= 3.21
project(raycast VERSION 0.1.0 LANGUAGES C)


# Add SDL3 package path
set(CMAKE_PREFIX_PATH "/nix/store/jkjmp40sysaqns8j9nqkhvsyw4i9hmqd-sdl3-3.1.6-dev/")
#set(CMAKE_PREFIX_PATH "$ENV{SDL3_PATH}")

# Find SDL3 package
find_package(SDL3 REQUIRED)
find_package(OpenGL REQUIRED)

add_compile_definitions(GL_GLEXT_PROTOTYPES=1)
add_compile_definitions(WINDOW_WIDTH=640)
add_compile_definitions(WINDOW_HEIGHT=480)

# Add your executable
file(GLOB_RECURSE SOURCES "src/*.c")
add_executable(raycast ${SOURCES})

# Link SDL3 libraries
target_link_libraries(raycast PRIVATE SDL3::SDL3)
target_link_libraries(raycast PRIVATE OpenGL::GL)

find_library(MATH_LIBRARY m)
if(MATH_LIBRARY)
    target_link_libraries(raycast PUBLIC ${MATH_LIBRARY})
endif()

# Enable testing if needed
include(CTest)
enable_testing()

# Package settings (optional)
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
