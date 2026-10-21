#if !defined(_WIN32)
#define _POSIX_C_SOURCE 200809L
#endif

#include "max_subarray.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#endif

#define AMOSTRAS 5
#define ALVO_SEGUNDOS 0.06
#define MAX_REPETICOES 10000000
#define QTD_TAMANHOS 4

static volatile uint64_t barreira_de_otimizacao = 0;

typedef struct {
    size_t n;
    Resultado cubico;
    Resultado quadratico;
    int repeticoes_cubico;
    int repeticoes_quadratico;
    double tempo_cubico_ms;
    double tempo_quadratico_ms;
} Experimento;

static uint32_t aleatorio(uint32_t *estado) {
    uint32_t x = *estado;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *estado = x;
    return x;
}

static void preencher_vetor(int *vetor, size_t n, uint32_t semente) {
    uint32_t estado = semente;
    for (size_t i = 0; i < n; ++i) {
        vetor[i] = (int)(aleatorio(&estado) % 201u) - 100;
    }
}

static double mediana(const double tempos[AMOSTRAS]) {
    double copias[AMOSTRAS];
    for (int i = 0; i < AMOSTRAS; ++i) copias[i] = tempos[i];
    for (int i = 1; i < AMOSTRAS; ++i) {
        double atual = copias[i];
        int j = i - 1;
        while (j >= 0 && copias[j] > atual) {
            copias[j + 1] = copias[j];
            --j;
        }
        copias[j + 1] = atual;
    }
    return copias[AMOSTRAS / 2];
}

static double agora_segundos(void) {
#ifdef _WIN32
    LARGE_INTEGER frequencia;
    LARGE_INTEGER contador;
    if (!QueryPerformanceFrequency(&frequencia) || !QueryPerformanceCounter(&contador)) {
        fputs("Relogio de alta resolucao indisponivel.\n", stderr);
        exit(EXIT_FAILURE);
    }
    return (double)contador.QuadPart / (double)frequencia.QuadPart;
#else
    struct timespec instante;
    if (clock_gettime(CLOCK_MONOTONIC, &instante) != 0) {
        perror("clock_gettime");
        exit(EXIT_FAILURE);
    }
    return (double)instante.tv_sec + (double)instante.tv_nsec / 1000000000.0;
#endif
}

static void executar_repeticoes(bool (*algoritmo)(const int *, size_t, Resultado *),
                               const int *vetor, size_t n, int repeticoes, Resultado *saida) {
    for (int r = 0; r < repeticoes; ++r) {
        Resultado resultado;
        if (!algoritmo(vetor, n, &resultado)) {
            fputs("Erro ao executar o algoritmo.\n", stderr);
            exit(EXIT_FAILURE);
        }
        barreira_de_otimizacao ^= (uint64_t)resultado.soma_maxima + resultado.somas;
        *saida = resultado;
    }
}

static double medir(bool (*algoritmo)(const int *, size_t, Resultado *),
                    const int *vetor, size_t n, int repeticoes_minimas,
                    int *repeticoes_efetivas, Resultado *saida) {
    int repeticoes = repeticoes_minimas;
    double tempos[AMOSTRAS];

    /* Calibra separadamente cada algoritmo para evitar amostras curtas demais. */
    for (;;) {
        double inicio = agora_segundos();
        executar_repeticoes(algoritmo, vetor, n, repeticoes, saida);
        double duracao = agora_segundos() - inicio;
        if (duracao >= ALVO_SEGUNDOS || repeticoes >= MAX_REPETICOES) break;

        double fator = duracao > 0.0 ? ALVO_SEGUNDOS / duracao : 10.0;
        if (fator < 2.0) fator = 2.0;
        if (fator > 10.0) fator = 10.0;
        int proximo = (int)((double)repeticoes * fator);
        if (proximo <= repeticoes) proximo = repeticoes + 1;
        if (proximo > MAX_REPETICOES) proximo = MAX_REPETICOES;
        repeticoes = proximo;
    }

    for (int amostra = 0; amostra < AMOSTRAS; ++amostra) {
        double inicio = agora_segundos();
        executar_repeticoes(algoritmo, vetor, n, repeticoes, saida);
        tempos[amostra] = (agora_segundos() - inicio) * 1000.0 / (double)repeticoes;
    }
    *repeticoes_efetivas = repeticoes;
    return mediana(tempos);
}

static void formatar_milhar(uint64_t valor, char *destino, size_t capacidade) {
    char digitos[32];
    snprintf(digitos, sizeof(digitos), "%" PRIu64, valor);
    size_t tamanho = strlen(digitos);
    size_t posicao = 0;
    for (size_t i = 0; i < tamanho && posicao + 1 < capacidade; ++i) {
        if (i > 0 && (tamanho - i) % 3 == 0 && posicao + 2 < capacidade) {
            destino[posicao++] = '.';
        }
        destino[posicao++] = digitos[i];
    }
    if (capacidade > 0) destino[posicao] = '\0';
}

static void formatar_soma(int64_t valor, char *destino, size_t capacidade) {
    if (valor >= 0) {
        formatar_milhar((uint64_t)valor, destino, capacidade);
        return;
    }
    char magnitude[32];
    formatar_milhar((uint64_t)(-(valor + 1)) + 1u, magnitude, sizeof(magnitude));
    snprintf(destino, capacidade, "-%s", magnitude);
}

static void formatar_tempo(double milissegundos, char *destino, size_t capacidade) {
    if (milissegundos >= 1.0) {
        snprintf(destino, capacidade, "%.3f ms", milissegundos);
    } else if (milissegundos >= 0.001) {
        snprintf(destino, capacidade, "%.2f us", milissegundos * 1000.0);
    } else {
        snprintf(destino, capacidade, "%.1f ns", milissegundos * 1000000.0);
    }
    for (char *p = destino; *p; ++p) {
        if (*p == '.') *p = ',';
    }
}

static void exibir_tabela(const Experimento *experimentos, size_t quantidade,
                         uint32_t semente) {
    puts("\n===========================================================================================");
    puts("                        PROJETO 10 | SOMA MAXIMA DE SUBVETOR");
    puts("===========================================================================================");
    puts("  Forca bruta O(n^3)  versus  Soma acumulada O(n^2)");
    printf("  Semente: %" PRIu32 " | Amostras: %d por algoritmo | Valores: [-100, 100]\n",
           semente, AMOSTRAS);

    puts("\n  1. RESULTADOS E CONTAGEM DE OPERACOES");
    puts("-------------------------------------------------------------------------------------------");
    printf("  %-5s %11s %13s %13s %14s %14s %10s\n",
           "n", "Soma maxima", "Intervalo", "Subvetores", "Somas O(n^3)",
           "Somas O(n^2)", "Reducao");
    puts("-------------------------------------------------------------------------------------------");
    for (size_t i = 0; i < quantidade; ++i) {
        const Experimento *e = &experimentos[i];
        char soma[64], subvetores[32], somas_c[32], somas_q[32];
        char intervalo[32], reducao[32];
        formatar_soma(e->cubico.soma_maxima, soma, sizeof(soma));
        formatar_milhar(e->cubico.subvetores_avaliados, subvetores, sizeof(subvetores));
        formatar_milhar(e->cubico.somas, somas_c, sizeof(somas_c));
        formatar_milhar(e->quadratico.somas, somas_q, sizeof(somas_q));
        snprintf(intervalo, sizeof(intervalo), "[%zu, %zu]", e->cubico.inicio, e->cubico.fim);
        snprintf(reducao, sizeof(reducao), "%.2fx",
                 (double)e->cubico.somas / (double)e->quadratico.somas);
        for (char *p = reducao; *p; ++p) if (*p == '.') *p = ',';
        printf("  %-5zu %11s %13s %13s %14s %14s %10s\n",
               e->n, soma, intervalo, subvetores, somas_c, somas_q, reducao);
    }
    puts("-------------------------------------------------------------------------------------------");

    puts("\n  2. TEMPO POR EXECUCAO (MEDIANA)");
    puts("-------------------------------------------------------------------------------------------");
    printf("  %-5s %16s %16s %12s %16s %16s\n", "n", "Cubico", "Quadratico",
           "Ganho", "Repet. cubico", "Repet. quadr.");
    puts("-------------------------------------------------------------------------------------------");
    for (size_t i = 0; i < quantidade; ++i) {
        const Experimento *e = &experimentos[i];
        char tempo_c[32], tempo_q[32], ganho[32], repet_c[32], repet_q[32];
        formatar_tempo(e->tempo_cubico_ms, tempo_c, sizeof(tempo_c));
        formatar_tempo(e->tempo_quadratico_ms, tempo_q, sizeof(tempo_q));
        formatar_milhar((uint64_t)e->repeticoes_cubico, repet_c, sizeof(repet_c));
        formatar_milhar((uint64_t)e->repeticoes_quadratico, repet_q, sizeof(repet_q));
        if (e->tempo_quadratico_ms > 0.0) {
            snprintf(ganho, sizeof(ganho), "%.2fx", e->tempo_cubico_ms / e->tempo_quadratico_ms);
            for (char *p = ganho; *p; ++p) if (*p == '.') *p = ',';
        } else {
            snprintf(ganho, sizeof(ganho), "n/d");
        }
        printf("  %-5zu %16s %16s %12s %16s %16s\n",
               e->n, tempo_c, tempo_q, ganho, repet_c, repet_q);
    }
    puts("-------------------------------------------------------------------------------------------");
    puts("  Nota: tempos variam conforme CPU, sistema e carga; 'us' = microssegundos.");
    puts("  Repeticoes calibradas por algoritmo para reduzir erros de resolucao do relogio.");

    puts("\n  3. VALIDACAO");
    printf("  [OK] %zu/%zu cenarios: mesma soma maxima e mesmo intervalo nos dois algoritmos.\n",
           quantidade, quantidade);
    puts("  [OK] Contadores de somas conforme as formulas O(n^3) e O(n^2).");
}

static void escrever_cabecalho_csv(FILE *arquivo) {
    fprintf(arquivo,
            "n,semente,repeticoes_por_amostra,amostras,soma_maxima,inicio,fim,"
            "iteracoes_laco_i,iteracoes_laco_j,subvetores,cubico_somas,quadratico_somas,razao_somas,"
            "cubico_tempo_ms,quadratico_tempo_ms,razao_tempo,quadratico_repeticoes_por_amostra\n");
}

static void escrever_experimento_csv(FILE *arquivo, const Experimento *e, uint32_t semente) {
    fprintf(arquivo,
            "%zu,%" PRIu32 ",%d,%d,%" PRId64 ",%zu,%zu,%" PRIu64
            ",%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%.6f,%.6f,%.6f,%.6f,%d\n",
            e->n, semente, e->repeticoes_cubico, AMOSTRAS, e->cubico.soma_maxima,
            e->cubico.inicio, e->cubico.fim, e->cubico.iteracoes_externas,
            e->cubico.subvetores_avaliados, e->cubico.subvetores_avaliados,
            e->cubico.somas, e->quadratico.somas,
            (double)e->cubico.somas / (double)e->quadratico.somas,
            e->tempo_cubico_ms, e->tempo_quadratico_ms,
            e->tempo_quadratico_ms > 0.0 ? e->tempo_cubico_ms / e->tempo_quadratico_ms : 0.0,
            e->repeticoes_quadratico);
}

int main(int argc, char **argv) {
    const size_t tamanhos[QTD_TAMANHOS] = {50u, 100u, 300u, 500u};
    const int repeticoes_minimas[QTD_TAMANHOS] = {1000, 250, 20, 5};
    const uint32_t semente = 20261020u;
    Experimento experimentos[QTD_TAMANHOS] = {0};
    FILE *arquivo = NULL;
    const char *caminho_csv = NULL;
    bool apenas_csv = false;

    if (argc > 2) {
        fprintf(stderr, "Uso: %s [arquivo.csv | --csv | --help]\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (argc == 2) {
        if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
            printf("Uso: %s [arquivo.csv | --csv | --help]\n", argv[0]);
            puts("  sem argumentos: mostra tabelas formatadas no terminal");
            puts("  arquivo.csv: mostra tabelas e salva os resultados em CSV");
            puts("  --csv: imprime somente o CSV no terminal (para redirecionamento)");
            return EXIT_SUCCESS;
        }
        if (strcmp(argv[1], "--csv") == 0) {
            apenas_csv = true;
            arquivo = stdout;
        } else {
            caminho_csv = argv[1];
            arquivo = fopen(caminho_csv, "w");
            if (!arquivo) {
                perror("Nao foi possivel abrir o CSV");
                return EXIT_FAILURE;
            }
        }
    }
    if (arquivo != NULL) escrever_cabecalho_csv(arquivo);

    for (size_t t = 0; t < QTD_TAMANHOS; ++t) {
        size_t n = tamanhos[t];
        int *vetor = malloc(n * sizeof(*vetor));
        if (!vetor) {
            fputs("Memoria insuficiente.\n", stderr);
            if (arquivo != NULL && arquivo != stdout) fclose(arquivo);
            return EXIT_FAILURE;
        }
        preencher_vetor(vetor, n, semente);

        Experimento *e = &experimentos[t];
        e->n = n;
        e->tempo_cubico_ms = medir(soma_maxima_cubica, vetor, n, repeticoes_minimas[t],
                                   &e->repeticoes_cubico, &e->cubico);
        e->tempo_quadratico_ms = medir(soma_maxima_quadratica, vetor, n, repeticoes_minimas[t],
                                      &e->repeticoes_quadratico, &e->quadratico);

        if (e->cubico.soma_maxima != e->quadratico.soma_maxima ||
            e->cubico.inicio != e->quadratico.inicio || e->cubico.fim != e->quadratico.fim) {
            fprintf(stderr, "Divergencia entre algoritmos para n=%zu\n", n);
            free(vetor);
            if (arquivo != NULL && arquivo != stdout) fclose(arquivo);
            return EXIT_FAILURE;
        }
        const uint64_t subvetores = (uint64_t)n * (n + 1u) / 2u;
        const uint64_t somas_cubicas = (uint64_t)n * (n + 1u) * (n + 2u) / 6u;
        if (e->cubico.subvetores_avaliados != subvetores ||
            e->quadratico.subvetores_avaliados != subvetores ||
            e->cubico.somas != somas_cubicas || e->quadratico.somas != subvetores) {
            fprintf(stderr, "Contagens de operacoes incorretas para n=%zu\n", n);
            free(vetor);
            if (arquivo != NULL && arquivo != stdout) fclose(arquivo);
            return EXIT_FAILURE;
        }
        if (arquivo != NULL) escrever_experimento_csv(arquivo, e, semente);
        free(vetor);
    }

    if (arquivo != NULL && arquivo != stdout && fclose(arquivo) != 0) {
        perror("Erro ao fechar o arquivo CSV");
        return EXIT_FAILURE;
    }

    if (!apenas_csv) {
        exibir_tabela(experimentos, QTD_TAMANHOS, semente);
        if (caminho_csv) printf("  [OK] CSV salvo em: %s\n", caminho_csv);
        else puts("  [INFO] CSV nao gerado. Para salvar, passe um caminho como argumento.");
        puts("===========================================================================================\n");
    }
    return EXIT_SUCCESS;
}
