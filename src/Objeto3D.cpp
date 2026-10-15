#include "Objeto3D.hpp"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <GL/freeglut.h>

namespace {

Vetor3 subtrair(const Vetor3& a, const Vetor3& b) {
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

Vetor3 produtoVetorial(const Vetor3& a, const Vetor3& b) {
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

Vetor3 normalizarVetor(const Vetor3& vetor) {
    const float comprimento = std::sqrt(
        vetor.x * vetor.x + vetor.y * vetor.y + vetor.z * vetor.z
    );

    if (comprimento == 0.0f) {
        return {0.0f, 1.0f, 0.0f};
    }

    return {
        vetor.x / comprimento,
        vetor.y / comprimento,
        vetor.z / comprimento
    };
}

int converterIndice(int indice, std::size_t quantidade) {
    if (indice == 0) {
        throw std::runtime_error("Indice OBJ invalido: 0");
    }

    const int convertido = indice > 0
        ? indice - 1
        : static_cast<int>(quantidade) + indice;

    if (convertido < 0 || convertido >= static_cast<int>(quantidade)) {
        throw std::runtime_error("Indice OBJ fora do intervalo");
    }

    return convertido;
}

struct IndicesObj {
    unsigned int posicao;
    int normal;
};

IndicesObj lerIndicesObj(
    const std::string& token,
    std::size_t quantidadeVertices,
    std::size_t quantidadeNormais
) {
    // Um token pode ser v, v/vt, v//vn ou v/vt/vn.
    const std::size_t primeiraBarra = token.find('/');
    const std::size_t segundaBarra = token.find('/', primeiraBarra + 1);

    const std::string indicePosicao = token.substr(0, primeiraBarra);
    const int posicao = converterIndice(
        std::stoi(indicePosicao), quantidadeVertices
    );

    int normal = -1;
    if (segundaBarra != std::string::npos && segundaBarra + 1 < token.size()) {
        const std::string indiceNormal = token.substr(segundaBarra + 1);
        normal = converterIndice(
            std::stoi(indiceNormal), quantidadeNormais
        );
    }

    return {static_cast<unsigned int>(posicao), normal};
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
        {-metade, -metade, -metade}, { metade, -metade, -metade},
        { metade,  metade, -metade}, {-metade,  metade, -metade},
        {-metade, -metade,  metade}, { metade, -metade,  metade},
        { metade,  metade,  metade}, {-metade,  metade,  metade}
    };

    // Cada quadrado do cubo é dividido em dois triângulos.
    adicionarFace(0, 1, 2);
    adicionarFace(0, 2, 3);
    adicionarFace(4, 6, 5);
    adicionarFace(4, 7, 6);
    adicionarFace(0, 4, 5);
    adicionarFace(0, 5, 1);
    adicionarFace(3, 2, 6);
    adicionarFace(3, 6, 7);
    adicionarFace(0, 3, 7);
    adicionarFace(0, 7, 4);
    adicionarFace(1, 5, 6);
    adicionarFace(1, 6, 2);
}

void Objeto3D::criarPiramide(float tamanho) {
    const float metade = tamanho / 2.0f;

    vertices = {
        {-metade, 0.0f, -metade}, { metade, 0.0f, -metade},
        { metade, 0.0f,  metade}, {-metade, 0.0f,  metade},
        {0.0f, tamanho, 0.0f}
    };

    adicionarFace(0, 1, 2);
    adicionarFace(0, 2, 3);
    adicionarFace(0, 4, 1);
    adicionarFace(1, 4, 2);
    adicionarFace(2, 4, 3);
    adicionarFace(3, 4, 0);
}

void Objeto3D::carregarObj(const std::string& caminhoObj, float tamanho) {
    std::ifstream arquivo(caminhoObj);
    if (!arquivo.is_open()) {
        throw std::runtime_error("Nao foi possivel abrir o arquivo OBJ: " + caminhoObj);
    }

    std::string linha;
    unsigned int numeroLinha = 0;

    while (std::getline(arquivo, linha)) {
        ++numeroLinha;
        const std::size_t inicio = linha.find_first_not_of(" \t\r");

        if (inicio == std::string::npos || linha[inicio] == '#') {
            continue;
        }

        std::istringstream stream(linha.substr(inicio));
        std::string tipo;
        stream >> tipo;

        if (tipo == "v") {
            Vetor3 vertice{};
            if (!(stream >> vertice.x >> vertice.y >> vertice.z)) {
                throw std::runtime_error(
                    "Vertice invalido na linha " + std::to_string(numeroLinha)
                );
            }
            vertices.push_back(vertice);
        } else if (tipo == "vn") {
            Vetor3 normal{};
            if (!(stream >> normal.x >> normal.y >> normal.z)) {
                throw std::runtime_error(
                    "Normal invalida na linha " + std::to_string(numeroLinha)
                );
            }
            normais.push_back(normalizarVetor(normal));
        } else if (tipo == "f") {
            std::vector<IndicesObj> face;
            std::string token;

            while (stream >> token) {
                face.push_back(lerIndicesObj(token, vertices.size(), normais.size()));
            }

            if (face.size() < 3) {
                throw std::runtime_error(
                    "Face invalida na linha " + std::to_string(numeroLinha)
                );
            }

            // Converte uma face com n vértices em n - 2 triângulos.
            for (std::size_t i = 1; i + 1 < face.size(); ++i) {
                adicionarFace(
                    face[0].posicao,
                    face[i].posicao,
                    face[i + 1].posicao,
                    face[0].normal,
                    face[i].normal,
                    face[i + 1].normal
                );
            }
        }
    }

    if (vertices.empty() || faces.empty()) {
        throw std::runtime_error(
            "O arquivo OBJ nao possui vertices e faces validos: " + caminhoObj
        );
    }

    normalizar(tamanho);
}

void Objeto3D::adicionarFace(
    unsigned int a,
    unsigned int b,
    unsigned int c,
    int normalA,
    int normalB,
    int normalC
) {
    faces.push_back({
        {{{a, normalA}, {b, normalB}, {c, normalC}}},
        calcularNormal(a, b, c)
    });
}

Vetor3 Objeto3D::calcularNormal(
    unsigned int a,
    unsigned int b,
    unsigned int c
) const {
    const Vetor3 ladoA = subtrair(vertices[b], vertices[a]);
    const Vetor3 ladoB = subtrair(vertices[c], vertices[a]);
    return normalizarVetor(produtoVetorial(ladoA, ladoB));
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

    // As normais não precisam ser redimensionadas, mas são normalizadas para
    // que o cálculo de iluminação receba vetores unitários.
    for (Vetor3& normal : normais) {
        normal = normalizarVetor(normal);
    }
}

void Objeto3D::desenhar() const {
    glPushMatrix();
    glTranslatef(transformacao.posicao.x, transformacao.posicao.y, transformacao.posicao.z);
    glRotatef(transformacao.rotacao.x, 1.0f, 0.0f, 0.0f);
    glRotatef(transformacao.rotacao.y, 0.0f, 1.0f, 0.0f);
    glRotatef(transformacao.rotacao.z, 0.0f, 0.0f, 1.0f);
    glScalef(transformacao.escala.x, transformacao.escala.y, transformacao.escala.z);

    // Cada vértice é enviado junto com sua normal para o cálculo de luz.
    glBegin(GL_TRIANGLES);
    for (const Face& face : faces) {
        for (const VerticeFace& vertice : face.vertices) {
            const Vetor3& normal = vertice.normal >= 0
                ? normais[vertice.normal]
                : face.normalCalculada;

            glNormal3f(normal.x, normal.y, normal.z);
            const Vetor3& posicao = vertices[vertice.posicao];
            glVertex3f(posicao.x, posicao.y, posicao.z);
        }
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
