add_executable(hello-window main.cpp)

target_link_libraries(hello-window PRIVATE glfw glad)
target_include_directories(hello-window PRIVATE ${CMAKE_SOURCE_DIR}/external/glad/include)

