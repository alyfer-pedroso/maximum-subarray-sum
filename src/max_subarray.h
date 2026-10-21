#ifndef MAX_SUBARRAY_H
#define MAX_SUBARRAY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    int64_t soma_maxima;
    size_t inicio;
    size_t fim;
    uint64_t iteracoes_externas;
    uint64_t subvetores_avaliados;
    uint64_t somas;
} Resultado;

bool soma_maxima_cubica(const int *vetor, size_t n, Resultado *saida);
bool soma_maxima_quadratica(const int *vetor, size_t n, Resultado *saida);

#endif
