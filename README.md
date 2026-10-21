# Projeto 10 — Soma Máxima de Subvetor

**Disciplina:** Computabilidade e Complexidade de Algoritmos
**Linguagem:** C11  
**Tema:** Força bruta cúbica O(n³) versus acumulação quadrática O(n²)

## 1. Objetivo

Encontrar o **subvetor contínuo não vazio** com a maior soma, inclusive quando existem números negativos. Comparar duas implementações em C que produzem o mesmo resultado: a primeira recalcula a soma de cada intervalo com três laços; a segunda atualiza a soma acumulada em dois laços.

O enunciado exige os tamanhos **n = 50, 100, 300 e 500** e a contagem de **somas e iterações**.

## 2. Estrutura

```text
projeto10_soma_maxima/
├── Makefile
├── README.md
├── src/
│   ├── main.c                 # Experimentos, medições e geração de CSV
│   ├── max_subarray.c         # Os dois algoritmos
│   └── max_subarray.h         # Contratos e métricas
├── tests/
│   └── test_max_subarray.c    # Testes automatizados
├── resultados/
│   ├── resultados.csv         # Execução real de referência
│   ├── comparacao_somas.png   # Gráfico das operações
│   └── comparacao_tempo.png   # Gráfico dos tempos
└── relatorio/
    ├── Relatorio_Projeto_10.docx
    └── Relatorio_Projeto_10.pdf
```

## 3. Compilação e execução

A partir da pasta raiz deste projeto:

### Windows (PowerShell, GCC/MinGW-w64)

```powershell
gcc -std=c11 -O2 -Wall -Wextra -Wpedantic -Isrc src/main.c src/max_subarray.c -o projeto10.exe
.\projeto10.exe resultados\resultados_windows.csv
# Sem argumentos, mostra as tabelas (sem gerar CSV):
.\projeto10.exe
# Para imprimir o CSV antigo no terminal:
.\projeto10.exe --csv

gcc -std=c11 -O2 -Wall -Wextra -Wpedantic -Isrc tests/test_max_subarray.c src/max_subarray.c -o teste_projeto10.exe
.\teste_projeto10.exe
```

### Linux / macOS (GCC ou Clang)

```bash
make
make test
make experimento
```

Alternativamente, sem `make`:

```bash
gcc -std=c11 -O2 -Wall -Wextra -Wpedantic -Isrc src/main.c src/max_subarray.c -o projeto10
./projeto10 resultados/resultados_local.csv
```

### Apresentação no terminal e exportação

- `./projeto10` (ou `.\projeto10.exe` no PowerShell) mostra tabelas de operações, tempos e validação.
- `./projeto10 resultados/resultados_local.csv` mostra as tabelas **e** salva o arquivo CSV.
- `./projeto10 --csv` imprime **somente CSV** no terminal, útil para redirecionamento.
- `./projeto10 --help` explica as opções.

As tabelas usam caracteres ASCII para funcionar no PowerShell, sem depender de cores ou
fontes especiais. Os separadores de milhar e a vírgula decimal aparecem somente nas
tabelas; o CSV continua usando os números em formato compatível com planilhas e scripts.

## 4. Exemplo de entrada e saída

Vetor: `[-2, 1, -3, 4, -1, 2, 1, -5, 4]`

Subvetor máximo: `[4, -1, 2, 1]`  
Índices inclusivos com base zero: `início = 3`, `fim = 6`  
Soma máxima: `6`.

Quando todos os valores são negativos, a solução retorna o **maior elemento individual**, sem permitir o subvetor vazio. Em empate, prevalece o primeiro intervalo identificado pela varredura `(i, j)`.

## 5. Como as métricas são contadas

- **Iterações externas:** vezes em que o laço do índice `i` inicia uma iteração (n vezes em cada abordagem).
- **Subvetores avaliados:** vezes em que o laço do índice `j` define um intervalo `[i, j]`. O total é `n(n+1)/2` nas duas abordagens.
- **Somas:** adições da forma `soma += vetor[k]` no terceiro laço cúbico, ou `soma += vetor[j]` no segundo laço quadrático.
- **Tempo:** mediana de cinco amostras de **tempo decorrido** por execução: relógio de alta resolução `QueryPerformanceCounter` no Windows e `clock_gettime(CLOCK_MONOTONIC)` em Linux/macOS. O programa calibra a quantidade de repetições separadamente para cada algoritmo, buscando cerca de 60 ms por amostra, evitando medições zeradas com rotinas muito rápidas.
- **Repetições efetivas:** o CSV mantém a coluna `repeticoes_por_amostra` para o algoritmo cúbico e adiciona `quadratico_repeticoes_por_amostra`, porque os algoritmos têm durações muito diferentes.
- **Unidades:** as tabelas convertem os tempos para milissegundos (`ms`), microssegundos (`us`) ou nanossegundos (`ns`), conforme a magnitude. O CSV continua armazenando os tempos em milissegundos.

A contagem operacional é **determinística** e independente do equipamento. Os tempos **variam** conforme CPU, compilador, otimizações e carga do sistema.

## 6. Fundamentação

Número de intervalos contíguos não vazios:

`S(n) = n(n + 1)/2`.

Número exato de somas no algoritmo cúbico:

`T3(n) = Σ_{L=1}^{n} L(n - L + 1) = n(n + 1)(n + 2)/6 = Θ(n³)`.

Número exato de somas no algoritmo quadrático:

`T2(n) = Σ_{i=0}^{n-1} (n - i) = n(n + 1)/2 = Θ(n²)`.

Razão exata:

`T3(n)/T2(n) = (n + 2)/3`.

**Observação:** os algoritmos calculam a soma com inteiros de 64 bits (`int64_t`) para evitar overflow de somas que excedam um único `int` nos casos testados. Este trabalho não implementa a solução linear de Kadane, porque ela não foi solicitada pelo enunciado.

## 7. Reprodutibilidade

- Gerador pseudorrandômico determinístico: `xorshift32` com semente `20261020`.
- Intervalo dos valores: inteiros de `-100` a `100`, inclusive.
- Tamanhos avaliados: `50`, `100`, `300`, `500`.
- Amostras de tempo: `5`.
- Repetições **mínimas iniciais** por amostra: `1000`, `250`, `20`, `5` respectivamente; cada algoritmo tem suas repetições reais calibradas automaticamente.
- Compilação da execução de referência: **GCC 14.2.0, Linux x86_64, `-O2`**.
- Os tempos do `resultados.csv` são observações de referência desse ambiente e não benchmarks universais. A versão com tabelas usa um relógio de alta resolução e calibração automática, então novas medições não devem ser comparadas diretamente com tempos obtidos anteriormente por `clock()`. O relatório acadêmico fornecido separadamente contém os resultados da medição de referência.

## 8. Conferência

Todos os testes verificam **resultado, índices e as fórmulas exatas de contagem**. São cobertos casos positivos, negativos, zeros, empate, vetor unitário, soma superior a `INT_MAX` e argumentos inválidos.

Antes de entregar o projeto, confira a identificação acadêmica do relatório e, se desejar comparar os tempos de sua própria máquina, gere um novo CSV. Os valores exatos de operações devem continuar iguais. Agora execute sem argumentos para obter o resumo formatado ou passe um caminho para também exportar CSV.
