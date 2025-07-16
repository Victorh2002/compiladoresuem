# ===================================================================
# --- Variáveis de Configuração ---
# ===================================================================

# Ferramentas
CC = gcc
BISON = bison
FLEX = flex
LLVM_CONFIG = llvm-config-15 # Use a versão do LLVM que você instalou

# Flags
# CFLAGS: Adiciona as flags de include do LLVM usando llvm-config
# LDFLAGS: Adiciona as bibliotecas do Flex e do LLVM para a linkagem
CFLAGS = -g -Wall $(shell $(LLVM_CONFIG) --cflags)
LDFLAGS = -lfl $(shell $(LLVM_CONFIG) --libs --system-libs --ldflags)

# Arquivos
EXECUTABLE = meu_compilador
SOURCES = main.c ast.c symbol_table.c semantico.c codegen.c
GENERATED_C = lex.yy.c bison.tab.c
GENERATED_H = bison.tab.h
OBJECTS = $(SOURCES:.c=.o) $(GENERATED_C:.c=.o)


# ===================================================================
# --- Alvos Principais ---
# ===================================================================

.PHONY: all clean

all: $(EXECUTABLE)

clean:
	@echo "--> Limpando o projeto..."
	rm -f $(EXECUTABLE) $(OBJECTS) $(GENERATED_C) $(GENERATED_H) bison.output


# ===================================================================
# --- Regra de Linkagem Final ---
# ===================================================================

# Junta todos os arquivos objeto (.o) para criar o executável final.
$(EXECUTABLE): $(OBJECTS)
	@echo "==> Linkando o programa final com as bibliotecas do LLVM..."
	$(CC) $(CFLAGS) -o $(EXECUTABLE) $(OBJECTS) $(LDFLAGS)
	@echo -e "\n\033[0;32m==> Compilação concluída! Execute com: ./$(EXECUTABLE) -e <arquivo>\033[0m"


# ===================================================================
# --- Regras de Compilação (.c -> .o) ---
# ===================================================================
# Cada arquivo objeto (.o) depende do seu .c e dos cabeçalhos que ele inclui.

main.o: main.c ast.h bison.tab.h symbol_table.h semantico.h codegen.h
	$(CC) $(CFLAGS) -c main.c -o main.o

ast.o: ast.c ast.h
	$(CC) $(CFLAGS) -c ast.c -o ast.o

symbol_table.o: symbol_table.c symbol_table.h ast.h
	$(CC) $(CFLAGS) -c symbol_table.c -o symbol_table.o

semantico.o: semantico.c semantico.h ast.h symbol_table.h
	$(CC) $(CFLAGS) -c semantico.c -o semantico.o

codegen.o: codegen.c codegen.h ast.h
	$(CC) $(CFLAGS) -c codegen.c -o codegen.o

bison.tab.o: bison.tab.c ast.h
	$(CC) $(CFLAGS) -c bison.tab.c -o bison.tab.o

lex.yy.o: lex.yy.c bison.tab.h
	$(CC) $(CFLAGS) -c lex.yy.c -o lex.yy.o


# ===================================================================
# --- Regras de Geração de Código (Bison & Flex) ---
# ===================================================================

bison.tab.c bison.tab.h: bison.y
	@echo "--> Executando o Bison..."
	$(BISON) -dv bison.y

lex.yy.c: lexico.lex bison.tab.h
	@echo "--> Executando o Flex..."
	$(FLEX) -o lex.yy.c lexico.lex