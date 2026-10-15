#include "Objeto3D.hpp"

#include <GL/freeglut.h>

Objeto3D::Objeto3D(float tamanho) {
    const float metade = tamanho / 2.0f;

    vertices = {
        {-metade, -metade, -metade},
        { metade, -metade, -metade},
        { metade,  metade, -metade},
        {-metade,  metade, -metade},
        {-metade, -metade,  metade},
        { metade, -metade,  metade},
        { metade,  metade,  metade},
        {-metade,  metade,  metade}
    };

    arestas = {
        {0, 1}, {1, 2}, {2, 3}, {3, 0},
        {4, 5}, {5, 6}, {6, 7}, {7, 4},
        {0, 4}, {1, 5}, {2, 6}, {3, 7}
    };
}

void Objeto3D::desenhar() const {
    glPushMatrix();
    glTranslatef(transformacao.posicao.x, transformacao.posicao.y, transformacao.posicao.z);
    glRotatef(transformacao.rotacao.x, 1.0f, 0.0f, 0.0f);
    glRotatef(transformacao.rotacao.y, 0.0f, 1.0f, 0.0f);
    glRotatef(transformacao.rotacao.z, 0.0f, 0.0f, 1.0f);
    glScalef(transformacao.escala.x, transformacao.escala.y, transformacao.escala.z);

    glBegin(GL_LINES);
    for (const Aresta& aresta : arestas) {
        const Vetor3& inicio = vertices[aresta.inicio];
        const Vetor3& fim = vertices[aresta.fim];
        glVertex3f(inicio.x, inicio.y, inicio.z);
        glVertex3f(fim.x, fim.y, fim.z);
    }
    glEnd();
    glPopMatrix();
}

void Objeto3D::transladar(float x, float y, float z) {
    transformacao.posicao.x += x;
    transformacao.posicao.y += y;
    transformacao.posicao.z += z;
}

void Objeto3D::rotacionar(float x, float y, float z) {
    transformacao.rotacao.x += x;
    transformacao.rotacao.y += y;
    transformacao.rotacao.z += z;
}

void Objeto3D::escalar(float fator) {
    const float novaEscala = transformacao.escala.x * fator;
    if (novaEscala < 0.1f || novaEscala > 10.0f) {
        return;
    }
    transformacao.escala = {novaEscala, novaEscala, novaEscala};
}

void Objeto3D::resetarTransformacao() {
    transformacao = {};
    transformacao.escala = {1.0f, 1.0f, 1.0f};
}
