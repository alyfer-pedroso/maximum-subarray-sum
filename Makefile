CC ?= gcc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Wpedantic
INCLUDES = -Isrc
BIN = projeto10
TEST_BIN = teste_projeto10

.PHONY: all test experimento clean
all: $(BIN)

$(BIN): src/main.c src/max_subarray.c src/max_subarray.h
	$(CC) $(CFLAGS) $(INCLUDES) src/main.c src/max_subarray.c -o $@

$(TEST_BIN): tests/test_max_subarray.c src/max_subarray.c src/max_subarray.h
	$(CC) $(CFLAGS) $(INCLUDES) tests/test_max_subarray.c src/max_subarray.c -o $@

test: $(TEST_BIN)
	./$(TEST_BIN)

experimento: $(BIN)
	./$(BIN) resultados/resultados.csv

clean:
	rm -f $(BIN) $(TEST_BIN)
