#pragma once

#include <string>
#include <vector>

struct Vetor3 {
    float x;
    float y;
    float z;
};

struct Transformacao {
    Vetor3 posicao{0.0f, 0.0f, 0.0f};
    Vetor3 rotacao{0.0f, 0.0f, 0.0f};
    Vetor3 escala{1.0f, 1.0f, 1.0f};
};

enum class TipoGeometria {
    Cubo,
    Piramide
};

class Objeto3D {
public:
    explicit Objeto3D(TipoGeometria tipo, float tamanho = 2.0f);
    explicit Objeto3D(const std::string& caminhoObj, float tamanho = 2.0f);

    void desenhar() const;

    void transladar(float x, float y, float z);
    void rotacionar(float x, float y, float z);
    void escalar(float fator);
    void resetarTransformacao();

private:
    struct Aresta {
        unsigned int inicio;
        unsigned int fim;
    };

    std::vector<Vetor3> vertices;
    std::vector<Aresta> arestas;
    Transformacao transformacao;

    void criarCubo(float tamanho);
    void criarPiramide(float tamanho);
    void carregarObj(const std::string& caminhoObj, float tamanho);
    void normalizar(float tamanho);
};
