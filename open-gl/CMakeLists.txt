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
# Automatically add all examples (recursively)
# -----------------------------
file(GLOB_RECURSE EXAMPLES
  RELATIVE ${CMAKE_SOURCE_DIR}
  [0-9]*/*/main.cpp
)

foreach(example_file ${EXAMPLES})
  # Get the directory (e.g. "1.getting-started/1.1.hello-window")
  get_filename_component(example_dir ${example_file} DIRECTORY)

  # Use only the last folder as the target name (hello-window)
  get_filename_component(target_name ${example_dir} NAME)

  # Create executable
  add_executable(${target_name} ${example_file})
  target_link_libraries(${target_name} PRIVATE glad glfw)

  # Put all executables in a central bin folder
  set_target_properties(${target_name} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${example_dir}
  )

  message(STATUS "Added example: ${example_dir} → target ${target_name}")
endforeach()

