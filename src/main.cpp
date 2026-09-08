#include <iostream>
#include <cmath>
#include <GL/freeglut.h>
#include <vector>

struct Vec3{
	double x;
	double y;
	double z;
};

using vertice = Vec3;
using lista_vertices = std::vector<vertice>;
using aresta = std::pair<int, int>;
using lista_arestas = std::vector<aresta>;

struct Poligono {
	double tamanhoLado;
	int numLados;

	Vec3 posicao;
	Vec3 escala;
	double rotacao;

	lista_vertices vertices;
	lista_arestas arestas;
};

Poligono criar_poligono(
	double posicao_x,
	double posicao_y,
	double posicao_z,
	double tamanho_lado,
	int num_lados
);

void desenhar(Poligono poligono);
void movimentar(Poligono& poligono, double distancia, double angulo);
void escalar(Poligono& poligono, double escala_x, double escala_y);
void rotacionar(Poligono& poligono, double angulo);

// Implementação da tangente
double tangente(double angulo);

void display();
void redraw(int value);
void keyboard(unsigned char key, int x, int y);
void keyboard_special(int key, int x, int y);

Poligono cubo;
float velocidadeRotacao = 0;
int delay = 10;

int main(int argc, char** argv) {

	cubo = criar_poligono(128, 128, 128, 50, 4);

	glutInit(&argc, argv);

	glutInitWindowSize(512, 512);

	glutCreateWindow("Desenhando um Polígono");

	glClearColor(1, 1, 1, 0);

	glEnable(GL_DEPTH_TEST);

	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(keyboard_special);
	glutTimerFunc(10, redraw, 0);

	glutMainLoop();

	return 0;
}

double tangente(double angulo) {
	return sin(angulo) / cos(angulo);
}

void configurar_camera() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(
        60.0,    // campo de visão
        512.0 / 512.0, // aspect ratio
        0.1,     // plano próximo
        1000.0   // plano distante
    );

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Câmera olhando para o centro do cubo
    gluLookAt(
        300, 300, 300,   // posição da câmera
        128, 128, 103,   // ponto para onde olha
        0, 1, 0          // "cima"
    );
}

void display(void) {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    configurar_camera();

    desenhar(cubo);

    glFlush();
}

void keyboard(unsigned char key, int x, int y) {
	std::cout << key;

	switch (key) {

	case 27:
		exit(0);
		break;

	case 'o':
		escalar(cubo, 0.9, 0.9);
		break;

	case 'p':
		escalar(cubo, 1.1, 1.1);
		break;

	case 'k':
		velocidadeRotacao -= 0.01;
		break;

	case 'l':
		velocidadeRotacao += 0.01;
		break;
	}
}

void keyboard_special(int key, int x, int y) {
	std::cout << key;

	switch (key) {

	case GLUT_KEY_DOWN:
		movimentar(
			cubo,
			10,
			(270 / 180.0) * 3.1416
		);
		break;

	case GLUT_KEY_UP:
		movimentar(
			cubo,
			10,
			(90 / 180.0) * 3.1416
		);
		break;

	case GLUT_KEY_RIGHT:
		movimentar(
			cubo,
			10,
			(0 / 180.0) * 3.1416
		);
		break;

	case GLUT_KEY_LEFT:
		movimentar(
			cubo,
			10,
			(180 / 180.0) * 3.1416
		);
		break;
	}
}

void redraw(int value) {
	rotacionar(cubo, velocidadeRotacao);

	glutPostRedisplay();
	glutTimerFunc(delay, redraw, 0);
}

Poligono criar_poligono(
	double posicao_x,
	double posicao_y,
	double posicao_z,
	double tamanho_lado,
	int num_lados
) {
	Poligono novo_poligono;

	novo_poligono.numLados = num_lados;

	novo_poligono.posicao.x = posicao_x;
	novo_poligono.posicao.y = posicao_y;
	novo_poligono.posicao.z = posicao_z;

	novo_poligono.tamanhoLado = tamanho_lado;

	novo_poligono.escala.x = 1;
	novo_poligono.escala.y = 1;
	novo_poligono.escala.z = 1;

	novo_poligono.rotacao = 0;

	float angulo = 0;

	float passo_angulo =
		((360 / float(num_lados)) * 3.1415926536) / 180.0;

	// Apótema:
	// a = lado / (2 * tan(pi / n))
	float apothem =
		tamanho_lado /
		(2 * tangente(3.1416 / float(num_lados)));

	posicao_x -= tamanho_lado / 2.0;
	posicao_y -= apothem;

	novo_poligono.vertices.push_back(
			vertice(posicao_x, posicao_y, posicao_z)
		);
	novo_poligono.vertices.push_back(
			vertice(posicao_x, posicao_y, posicao_z - tamanho_lado)
	);

	// Vertices

	for (int i = 1; i < num_lados; i++) {

		posicao_x =
			posicao_x +
			tamanho_lado * cos(angulo);

		posicao_y =
			posicao_y +
			tamanho_lado * sin(angulo);

		novo_poligono.vertices.push_back(
			vertice(posicao_x, posicao_y, posicao_z)
		);
		novo_poligono.vertices.push_back(
			vertice(posicao_x, posicao_y, posicao_z - tamanho_lado)
		);

		angulo += passo_angulo;
	}

	// Arestas

	for (int i = 0; i < num_lados; i++) {
		int atual = i * 2;
		int proximo = ((i + 1) % num_lados) * 2;

		// Aresta da frente
		novo_poligono.arestas.push_back(
			aresta(atual, proximo)
		);

		// Aresta de trás
		novo_poligono.arestas.push_back(
			aresta(atual + 1, proximo + 1)
		);

		// Aresta lateral
		novo_poligono.arestas.push_back(
			aresta(atual, atual + 1)
		);
	}

	return novo_poligono;
}

void movimentar(
	Poligono& poligono,
	double distancia,
	double angulo
) {
	poligono.posicao.x =
		poligono.posicao.x +
		distancia * cos(angulo);

	poligono.posicao.y =
		poligono.posicao.y +
		distancia * sin(angulo);

	for (int i = 0; i < poligono.vertices.size(); i++) {

		poligono.vertices[i].x =
			poligono.vertices[i].x +
			distancia * cos(angulo);

		poligono.vertices[i].y =
			poligono.vertices[i].y +
			distancia * sin(angulo);
	}
}

void escalar(
	Poligono& poligono,
	double escala_x,
	double escala_y
) {
	poligono.escala.x *= escala_x;
	poligono.escala.y *= escala_y;

	for (int i = 0; i < poligono.vertices.size(); i++) {

		// Translada para a origem
		poligono.vertices[i].x -=
			poligono.posicao.x;

		poligono.vertices[i].y -=
			poligono.posicao.y;

		// Aplica escala
		poligono.vertices[i].x *= escala_x;
		poligono.vertices[i].y *= escala_y;

		// Volta para a posição original
		poligono.vertices[i].x +=
			poligono.posicao.x;

		poligono.vertices[i].y +=
			poligono.posicao.y;
	}
}

void rotacionar(
	Poligono& poligono,
	double angulo
) {
	poligono.rotacao += angulo;

	for (int i = 0;
		 i < poligono.vertices.size();
		 i++) {

		// Translada o vértice para a origem
		poligono.vertices[i].x -=
			poligono.posicao.x;

		poligono.vertices[i].y -=
			poligono.posicao.y;

		double novoX =
			poligono.vertices[i].x * cos(angulo) -
			poligono.vertices[i].y * sin(angulo);

		double novoY =
			poligono.vertices[i].x * sin(angulo) +
			poligono.vertices[i].y * cos(angulo);

		poligono.vertices[i].x = novoX;
		poligono.vertices[i].y = novoY;

		// Volta para a posição original
		poligono.vertices[i].x +=
			poligono.posicao.x;

		poligono.vertices[i].y +=
			poligono.posicao.y;
	}
}

void desenhar(Poligono poligono) {

	glColor3f(0, 0, 0);

	glBegin(GL_LINES);

	for (int i = 0;
		 i < poligono.arestas.size();
		 i++) {

		int v_o = poligono.arestas[i].first;
		int v_d = poligono.arestas[i].second;

		glVertex3f(
			poligono.vertices[v_o].x,
			poligono.vertices[v_o].y,
			poligono.vertices[v_o].z
		);

		glVertex3f(
			poligono.vertices[v_d].x,
			poligono.vertices[v_d].y,
			poligono.vertices[v_d].z
		);
	}

	glEnd();
}