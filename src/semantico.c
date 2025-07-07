#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "ast.h"
#include "symbol_table.h"
#include "semantico.h"

/*
void preenche_tabela(ASTNode* raizAST, ASTNode* raizASTpai) {
    if (raizAST == NULL) {
        return;
    }
    
    switch (raizAST->type)
    {
        case NODE_TYPE_VAR_DECL:
            if (raizASTpai == NULL || strcmp("Programa",raizASTpai->valor) == 0)
            {
                insert_symbol(raizAST->valor, raizAST->tipo_dado, SYM_VARIAVEL, "global", raizAST->linha);
            } else {
                insert_symbol(raizAST->valor, raizAST->tipo_dado, SYM_VARIAVEL, raizASTpai->valor, raizAST->linha);
            }
            break;
        case NODE_TYPE_FUNCAO_DECL:
            if (raizASTpai == NULL || strcmp("Programa",raizASTpai->valor) == 0)
            {
                insert_symbol(raizAST->valor, raizAST->tipo_dado, SYM_FUNCAO, "global", raizAST->linha);
            } else {
                insert_symbol(raizAST->valor, raizAST->tipo_dado, SYM_FUNCAO, raizASTpai->valor, raizAST->linha);
            }
            break;
        case NODE_TYPE_CLASSE_DECL:
            insert_symbol(raizAST->valor, NULL, SYM_CLASSE, "global", raizAST->linha);
            break;
        default:
            break;
    }
    
    for (int i = 0; i < raizAST->child_count; i++) {
        preenche_tabela(raizAST->filhos[i], raizAST);
    }
}*/

void teste(ASTNode* raizAST, TabelaDeSimbolos* tabela) {
    if (raizAST == NULL) {
        return;
    }

    int abriu_escopo = 0;

    // Inserir classes e funções no escopo global
    if (raizAST->type == NODE_TYPE_CLASSE_DECL) {
        inserir_simbolo(tabela, raizAST->valor, raizAST->tipo_dado,
                        raizAST->type == NODE_TYPE_CLASSE_DECL ? SYM_CLASSE : SYM_FUNCAO,
                        raizAST->linha, 1); // Global

        entrar_escopo(tabela);
        abriu_escopo = 1;

        printf("\n==> DEBUG: Entrando em %s '%s' (nível %d)\n",
               raizAST->type == NODE_TYPE_CLASSE_DECL ? "Classe" : "Função",
               raizAST->valor,
               tabela->nivel_escopo_atual);
        imprimir_tabela_simbolos(*tabela);
    }

    else if (raizAST->type == NODE_TYPE_FUNCAO_DECL) {
        // Insere a função no escopo global
        Symbol* func = inserir_simbolo(tabela, raizAST->valor, raizAST->tipo_dado, SYM_FUNCAO, raizAST->linha, 1);
        if (!func) return;

        entrar_escopo(tabela);
        abriu_escopo = 1;

        // Inserir parâmetros: filhos com mesma linha da função
        for (int i = 0; i < raizAST->child_count; i++) {
            ASTNode* filho = raizAST->filhos[i];
            if (filho->linha == raizAST->linha && filho->type == NODE_TYPE_VAR_DECL) {
                Param* p = criar_parametro(filho->valor, filho->tipo_dado);
                adicionar_parametro(func, p);

                // Insere o parâmetro também na tabela local
                inserir_simbolo(tabela, filho->valor, filho->tipo_dado, SYM_PARAM, filho->linha, 0);
            }
        }

        printf("\n==> DEBUG: Função '%s' com parâmetros inseridos (nível %d)\n", func->nome, tabela->nivel_escopo_atual);
        imprimir_tabela_simbolos(*tabela);

    }

    // Abrir escopo para IF/WHILE (sem inserir símbolo)
    else if (raizAST->type == NODE_TYPE_IF || raizAST->type == NODE_TYPE_WHILE) {
        entrar_escopo(tabela);
        abriu_escopo = 1;

        printf("\n==> DEBUG: Entrando em bloco (if/while), nível %d\n", tabela->nivel_escopo_atual);
        imprimir_tabela_simbolos(*tabela);
    }

    // Inserir variáveis locais no escopo atual
    else if (raizAST->type == NODE_TYPE_VAR_DECL && buscar_simbolo(tabela, raizAST->valor) == NULL) {
        inserir_simbolo(tabela, raizAST->valor, raizAST->tipo_dado,
                        SYM_VARIAVEL,
                        raizAST->linha, 0); // Local
        imprimir_tabela_simbolos(*tabela);
    }

    // Travessia recursiva dos filhos
    for (int i = 0; i < raizAST->child_count; i++) {
        teste(raizAST->filhos[i], tabela);
    }

    // Sair do escopo se foi aberto
    if (abriu_escopo) {
        // Buscar e imprimir
        sair_escopo(tabela);
        printf("\n==> DEBUG: Saindo de escopo (nível agora %d)\n", tabela->nivel_escopo_atual);
    }
}

