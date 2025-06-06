# --- Variáveis de Configuração ---
CC = gcc
CFLAGS = -g -Wall
LDFLAGS = -lfl
EXECUTABLE = teste

# Lista de todos os seus arquivos fonte .c que você criou
SOURCES = main.c ast.c

# Arquivos gerados
GENERATED_C = lex.yy.c bison.tab.c
GENERATED_H = bison.tab.h

# Converte todas as fontes .c em nomes de arquivos objeto .o
OBJECTS = $(SOURCES:.c=.o) $(GENERATED_C:.c=.o)


# --- Regras de Compilação ---

.PHONY: all
all: $(EXECUTABLE)

# Regra para criar o executável final
# Depende de todos os arquivos objeto.
$(EXECUTABLE): $(OBJECTS)
	@echo "==> Linkando o programa final..."
	$(CC) $(CFLAGS) -o $(EXECUTABLE) $(OBJECTS) $(LDFLAGS)
	@echo -e "\n\033[0;32m==> Compilação concluída! Execute com: ./$(EXECUTABLE) <arquivo_de_teste>\033[0m"

# --- Regras de Dependência dos Arquivos Objeto ---
# Aqui está a correção principal. Declaramos do que cada .o precisa.

main.o: main.c ast.h bison.tab.h
	@echo "--> Compilando main.c..."
	$(CC) $(CFLAGS) -c main.c -o main.o

ast.o: ast.c ast.h
	@echo "--> Compilando ast.c..."
	$(CC) $(CFLAGS) -c ast.c -o ast.o

bison.tab.o: bison.tab.c ast.h
	@echo "--> Compilando o parser (bison.tab.c)..."
	$(CC) $(CFLAGS) -c bison.tab.c -o bison.tab.o

lex.yy.o: lex.yy.c ast.h bison.tab.h
	@echo "--> Compilando o lexer (lex.yy.c)..."
	$(CC) $(CFLAGS) -c lex.yy.c -o lex.yy.o


# --- Regras para Gerar Código ---

# Regra para criar os arquivos do Bison.
bison.tab.c bison.tab.h: bison.y
	@echo "--> Executando o Bison..."
	bison -dv bison.y

# Regra para criar o arquivo do Flex.
lex.yy.c: lexico.lex bison.tab.h
	@echo "--> Executando o Flex..."
	flex -o lex.yy.c lexico.lex


# Regra "clean" para apagar todos os arquivos gerados
.PHONY: clean
clean:
	@echo "--> Limpando o projeto..."
	rm -f $(EXECUTABLE) $(OBJECTS) lex.yy.c bison.tab.c bison.tab.h