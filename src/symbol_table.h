#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Symbol {
    char* nome;
    char* tipo;
    char* escopo;
    struct Symbol *proximo;
} Symbol;

void insert_symbol(const char* nome, const char* tipo, const char* nome_escopo);
Symbol* lookup_symbol(const char* nome, const char* escopo_atual);
void imprimir_tabela_simbolos(void);

#endif