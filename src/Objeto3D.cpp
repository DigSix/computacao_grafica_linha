#include "Objeto3D.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <utility>
#include <unordered_set>
#include <GL/freeglut.h>

namespace {

// A mesma aresta pode aparecer em duas faces. O conjunto abaixo permite
// desenhá-la apenas uma vez no wireframe.
using ChaveAresta = std::pair<unsigned int, unsigned int>;

struct HashAresta {
    std::size_t operator()(const ChaveAresta& aresta) const {
        // Combina os dois índices em um valor usado pelo unordered_set.
        return (static_cast<std::size_t>(aresta.first) << 32) ^ aresta.second;
    }
};

// Um token de face pode ser "3", "3/2" ou "3/2/1".
// Nesta etapa só usamos o primeiro índice, que aponta para a posição.
unsigned int indiceObj(const std::string& token, std::size_t quantidadeVertices) {
    // A parte antes da primeira barra é o índice da posição. As demais partes
    // representam textura e normal, que serão aproveitadas em outra etapa.
    const std::size_t separador = token.find('/');
    const std::string indice = token.substr(0, separador);
    const int valor = std::stoi(indice);

    if (valor == 0) {
        throw std::runtime_error("Indice de vertice OBJ invalido: 0");
    }

    if (valor > 0) {
        // Índices positivos do OBJ começam em 1, por isso subtraímos 1.
        const std::size_t indiceConvertido = static_cast<std::size_t>(valor - 1);
        if (indiceConvertido >= quantidadeVertices) {
            throw std::runtime_error("Face OBJ referencia vertice inexistente");
        }
        return static_cast<unsigned int>(indiceConvertido);
    }

    // O OBJ também permite índices negativos, relativos ao último vértice.
    const int indiceRelativo = static_cast<int>(quantidadeVertices) + valor;
    if (indiceRelativo < 0) {
        throw std::runtime_error("Indice relativo de vertice OBJ invalido");
    }
    return static_cast<unsigned int>(indiceRelativo);
}

} // namespace

Objeto3D::Objeto3D(TipoGeometria tipo, float tamanho) {
    // O construtor delega a criação para a primitiva escolhida.
    if (tipo == TipoGeometria::Cubo) {
        criarCubo(tamanho);
    } else {
        criarPiramide(tamanho);
    }
}

Objeto3D::Objeto3D(const std::string& caminhoObj, float tamanho) {
    // O arquivo é lido no construtor para que o objeto já esteja pronto
    // quando a cena começar a desenhar.
    carregarObj(caminhoObj, tamanho);
}

void Objeto3D::criarCubo(float tamanho) {
    // O tamanho é dividido pela metade porque o cubo é criado ao redor da
    // origem: de -metade até +metade em cada eixo.
    const float metade = tamanho / 2.0f;

    // Os vértices são centralizados na origem. Isso facilita a aplicação
    // de rotação e escala usando as matrizes do OpenGL.
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

    // Quatro arestas na frente, quatro atrás e quatro ligando as duas faces.
    arestas = {
        {0, 1}, {1, 2}, {2, 3}, {3, 0},
        {4, 5}, {5, 6}, {6, 7}, {7, 4},
        {0, 4}, {1, 5}, {2, 6}, {3, 7}
    };
}

void Objeto3D::criarPiramide(float tamanho) {
    // A pirâmide possui quatro vértices na base e um vértice no topo.
    const float metade = tamanho / 2.0f;
    const float altura = tamanho;

    // A base fica no plano Y = 0 e o quinto vértice é o topo.
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
    // ifstream lê o arquivo OBJ como texto, uma linha de cada vez.
    std::ifstream arquivo(caminhoObj);
    if (!arquivo.is_open()) {
        throw std::runtime_error("Nao foi possivel abrir o arquivo OBJ: " + caminhoObj);
    }

    std::unordered_set<ChaveAresta, HashAresta> arestasEncontradas;
    std::string linha;
    unsigned int numeroLinha = 0;

    // O formato OBJ é textual e organizado por linhas. Nesta versão,
    // linhas v definem posições e linhas f definem faces.
    while (std::getline(arquivo, linha)) {
        ++numeroLinha;

        // Ignora espaços no começo, linhas vazias e comentários do OBJ.
        const std::size_t inicioConteudo = linha.find_first_not_of(" \t\r");
        if (inicioConteudo == std::string::npos || linha[inicioConteudo] == '#') {
            continue;
        }

        // istringstream permite separar o comando e seus valores por espaço.
        std::istringstream stream(linha.substr(inicioConteudo));
        std::string tipo;
        stream >> tipo;

        if (tipo == "v") {
            // Exemplo: v 1.0 2.0 3.0
            Vetor3 vertice{};
            if (!(stream >> vertice.x >> vertice.y >> vertice.z)) {
                throw std::runtime_error("Vertice OBJ invalido na linha " + std::to_string(numeroLinha));
            }
            vertices.push_back(vertice); // Guarda a posição para uso pelas faces.
            continue;
        }

        if (tipo != "f") {
            // vt, vn, mtllib, g e outros comandos serão usados em etapas
            // posteriores da M2.
            continue;
        }

        // Uma face guarda os índices dos seus vértices. O OBJ começa os
        // índices em 1; indiceObj converte para o padrão do C++, que começa 0.
        std::vector<unsigned int> face;
        std::string token;
        while (stream >> token) {
            face.push_back(indiceObj(token, vertices.size()));
        }

        if (face.size() < 3) {
            throw std::runtime_error("Face OBJ com menos de 3 vertices na linha " + std::to_string(numeroLinha));
        }

        // Uma face com n vértices vira n - 2 triângulos em leque.
        // Exemplo: 1 2 3 4 vira os triângulos 1-2-3 e 1-3-4.
        for (std::size_t i = 1; i + 1 < face.size(); ++i) {
            const unsigned int triangulo[] = {face[0], face[i], face[i + 1]};
            for (int lado = 0; lado < 3; ++lado) {
                // Cada lado do triângulo vira uma aresta do wireframe.
                unsigned int inicio = triangulo[lado];
                unsigned int fim = triangulo[(lado + 1) % 3];
                if (inicio > fim) {
                    // Ordenar o par faz (1, 4) e (4, 1) representarem a mesma
                    // aresta no conjunto de controle.
                    std::swap(inicio, fim);
                }
                if (arestasEncontradas.insert({inicio, fim}).second) {
                    // insert().second é true somente quando a aresta é nova.
                    arestas.push_back({inicio, fim});
                }
            }
        }
    }

    if (vertices.empty() || arestas.empty()) {
        throw std::runtime_error("O arquivo OBJ nao possui vertices e faces validos: " + caminhoObj);
    }

    // Coloca modelos de escalas diferentes em um tamanho comum para a câmera.
    normalizar(tamanho);
}

void Objeto3D::normalizar(float tamanho) {
    // Arquivos OBJ podem usar escalas e posições muito diferentes. O bounding
    // box permite centralizar o modelo e colocá-lo dentro da área da câmera.
    Vetor3 minimo = vertices.front();
    Vetor3 maximo = vertices.front();

    for (const Vetor3& vertice : vertices) {
        // Calcula os extremos do modelo nos três eixos.
        minimo.x = std::min(minimo.x, vertice.x);
        minimo.y = std::min(minimo.y, vertice.y);
        minimo.z = std::min(minimo.z, vertice.z);
        maximo.x = std::max(maximo.x, vertice.x);
        maximo.y = std::max(maximo.y, vertice.y);
        maximo.z = std::max(maximo.z, vertice.z);
    }

    // O centro do bounding box será levado para a origem.
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
        // Primeiro centraliza; depois aplica a mesma escala nos três eixos.
        vertice.x = (vertice.x - centro.x) * fator;
        vertice.y = (vertice.y - centro.y) * fator;
        vertice.z = (vertice.z - centro.z) * fator;
    }
}

void Objeto3D::desenhar() const {
    // Salva a matriz da cena para que a transformação deste objeto não afete
    // outros objetos desenhados depois dele.
    glPushMatrix();
    // A ordem das transformações influencia o resultado final.
    glTranslatef(transformacao.posicao.x, transformacao.posicao.y, transformacao.posicao.z);
    // glRotatef recebe graus, ao contrário de sin/cos, que usam radianos.
    glRotatef(transformacao.rotacao.x, 1.0f, 0.0f, 0.0f);
    glRotatef(transformacao.rotacao.y, 0.0f, 1.0f, 0.0f);
    glRotatef(transformacao.rotacao.z, 0.0f, 0.0f, 1.0f);
    glScalef(transformacao.escala.x, transformacao.escala.y, transformacao.escala.z);

    // Por enquanto o modelo é desenhado como wireframe. Na próxima etapa,
    // as faces poderão ser renderizadas preenchidas com GL_TRIANGLES.
    glBegin(GL_LINES);
    for (const Aresta& aresta : arestas) {
        // Cada Aresta fornece dois índices para buscar posições no vetor.
        const Vetor3& inicio = vertices[aresta.inicio];
        const Vetor3& fim = vertices[aresta.fim];
        glVertex3f(inicio.x, inicio.y, inicio.z);
        glVertex3f(fim.x, fim.y, fim.z);
    }
    glEnd();
    glPopMatrix();
}

void Objeto3D::transladar(float x, float y, float z) {
    // Soma o deslocamento recebido ao estado atual do objeto.
    transformacao.posicao.x += x;
    transformacao.posicao.y += y;
    transformacao.posicao.z += z;
}

void Objeto3D::rotacionar(float x, float y, float z) {
    // A rotação é acumulada para que vários eventos tenham efeito contínuo.
    transformacao.rotacao.x += x;
    transformacao.rotacao.y += y;
    transformacao.rotacao.z += z;
}

void Objeto3D::escalar(float fator) {
    // Limita a escala para impedir que o objeto desapareça ou fique enorme.
    const float novaEscala = transformacao.escala.x * fator;
    if (novaEscala < 0.1f || novaEscala > 10.0f) {
        return;
    }
    transformacao.escala = {novaEscala, novaEscala, novaEscala};
}

void Objeto3D::resetarTransformacao() {
    // A atribuição vazia restaura zeros na posição e rotação.
    transformacao = {};
    transformacao.escala = {1.0f, 1.0f, 1.0f};
}
