cmake_minimum_required(VERSION 3.15)
project(LearningOpenGL)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------------
# GLAD
# -----------------------------
add_library(glad external/glad/src/glad.c)
target_include_directories(glad PUBLIC external/glad/include)

# -----------------------------
# GLFW (via FetchContent)
# -----------------------------
include(FetchContent)

FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.3.9
)
FetchContent_MakeAvailable(glfw)

# -----------------------------
# Automatically add all subprojects
# -----------------------------
file(GLOB_RECURSE EXAMPLES RELATIVE ${CMAKE_SOURCE_DIR} */main.cpp)

foreach(example_file ${EXAMPLES})
  get_filename_component(example_dir ${example_file} DIRECTORY)
  get_filename_component(example_name ${example_dir} NAME)

  add_executable(${example_name} ${example_file})
  target_link_libraries(${example_name} PRIVATE glad glfw)
  message(STATUS "Added example: ${example_name}")
endforeach()

