add_executable(creating-a-window main.cpp)

target_link_libraries(creating-a-window PRIVATE glfw glad)
target_include_directories(creating-a-window PRIVATE ${CMAKE_SOURCE_DIR}/external/glad/include)

