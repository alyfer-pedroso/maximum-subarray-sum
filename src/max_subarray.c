#include "max_subarray.h"

#include <stdint.h>

static bool entrada_valida(const int *vetor, size_t n, Resultado *saida) {
    return vetor != NULL && n > 0 && saida != NULL;
}

static void iniciar_resultado(Resultado *resultado) {
    resultado->soma_maxima = INT64_MIN;
    resultado->inicio = 0;
    resultado->fim = 0;
    resultado->iteracoes_externas = 0;
    resultado->subvetores_avaliados = 0;
    resultado->somas = 0;
}

static void registrar_melhor(Resultado *resultado, int64_t soma, size_t inicio,
                             size_t fim) {
    if (soma > resultado->soma_maxima) {
        resultado->soma_maxima = soma;
        resultado->inicio = inicio;
        resultado->fim = fim;
    }
}

bool soma_maxima_cubica(const int *vetor, size_t n, Resultado *saida) {
    if (!entrada_valida(vetor, n, saida)) {
        return false;
    }
    iniciar_resultado(saida);

    for (size_t i = 0; i < n; ++i) {
        ++saida->iteracoes_externas;
        for (size_t j = i; j < n; ++j) {
            ++saida->subvetores_avaliados;
            int64_t soma = 0;
            for (size_t k = i; k <= j; ++k) {
                soma += vetor[k];
                ++saida->somas;
            }
            registrar_melhor(saida, soma, i, j);
        }
    }
    return true;
}

bool soma_maxima_quadratica(const int *vetor, size_t n, Resultado *saida) {
    if (!entrada_valida(vetor, n, saida)) {
        return false;
    }
    iniciar_resultado(saida);

    for (size_t i = 0; i < n; ++i) {
        ++saida->iteracoes_externas;
        int64_t soma = 0;
        for (size_t j = i; j < n; ++j) {
            ++saida->subvetores_avaliados;
            soma += vetor[j];
            ++saida->somas;
            registrar_melhor(saida, soma, i, j);
        }
    }
    return true;
}
