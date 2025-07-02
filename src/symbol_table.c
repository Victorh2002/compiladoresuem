#include <stdio.h>
#include <string.h>
#include "symbol_table.h"

extern Symbol* tabela_de_simbolos;

/**
 * Insere um novo símbolo. ANTES de inserir, verifica se já não existe
 * um símbolo com o mesmo nome NO MESMO ESCOPO.
 */
void insert_symbol(const char* nome, const char* tipo, SymbolKind kind, const char* nome_escopo, long linha) {
    // Lógica de verificação ANTES de inserir
    Symbol* no_atual = tabela_de_simbolos;
    while (no_atual != NULL) {
        // Checa se o nome E o escopo são iguais
        if (strcmp(nome, no_atual->nome) == 0 && strcmp(nome_escopo, no_atual->escopo) == 0) {
            fprintf(stderr, "Erro Semântico: Variável '%s' já declarada no escopo '%s'.\n", nome, nome_escopo);
            return;
        }
        no_atual = no_atual->proximo;
    }

    // Se passou na verificação, cria e insere o novo símbolo no início da lista
    Symbol* novo_simbolo = (Symbol*) malloc(sizeof(Symbol));
    novo_simbolo->nome = strdup(nome);
    if (tipo)
    {
        novo_simbolo->tipo = strdup(tipo);
    }
    novo_simbolo->kind = kind;
    novo_simbolo->escopo = strdup(nome_escopo);
    novo_simbolo->linha = linha;
    novo_simbolo->proximo = tabela_de_simbolos;
    tabela_de_simbolos = novo_simbolo;
}

/**
 * Procura por um símbolo. Primeiro no escopo atual, depois no escopo global.
 */
Symbol* lookup_symbol(const char* nome, const char* escopo_atual) {
    Symbol* no_atual = tabela_de_simbolos;

    // Fase 1: Procurar no escopo atual
    while (no_atual != NULL) {
        if (strcmp(nome, no_atual->nome) == 0 && strcmp(escopo_atual, no_atual->escopo) == 0) {
            return no_atual; // Achou no escopo local!
        }
        no_atual = no_atual->proximo;
    }

    // Fase 2: Se não achou, procura no escopo global (se não já estivermos nele)
    if (strcmp(escopo_atual, "global") != 0) {
        no_atual = tabela_de_simbolos;
        while (no_atual != NULL) {
            if (strcmp(nome, no_atual->nome) == 0 && strcmp("global", no_atual->escopo) == 0) {
                return no_atual; // Achou no escopo global!
            }
            no_atual = no_atual->proximo;
        }
    }
    
    return NULL; // Não encontrou em nenhum escopo visível
}

const char* kind_para_string(SymbolKind kind) {
    switch (kind) {
        case SYM_VARIAVEL: return "Variavel";
        case SYM_FUNCAO:   return "Funcao";
        case SYM_CLASSE:   return "Classe";
        default:           return "Desconhecido";
    }
}

void imprimir_tabela_simbolos(void) {
    // Cria um ponteiro temporário para percorrer a lista sem modificar o original
    Symbol* no_atual = tabela_de_simbolos;

    printf("\n.--------------------------------------------------------------------------.\n");
    printf("|                              Tabela de Símbolos                          |\n");
    printf("+-----------------+-----------------+-----------------+--------------------+\n");
    printf("| ESCOPO          | CATEGORIA       | TIPO            | NOME       | LINHA |\n");
    printf("+-----------------+-----------------+-----------------+--------------------+\n");

    if (no_atual == NULL) {
        printf("| (Tabela Vazia)                                                   |\n");
    } else {
        while (no_atual != NULL) {
            printf("| %-15s | %-15s | %-15s | %-10s | %-5ld |\n", 
                    no_atual->escopo, 
                    kind_para_string(no_atual->kind), // <-- Imprime a nova coluna
                    no_atual->tipo ? no_atual->tipo : "N/A", // <-- O tipo de dado
                    no_atual->nome,
                    no_atual->linha);
            no_atual = no_atual->proximo;
        }
    }
    
    printf("+-----------------+-----------------+-----------------+------------+-------+\n\n");
}
