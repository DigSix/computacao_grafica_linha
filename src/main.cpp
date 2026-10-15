#include "Objeto3D.hpp"

#include <cstdlib>
#include <GL/freeglut.h>

namespace {

constexpr int larguraInicial = 800;
constexpr int alturaInicial = 600;
constexpr float passoTranslacao = 0.1f;
constexpr float passoRotacao = 5.0f;
constexpr float passoEscala = 1.1f;

Objeto3D cubo(2.0f);
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
    cubo.desenhar();

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
        cubo.transladar(0.0f, passoTranslacao, 0.0f);
        break;
    case 's':
        cubo.transladar(0.0f, -passoTranslacao, 0.0f);
        break;
    case 'a':
        cubo.transladar(-passoTranslacao, 0.0f, 0.0f);
        break;
    case 'd':
        cubo.transladar(passoTranslacao, 0.0f, 0.0f);
        break;
    case 'q':
        cubo.transladar(0.0f, 0.0f, passoTranslacao);
        break;
    case 'e':
        cubo.transladar(0.0f, 0.0f, -passoTranslacao);
        break;
    case 'i':
        cubo.rotacionar(passoRotacao, 0.0f, 0.0f);
        break;
    case 'k':
        cubo.rotacionar(-passoRotacao, 0.0f, 0.0f);
        break;
    case 'j':
        cubo.rotacionar(0.0f, -passoRotacao, 0.0f);
        break;
    case 'l':
        cubo.rotacionar(0.0f, passoRotacao, 0.0f);
        break;
    case 'u':
        cubo.rotacionar(0.0f, 0.0f, passoRotacao);
        break;
    case 'o':
        cubo.rotacionar(0.0f, 0.0f, -passoRotacao);
        break;
    case '+':
    case '=':
        cubo.escalar(passoEscala);
        break;
    case '-':
    case '_':
        cubo.escalar(1.0f / passoEscala);
        break;
    case 'r':
    case 'R':
        cubo.resetarTransformacao();
        break;
    default:
        return;
    }

    glutPostRedisplay();
}

void teclasEspeciais(int tecla, int, int) {
    switch (tecla) {
    case GLUT_KEY_UP:
        cubo.transladar(0.0f, passoTranslacao, 0.0f);
        break;
    case GLUT_KEY_DOWN:
        cubo.transladar(0.0f, -passoTranslacao, 0.0f);
        break;
    case GLUT_KEY_LEFT:
        cubo.transladar(-passoTranslacao, 0.0f, 0.0f);
        break;
    case GLUT_KEY_RIGHT:
        cubo.transladar(passoTranslacao, 0.0f, 0.0f);
        break;
    case GLUT_KEY_PAGE_UP:
        cubo.transladar(0.0f, 0.0f, -passoTranslacao);
        break;
    case GLUT_KEY_PAGE_DOWN:
        cubo.transladar(0.0f, 0.0f, passoTranslacao);
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
            cubo.escalar(passoEscala);
        } else if (botao == 4) {
            cubo.escalar(1.0f / passoEscala);
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
        cubo.rotacionar(
            static_cast<float>(deltaY) * 0.5f,
            static_cast<float>(deltaX) * 0.5f,
            0.0f
        );
    } else if (arrastandoTranslacao) {
        cubo.transladar(
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
