#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    SYM_VARIAVEL,
    SYM_FUNCAO,
    SYM_CLASSE
} SymbolKind;

typedef struct Symbol {
    char* nome;
    char* tipo;
    char* escopo;
    long linha;
    SymbolKind kind;
    struct Symbol *proximo;
} Symbol;

void insert_symbol(const char* nome, const char* tipo, SymbolKind kind, const char* nome_escopo, long linha);
Symbol* lookup_symbol(const char* nome, const char* escopo_atual);
const char* kind_para_string(SymbolKind kind);
void imprimir_tabela_simbolos(void);

#endif