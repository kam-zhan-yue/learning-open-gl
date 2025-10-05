cmake_minimum_required(VERSION 3.15)
project(LearningOpenGL)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add GLAD as a library
add_library(glad external/glad/src/glad.c)
target_include_directories(glad PUBLIC external/glad/include)

# Find GLFW
include(FetchContent)

FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.3.9
)
FetchContent_MakeAvailable(glfw)

# Add each example project
add_subdirectory(3-creating-a-window)
