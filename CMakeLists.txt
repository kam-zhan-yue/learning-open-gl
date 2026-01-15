cmake_minimum_required(VERSION 3.15)
project(LearningOpenGL)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------------
# GLAD
# -----------------------------
add_library(glad external/glad/src/glad.c)
target_include_directories(glad PUBLIC external/glad/include)

# -----------------------------
# STB_IMAGE
# -----------------------------
add_library(stb_image external/stb_image.cpp)
target_include_directories(stb_image PUBLIC includes)

# -----------------------------
# GLM (header-only)
# -----------------------------
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE includes)

# FetchContent stuff
include(FetchContent)

# -----------------------------
# Assimp (via FetchContent)
# -----------------------------
FetchContent_Declare(
  assimp
  GIT_REPOSITORY https://github.com/assimp/assimp.git
  GIT_TAG master
)

set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_ZLIB ON CACHE INTERNAL "" FORCE)

FetchContent_MakeAvailable(assimp)

# -----------------------------
# GLFW (via FetchContent)
# -----------------------------

FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.3.9
)
FetchContent_MakeAvailable(glfw)

# -----------------------------
# FreeType (via FetchContent)
# -----------------------------

FetchContent_Declare(
  freetype
  GIT_REPOSITORY https://gitlab.freedesktop.org/freetype/freetype
  GIT_TAG VER-2-14-1
)
FetchContent_MakeAvailable(freetype)


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
  target_link_libraries(${target_name} PRIVATE glad glfw stb_image assimp freetype)
  target_include_directories(${target_name} PRIVATE includes)

  # Put all executables in a central bin folder
  set_target_properties(${target_name} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${example_dir}
  )

  # Make all calls to the shader directory absolute
  target_compile_definitions(${target_name} PRIVATE
    SHADER_DIR="${CMAKE_BINARY_DIR}/bin/${example_dir}"
    RESOURCES_DIR="${CMAKE_SOURCE_DIR}/resources"
  )

  # -----------------------------
  # Copy shader files to build folder
  # -----------------------------
  file(GLOB SHADERS
    ${CMAKE_SOURCE_DIR}/${example_dir}/*.vs
    ${CMAKE_SOURCE_DIR}/${example_dir}/*.fs
    ${CMAKE_SOURCE_DIR}/${example_dir}/*.glsl
    ${CMAKE_SOURCE_DIR}/${example_dir}/*.frag
    ${CMAKE_SOURCE_DIR}/${example_dir}/*.vert
    ${CMAKE_SOURCE_DIR}/${example_dir}/*.png
  )

  foreach(shader_file ${SHADERS})
    get_filename_component(shader_name ${shader_file} NAME)
    set(output_file ${CMAKE_BINARY_DIR}/bin/${example_dir}/${shader_name})
    
    add_custom_command(
      OUTPUT ${output_file}
      COMMAND ${CMAKE_COMMAND} -E copy_if_different
              ${shader_file}
              ${output_file}
      DEPENDS ${shader_file}
      COMMENT "Copying ${shader_name}"
    )
    
    list(APPEND SHADER_OUTPUTS ${output_file})
  endforeach()

  # Make target depend on shader outputs
  if(SHADER_OUTPUTS)
    add_custom_target(${target_name}_shaders ALL DEPENDS ${SHADER_OUTPUTS})
    add_dependencies(${target_name} ${target_name}_shaders)
  endif()

  message(STATUS "Added example: ${example_dir} → target ${target_name}")
endforeach()

