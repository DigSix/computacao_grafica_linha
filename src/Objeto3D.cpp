#include "Objeto3D.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <unordered_set>
#include <GL/freeglut.h>

namespace {

using ChaveAresta = std::pair<unsigned int, unsigned int>;

struct HashAresta {
    std::size_t operator()(const ChaveAresta& aresta) const {
        return (static_cast<std::size_t>(aresta.first) << 32) ^ aresta.second;
    }
};

unsigned int indiceObj(const std::string& token, std::size_t quantidadeVertices) {
    const std::size_t separador = token.find('/');
    const std::string indice = token.substr(0, separador);
    const int valor = std::stoi(indice);

    if (valor == 0) {
        throw std::runtime_error("Indice de vertice OBJ invalido: 0");
    }

    if (valor > 0) {
        const std::size_t indiceConvertido = static_cast<std::size_t>(valor - 1);
        if (indiceConvertido >= quantidadeVertices) {
            throw std::runtime_error("Face OBJ referencia vertice inexistente");
        }
        return static_cast<unsigned int>(indiceConvertido);
    }

    const int indiceRelativo = static_cast<int>(quantidadeVertices) + valor;
    if (indiceRelativo < 0) {
        throw std::runtime_error("Indice relativo de vertice OBJ invalido");
    }
    return static_cast<unsigned int>(indiceRelativo);
}

} // namespace

Objeto3D::Objeto3D(TipoGeometria tipo, float tamanho) {
    if (tipo == TipoGeometria::Cubo) {
        criarCubo(tamanho);
    } else {
        criarPiramide(tamanho);
    }
}

Objeto3D::Objeto3D(const std::string& caminhoObj, float tamanho) {
    carregarObj(caminhoObj, tamanho);
}

void Objeto3D::criarCubo(float tamanho) {
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

void Objeto3D::criarPiramide(float tamanho) {
    const float metade = tamanho / 2.0f;
    const float altura = tamanho;

    vertices = {
        {-metade, 0.0f, -metade},
        { metade, 0.0f, -metade},
        { metade, 0.0f,  metade},
        {-metade, 0.0f,  metade},
        {0.0f, altura, 0.0f}
    };

    arestas = {
        {0, 1}, {1, 2}, {2, 3}, {3, 0},
        {0, 4}, {1, 4}, {2, 4}, {3, 4}
    };
}

void Objeto3D::carregarObj(const std::string& caminhoObj, float tamanho) {
    std::ifstream arquivo(caminhoObj);
    if (!arquivo.is_open()) {
        throw std::runtime_error("Nao foi possivel abrir o arquivo OBJ: " + caminhoObj);
    }

    std::unordered_set<ChaveAresta, HashAresta> arestasEncontradas;
    std::string linha;
    unsigned int numeroLinha = 0;

    while (std::getline(arquivo, linha)) {
        ++numeroLinha;

        const std::size_t inicioConteudo = linha.find_first_not_of(" \t\r");
        if (inicioConteudo == std::string::npos || linha[inicioConteudo] == '#') {
            continue;
        }

        std::istringstream stream(linha.substr(inicioConteudo));
        std::string tipo;
        stream >> tipo;

        if (tipo == "v") {
            Vetor3 vertice{};
            if (!(stream >> vertice.x >> vertice.y >> vertice.z)) {
                throw std::runtime_error("Vertice OBJ invalido na linha " + std::to_string(numeroLinha));
            }
            vertices.push_back(vertice);
            continue;
        }

        if (tipo != "f") {
            continue;
        }

        std::vector<unsigned int> face;
        std::string token;
        while (stream >> token) {
            face.push_back(indiceObj(token, vertices.size()));
        }

        if (face.size() < 3) {
            throw std::runtime_error("Face OBJ com menos de 3 vertices na linha " + std::to_string(numeroLinha));
        }

        // Uma face com n vertices vira n - 2 triangulos em leque.
        for (std::size_t i = 1; i + 1 < face.size(); ++i) {
            const unsigned int triangulo[] = {face[0], face[i], face[i + 1]};
            for (int lado = 0; lado < 3; ++lado) {
                unsigned int inicio = triangulo[lado];
                unsigned int fim = triangulo[(lado + 1) % 3];
                if (inicio > fim) {
                    std::swap(inicio, fim);
                }
                if (arestasEncontradas.insert({inicio, fim}).second) {
                    arestas.push_back({inicio, fim});
                }
            }
        }
    }

    if (vertices.empty() || arestas.empty()) {
        throw std::runtime_error("O arquivo OBJ nao possui vertices e faces validos: " + caminhoObj);
    }

    normalizar(tamanho);
}

void Objeto3D::normalizar(float tamanho) {
    Vetor3 minimo = vertices.front();
    Vetor3 maximo = vertices.front();

    for (const Vetor3& vertice : vertices) {
        minimo.x = std::min(minimo.x, vertice.x);
        minimo.y = std::min(minimo.y, vertice.y);
        minimo.z = std::min(minimo.z, vertice.z);
        maximo.x = std::max(maximo.x, vertice.x);
        maximo.y = std::max(maximo.y, vertice.y);
        maximo.z = std::max(maximo.z, vertice.z);
    }

    const Vetor3 centro{
        (minimo.x + maximo.x) / 2.0f,
        (minimo.y + maximo.y) / 2.0f,
        (minimo.z + maximo.z) / 2.0f
    };

    const float maiorDimensao = std::max({
        maximo.x - minimo.x,
        maximo.y - minimo.y,
        maximo.z - minimo.z
    });
    const float fator = maiorDimensao > 0.0f ? tamanho / maiorDimensao : 1.0f;

    for (Vetor3& vertice : vertices) {
        vertice.x = (vertice.x - centro.x) * fator;
        vertice.y = (vertice.y - centro.y) * fator;
        vertice.z = (vertice.z - centro.z) * fator;
    }
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
