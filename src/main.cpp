#include "Objeto3D.hpp"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <GL/freeglut.h>

namespace {

// Constantes de interação: alterar estes valores muda a sensibilidade dos
// controles sem precisar modificar cada callback.
constexpr int larguraInicial = 800;
constexpr int alturaInicial = 600;
constexpr float passoTranslacao = 0.1f;
constexpr float passoRotacao = 5.0f;
constexpr float passoEscala = 1.1f;

// O ponteiro permite escolher a geometria depois de ler os argumentos da
// linha de comando, antes de iniciar a janela do FreeGLUT.
std::unique_ptr<Objeto3D> objeto; // Modelo atualmente exibido.
int larguraJanela = larguraInicial; // Largura usada no cálculo do aspecto.
int alturaJanela = alturaInicial;   // Altura usada no cálculo do aspecto.
int mouseX = 0; // Última posição horizontal conhecida do mouse.
int mouseY = 0; // Última posição vertical conhecida do mouse.
bool arrastandoRotacao = false; // Botão esquerdo está pressionado.
bool arrastandoTranslacao = false; // Botão direito está pressionado.
bool luzAtiva[3] = {true, true, true}; // Estado persistente das três luzes.

void configurarIluminacao() {
    // Cada luz possui posição e as três componentes exigidas no trabalho:
    // ambiente, difusa e especular.
    const GLenum luzes[] = {GL_LIGHT0, GL_LIGHT1, GL_LIGHT2};
    const GLfloat posicoes[][4] = {
        { 3.0f,  3.0f,  3.0f, 1.0f},
        {-3.0f,  2.0f,  1.0f, 1.0f},
        { 0.0f, -2.0f, -3.0f, 1.0f}
    };
    // Ambientes baixos deixam a diferença entre as luzes mais perceptível.
    const GLfloat ambientes[][4] = {
        {0.02f, 0.00f, 0.00f, 1.0f},
        {0.00f, 0.02f, 0.00f, 1.0f},
        {0.00f, 0.00f, 0.02f, 1.0f}
    };
    // Cada luz tem uma cor predominante diferente para facilitar o teste.
    const GLfloat difusas[][4] = {
        {1.0f, 0.10f, 0.10f, 1.0f},
        {0.10f, 1.0f, 0.10f, 1.0f},
        {0.10f, 0.10f, 1.0f, 1.0f}
    };
    const GLfloat especular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    const GLfloat materialEspecular[] = {1.0f, 1.0f, 1.0f, 1.0f};

    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    for (int i = 0; i < 3; ++i) {
        glLightfv(luzes[i], GL_POSITION, posicoes[i]);
        glLightfv(luzes[i], GL_AMBIENT, ambientes[i]);
        glLightfv(luzes[i], GL_DIFFUSE, difusas[i]);
        glLightfv(luzes[i], GL_SPECULAR, especular);
        // Não habilita sempre: respeita o estado escolhido pelo usuário.
        if (luzAtiva[i]) {
            glEnable(luzes[i]);
        } else {
            glDisable(luzes[i]);
        }
    }

    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, materialEspecular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 64.0f);
}

void alternarLuz(int indice) {
    const GLenum luz = GL_LIGHT0 + indice;
    luzAtiva[indice] = !luzAtiva[indice];

    if (!luzAtiva[indice]) {
        glDisable(luz);
    } else {
        glEnable(luz);
    }
}

void configurarCamera() {
    // A matriz de projeção define como o mundo 3D será projetado na janela.
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // O cast evita uma divisão inteira no cálculo da proporção.
    const double aspecto = static_cast<double>(larguraJanela) /
                           static_cast<double>(alturaJanela);
    gluPerspective(60.0, aspecto, 0.1, 100.0);

    // A matriz MODELVIEW combina a posição da câmera com as transformações
    // aplicadas aos objetos.
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    // Câmera em (4, 3, 5), olhando para a origem, com Y como direção vertical.
    gluLookAt(4.0, 3.0, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    // A posição da luz é transformada pela matriz atual; por isso as luzes
    // são configuradas depois da câmera em cada frame.
    configurarIluminacao();
}

void desenharCena() {
    // O buffer de profundidade é limpo junto com a cor a cada novo frame.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    configurarCamera();

    // A cor é definida antes do objeto porque afeta os vértices seguintes.
    // Branco permite que a cor de cada luz seja percebida no objeto.
    glColor3f(1.0f, 1.0f, 1.0f);
    objeto->desenhar();

    glutSwapBuffers();
}

void redimensionar(int largura, int altura) {
    // Atualiza o viewport e o aspecto da câmera para evitar deformação.
    // Evita altura zero, que causaria divisão por zero na câmera.
    larguraJanela = largura;
    alturaJanela = altura > 0 ? altura : 1;
    glViewport(0, 0, larguraJanela, alturaJanela);
    glutPostRedisplay();
}

void teclado(unsigned char tecla, int, int) {
    // Teclas comuns controlam as três operações da M1.
    switch (tecla) {
    case 27:
        // ESC encerra imediatamente a aplicação.
        std::exit(EXIT_SUCCESS);
    case 'w':
        // W/S movimentam no eixo Y; A/D no eixo X; Q/E no eixo Z.
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
        // I/K giram em X; J/L em Y; U/O em Z.
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
        // O sinal de mais aumenta e o sinal de menos reduz a escala.
        objeto->escalar(passoEscala);
        break;
    case '-':
    case '_':
        objeto->escalar(1.0f / passoEscala);
        break;
    case 'r':
    case 'R':
        // R volta o objeto ao estado inicial.
        objeto->resetarTransformacao();
        break;
    case '1':
        alternarLuz(0);
        break;
    case '2':
        alternarLuz(1);
        break;
    case '3':
        alternarLuz(2);
        break;
    default:
        return;
    }

    // Solicita um novo frame depois da alteração.
    glutPostRedisplay();
}

void teclasEspeciais(int tecla, int, int) {
    // As setas e Page Up/Page Down são tratadas separadamente pelo GLUT.
    switch (tecla) {
    // Teclas especiais não passam pelo callback teclado().
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
    // Botão esquerdo inicia rotação; botão direito inicia translação.
    // Em algumas plataformas, os botões 3 e 4 representam o scroll.
    if (estado == GLUT_DOWN) {
        // Guarda o ponto inicial do arraste para calcular deltas depois.
        mouseX = x;
        mouseY = y;
        arrastandoRotacao = botao == GLUT_LEFT_BUTTON;
        arrastandoTranslacao = botao == GLUT_RIGHT_BUTTON;

        if (botao == 3) {
            // Scroll para cima aumenta a escala.
            objeto->escalar(passoEscala);
        } else if (botao == 4) {
            // Scroll para baixo diminui a escala.
            objeto->escalar(1.0f / passoEscala);
        }
    } else {
        arrastandoRotacao = false;
        arrastandoTranslacao = false;
    }

    glutPostRedisplay();
}

void movimentoMouse(int x, int y) {
    // Usa a diferença entre a posição atual e a anterior para calcular
    // quanto o objeto deve ser transformado.
    // Delta é a distância percorrida desde o último evento do mouse.
    const int deltaX = x - mouseX;
    const int deltaY = y - mouseY;
    mouseX = x;
    mouseY = y;

    if (arrastandoRotacao) {
        // Movimento horizontal gira em Y; movimento vertical gira em X.
        objeto->rotacionar(
            static_cast<float>(deltaY) * 0.5f,
            static_cast<float>(deltaX) * 0.5f,
            0.0f
        );
    } else if (arrastandoTranslacao) {
        // O botão direito transforma o movimento do mouse em translação XY.
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
    // O objeto é criado antes da janela para que erros de arquivo sejam
    // mostrados no terminal sem iniciar uma cena inválida.
    try {
        if (argc > 1) {
            const std::string argumento = argv[1];

            // Argumentos conhecidos selecionam primitivas; qualquer outro
            // argumento é tratado como caminho para um arquivo OBJ.
            if (argumento == "cubo") {
                objeto = std::make_unique<Objeto3D>(TipoGeometria::Cubo);
                std::cout << "Usando cubo primitivo.\n";
            } else if (argumento == "piramide") {
                objeto = std::make_unique<Objeto3D>(TipoGeometria::Piramide);
                std::cout << "Usando piramide primitiva.\n";
            } else {
                objeto = std::make_unique<Objeto3D>(argumento);
                std::cout << "Modelo carregado: " << argumento << '\n';
            }
        } else {
            objeto = std::make_unique<Objeto3D>(TipoGeometria::Cubo);
            std::cout << "Nenhum OBJ informado. Usando cubo primitivo.\n";
        }
    } catch (const std::exception& erro) {
        std::cerr << "Erro ao carregar modelo: " << erro.what() << '\n';
        return EXIT_FAILURE;
    }

    // A geometria é carregada antes da janela, pois o parser não depende de
    // funções da GPU. O contexto OpenGL é necessário somente para desenhar.
    // Inicializa o FreeGLUT e cria o contexto OpenGL.
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(larguraInicial, alturaInicial);
    glutCreateWindow("M1 - Transformacoes 3D");

    // Configurações iniciais do estado OpenGL.
    glClearColor(0.06f, 0.06f, 0.06f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    // Reescala as normais automaticamente quando o objeto é escalado.
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
    glLineWidth(2.0f);

    // Registro dos callbacks: o FreeGLUT chama essas funções quando os
    // eventos correspondentes acontecem.
    glutDisplayFunc(desenharCena);
    glutReshapeFunc(redimensionar);
    glutKeyboardFunc(teclado);
    glutSpecialFunc(teclasEspeciais);
    glutMouseFunc(mouse);
    glutMotionFunc(movimentoMouse);

    // A partir daqui o FreeGLUT controla o fluxo e chama os callbacks.
    glutMainLoop();
    return 0;
}
