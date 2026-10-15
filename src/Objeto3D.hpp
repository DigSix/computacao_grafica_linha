#pragma once

#include <string>
#include <vector>

// Representa uma posição ou direção no espaço 3D.
struct Vetor3 {
    float x; // Coordenada no eixo horizontal.
    float y; // Coordenada no eixo vertical.
    float z; // Coordenada de profundidade.
};

// Estado separado da geometria: os vértices não precisam ser alterados
// toda vez que o usuário move, gira ou escala o objeto.
struct Transformacao {
    Vetor3 posicao{0.0f, 0.0f, 0.0f}; // Deslocamento do objeto.
    Vetor3 rotacao{0.0f, 0.0f, 0.0f}; // Ângulo em graus em cada eixo.
    Vetor3 escala{1.0f, 1.0f, 1.0f};  // Tamanho relativo em cada eixo.
};

// Primitivas disponíveis sem a necessidade de um arquivo OBJ.
enum class TipoGeometria {
    Cubo,
    Piramide
};

class Objeto3D {
public:
    // Cria uma primitiva, como cubo ou pirâmide.
    explicit Objeto3D(TipoGeometria tipo, float tamanho = 2.0f);

    // Carrega somente a geometria do arquivo nesta etapa: v e f.
    explicit Objeto3D(const std::string& caminhoObj, float tamanho = 2.0f);

    // Envia o wireframe para o pipeline fixo do OpenGL.
    void desenhar() const;

    // Altera a posição sem modificar os vértices originais.
    void transladar(float x, float y, float z);

    // Soma ângulos aos três eixos de rotação.
    void rotacionar(float x, float y, float z);

    // Aplica uma escala uniforme ao objeto.
    void escalar(float fator);

    // Retorna posição, rotação e escala aos valores iniciais.
    void resetarTransformacao();

private:
    // Uma face OBJ é convertida em arestas para o desenho wireframe.
    struct Aresta {
        unsigned int inicio; // Índice do primeiro vértice.
        unsigned int fim;    // Índice do segundo vértice.
    };

    std::vector<Vetor3> vertices;
    std::vector<Aresta> arestas;
    Transformacao transformacao;

    void criarCubo(float tamanho);
    void criarPiramide(float tamanho);
    void carregarObj(const std::string& caminhoObj, float tamanho);
    void normalizar(float tamanho);
};
