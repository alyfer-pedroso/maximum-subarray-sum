#include "max_subarray.h"

#include <assert.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

static void testar(const char *nome, const int *vetor, size_t n, int64_t esperado,
                   size_t inicio_esperado, size_t fim_esperado) {
    Resultado cubico;
    Resultado quadratico;
    assert(soma_maxima_cubica(vetor, n, &cubico));
    assert(soma_maxima_quadratica(vetor, n, &quadratico));
    assert(cubico.soma_maxima == esperado);
    assert(cubico.inicio == inicio_esperado);
    assert(cubico.fim == fim_esperado);
    assert(quadratico.soma_maxima == cubico.soma_maxima);
    assert(quadratico.inicio == cubico.inicio);
    assert(quadratico.fim == cubico.fim);
    assert(cubico.subvetores_avaliados == n * (n + 1u) / 2u);
    assert(quadratico.subvetores_avaliados == cubico.subvetores_avaliados);
    assert(quadratico.somas == n * (n + 1u) / 2u);
    assert(cubico.somas == n * (n + 1u) * (n + 2u) / 6u);
    printf("  [OK] %-22s | n=%-3zu | soma=%" PRId64 " | intervalo=[%zu, %zu]\n",
           nome, n, esperado, inicio_esperado, fim_esperado);
}

int main(void) {
    const int classico[] = {-2, 1, -3, 4, -1, 2, 1, -5, 4};
    const int negativos[] = {-8, -3, -6, -2, -5, -4};
    const int positivos[] = {1, 2, 3, 4};
    const int zeros[] = {0, 0, 0};
    const int unitario[] = {-7};
    const int empate[] = {2, -2, 2};
    const int misto[] = {-1, 3, -2, 5, -9, 4};
    const int longo[] = {1000000000, 1000000000, 1000000000};
    Resultado resultado;

    puts("\n===============================================================");
    puts("                  PROJETO 10 | TESTES");
    puts("===============================================================");
    testar("Exemplo classico", classico, 9, 6, 3, 6);
    testar("Todos negativos", negativos, 6, -2, 3, 3);
    testar("Todos positivos", positivos, 4, 10, 0, 3);
    testar("Vetor de zeros", zeros, 3, 0, 0, 0);
    testar("Um elemento", unitario, 1, -7, 0, 0);
    testar("Empate", empate, 3, 2, 0, 0);
    testar("Valores mistos", misto, 6, 6, 1, 3);
    testar("Soma acima de INT_MAX", longo, 3, INT64_C(3000000000), 0, 2);

    assert(!soma_maxima_cubica(NULL, 4, &resultado));
    assert(!soma_maxima_quadratica(NULL, 4, &resultado));
    assert(!soma_maxima_cubica(classico, 0, &resultado));
    assert(!soma_maxima_quadratica(classico, 0, &resultado));
    assert(!soma_maxima_cubica(classico, 9, NULL));
    assert(!soma_maxima_quadratica(classico, 9, NULL));
    puts("  [OK] Argumentos invalidos rejeitados (6 verificacoes).");
    puts("---------------------------------------------------------------");
    puts("  RESULTADO: 8/8 cenarios aprovados. Todos os testes passaram.");
    puts("===============================================================\n");
    return 0;
}
