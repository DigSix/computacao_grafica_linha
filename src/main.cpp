#include "Objeto3D.hpp"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <GL/freeglut.h>

namespace {

constexpr int larguraInicial = 800;
constexpr int alturaInicial = 600;
constexpr float passoTranslacao = 0.1f;
constexpr float passoRotacao = 5.0f;
constexpr float passoEscala = 1.1f;

std::unique_ptr<Objeto3D> objeto;
int larguraJanela = larguraInicial;
int alturaJanela = alturaInicial;
int mouseX = 0;
int mouseY = 0;
bool arrastandoRotacao = false;
bool arrastandoTranslacao = false;

void configurarCamera() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    const double aspecto = static_cast<double>(larguraJanela) /
                           static_cast<double>(alturaJanela);
    gluPerspective(60.0, aspecto, 0.1, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(4.0, 3.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

void desenharCena() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    configurarCamera();

    glColor3f(0.05f, 0.05f, 0.05f);
    objeto->desenhar();

    glutSwapBuffers();
}

void redimensionar(int largura, int altura) {
    larguraJanela = largura;
    alturaJanela = altura > 0 ? altura : 1;
    glViewport(0, 0, larguraJanela, alturaJanela);
    glutPostRedisplay();
}

void teclado(unsigned char tecla, int, int) {
    switch (tecla) {
    case 27:
        std::exit(EXIT_SUCCESS);
    case 'w':
        objeto->transladar(0.0f, passoTranslacao, 0.0f);
        break;
    case 's':
        objeto->transladar(0.0f, -passoTranslacao, 0.0f);
        break;
    case 'a':
        objeto->transladar(-passoTranslacao, 0.0f, 0.0f);
        break;
    case 'd':
        objeto->transladar(passoTranslacao, 0.0f, 0.0f);
        break;
    case 'q':
        objeto->transladar(0.0f, 0.0f, passoTranslacao);
        break;
    case 'e':
        objeto->transladar(0.0f, 0.0f, -passoTranslacao);
        break;
    case 'i':
        objeto->rotacionar(passoRotacao, 0.0f, 0.0f);
        break;
    case 'k':
        objeto->rotacionar(-passoRotacao, 0.0f, 0.0f);
        break;
    case 'j':
        objeto->rotacionar(0.0f, -passoRotacao, 0.0f);
        break;
    case 'l':
        objeto->rotacionar(0.0f, passoRotacao, 0.0f);
        break;
    case 'u':
        objeto->rotacionar(0.0f, 0.0f, passoRotacao);
        break;
    case 'o':
        objeto->rotacionar(0.0f, 0.0f, -passoRotacao);
        break;
    case '+':
    case '=':
        objeto->escalar(passoEscala);
        break;
    case '-':
    case '_':
        objeto->escalar(1.0f / passoEscala);
        break;
    case 'r':
    case 'R':
        objeto->resetarTransformacao();
        break;
    default:
        return;
    }

    glutPostRedisplay();
}

void teclasEspeciais(int tecla, int, int) {
    switch (tecla) {
    case GLUT_KEY_UP:
        objeto->transladar(0.0f, passoTranslacao, 0.0f);
        break;
    case GLUT_KEY_DOWN:
        objeto->transladar(0.0f, -passoTranslacao, 0.0f);
        break;
    case GLUT_KEY_LEFT:
        objeto->transladar(-passoTranslacao, 0.0f, 0.0f);
        break;
    case GLUT_KEY_RIGHT:
        objeto->transladar(passoTranslacao, 0.0f, 0.0f);
        break;
    case GLUT_KEY_PAGE_UP:
        objeto->transladar(0.0f, 0.0f, -passoTranslacao);
        break;
    case GLUT_KEY_PAGE_DOWN:
        objeto->transladar(0.0f, 0.0f, passoTranslacao);
        break;
    default:
        return;
    }

    glutPostRedisplay();
}

void mouse(int botao, int estado, int x, int y) {
    if (estado == GLUT_DOWN) {
        mouseX = x;
        mouseY = y;
        arrastandoRotacao = botao == GLUT_LEFT_BUTTON;
        arrastandoTranslacao = botao == GLUT_RIGHT_BUTTON;

        if (botao == 3) {
            objeto->escalar(passoEscala);
        } else if (botao == 4) {
            objeto->escalar(1.0f / passoEscala);
        }
    } else {
        arrastandoRotacao = false;
        arrastandoTranslacao = false;
    }

    glutPostRedisplay();
}

void movimentoMouse(int x, int y) {
    const int deltaX = x - mouseX;
    const int deltaY = y - mouseY;
    mouseX = x;
    mouseY = y;

    if (arrastandoRotacao) {
        objeto->rotacionar(
            static_cast<float>(deltaY) * 0.5f,
            static_cast<float>(deltaX) * 0.5f,
            0.0f
        );
    } else if (arrastandoTranslacao) {
        objeto->transladar(
            static_cast<float>(deltaX) * 0.01f,
            static_cast<float>(-deltaY) * 0.01f,
            0.0f
        );
    } else {
        return;
    }

    glutPostRedisplay();
}

} // namespace

int main(int argc, char** argv) {
    try {
        if (argc > 1) {
            objeto = std::make_unique<Objeto3D>(std::string(argv[1]));
            std::cout << "Modelo carregado: " << argv[1] << '\n';
        } else {
            objeto = std::make_unique<Objeto3D>(TipoGeometria::Cubo);
            std::cout << "Nenhum OBJ informado. Usando cubo primitivo.\n";
        }
    } catch (const std::exception& erro) {
        std::cerr << "Erro ao carregar modelo: " << erro.what() << '\n';
        return EXIT_FAILURE;
    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(larguraInicial, alturaInicial);
    glutCreateWindow("M1 - Transformacoes 3D");

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glLineWidth(2.0f);

    glutDisplayFunc(desenharCena);
    glutReshapeFunc(redimensionar);
    glutKeyboardFunc(teclado);
    glutSpecialFunc(teclasEspeciais);
    glutMouseFunc(mouse);
    glutMotionFunc(movimentoMouse);

    glutMainLoop();
    return 0;
}
