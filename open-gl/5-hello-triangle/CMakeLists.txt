add_executable(hello-triangle main.cpp)

target_link_libraries(hello-triangle PRIVATE glfw glad)
target_include_directories(hello-triangle PRIVATE ${CMAKE_SOURCE_DIR}/external/glad/include)

