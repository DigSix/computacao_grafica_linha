cmake_minimum_required(VERSION 3.20)

project(Linha)

set(CMAKE_CXX_STANDARD 20)


find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)


add_executable(Linha
    src/main.cpp
)


target_link_libraries(Linha
    OpenGL::GL
    OpenGL::GLU
    GLUT::GLUT
)