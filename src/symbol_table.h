#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ESCOPOS 50

typedef enum {
    SYM_VARIAVEL,
    SYM_PARAM,
    SYM_FUNCAO,
    SYM_CLASSE
} SymbolKind;

typedef struct Param {
    char* nome;
    char* tipo;
    struct Param* proximo;
} Param;

typedef struct Symbol {
    char* nome;
    char* tipo;        // tipo de retorno para funções, tipo da variável para variáveis
    long linha;
    SymbolKind kind;
    Param* parametros;  // lista de parâmetros, só usada para funções
    struct Symbol* proximo;
} Symbol;

typedef struct {
    Symbol* escopos[MAX_ESCOPOS]; // Pilha de escopos locais
    int nivel_escopo_atual;
    Symbol* globais;              // Lista de símbolos globais (fora da pilha)
} TabelaDeSimbolos;

void inicializar_tabela(TabelaDeSimbolos* tabela);
void entrar_escopo(TabelaDeSimbolos* tabela);
void sair_escopo(TabelaDeSimbolos* tabela);
Symbol* inserir_simbolo(TabelaDeSimbolos* tabela, char* nome, const char* tipo, SymbolKind kind, long linha, int is_global);
Symbol* buscar_simbolo(TabelaDeSimbolos* tabela, const char* nome);
const char* kind_para_string(SymbolKind kind);
void imprimir_tabela_simbolos(TabelaDeSimbolos tabela);
Param* criar_parametro(const char* nome, const char* tipo);
void adicionar_parametro(Symbol* func, Param* param);

#endif
