cmake_minimum_required(VERSION 3.20)

# Nome do projeto e padrão da linguagem usado pelo compilador.
project(Linha)

set(CMAKE_CXX_STANDARD 20)


# Localiza as bibliotecas instaladas no sistema.
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)


# Os dois arquivos formam a aplicação: janela/callbacks e objeto 3D.
add_executable(Linha
    src/main.cpp
    src/Objeto3D.cpp
)

# Permite que main.cpp encontre Objeto3D.hpp dentro de src/.
target_include_directories(Linha PRIVATE src)


# GLU fornece funções auxiliares da câmera, como gluPerspective e gluLookAt.
target_link_libraries(Linha
    OpenGL::GL
    OpenGL::GLU
    GLUT::GLUT
)
