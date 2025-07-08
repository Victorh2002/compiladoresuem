#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "ast.h"
#include "symbol_table.h"
#include "semantico.h"

bool erro_semantico = false;
bool tipo_igual = true;
bool expressao_logica = false;
extern long linha;

void verifica_tipo_expressao(ASTNode* raizAST) {
    if (raizAST == NULL) {
        return;
    }

    if (raizAST->type == NODE_TYPE_OPERACAO_BINARIA)
    {
        if (strcmp(raizAST->valor, "+") != 0 && strcmp(raizAST->valor, "-") != 0 && strcmp(raizAST->valor, "*") != 0 && strcmp(raizAST->valor, "/") != 0)
        {
            expressao_logica = true;
        }
    }
}

void verifica_tipagem(ASTNode* raizAST, char* tipo, TabelaDeSimbolos* tabela) {
    if (raizAST == NULL) {
        return;
    }

    int eh_string = strcmp("string", tipo);

    if (raizAST->type == NODE_TYPE_IDENTIFICADOR) {
        Symbol* ID = buscar_simbolo(tabela, raizAST->valor);
        if(ID){
            if (strcmp(ID->tipo, "int") == 0 && strcmp(tipo, "float") == 0)
            {
                
            } else if (strcmp(ID->tipo, tipo) != 0 && eh_string != 0)
            {
                tipo_igual = false;   
            } 
        } else {
            printf("ID '%s' não existe semelhante ao mundial do Palmeiras!\n", raizAST->valor);
            exit(0);
        }
    }

    if (raizAST->type == NODE_TYPE_NUMERO)
    {
        char* resultado = strchr(raizAST->valor, '.');
        bool decimal = false;
        if(resultado != NULL) decimal = true;
        
        if (decimal == true && strcmp("float", tipo) != 0){

            tipo_igual = false;

        } else if (strcmp(tipo, "string") == 0 || strcmp(tipo, "char") == 0)
        {
            tipo_igual = false;
        }
        
    }

    if (raizAST->type == NODE_TYPE_STRING)
    {
        size_t tamanho_string = strlen(raizAST->valor);

        if ((strcmp(tipo, "char") == 0 && tamanho_string > 1) || (strcmp(tipo, "string") != 0 && strcmp(tipo, "char") != 0 ))
        {
            tipo_igual = false;
        }
    }
    

    for (int i = 0; i < raizAST->child_count; i++)
    {      
        verifica_tipagem(raizAST->filhos[i], tipo, tabela);
    }
}

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
    else if (raizAST->type == NODE_TYPE_VAR_DECL) {
        if (tabela->nivel_escopo_atual != -1) {
            inserir_simbolo(tabela, raizAST->valor, raizAST->tipo_dado,
                        SYM_VARIAVEL,
                        raizAST->linha, 0); // Local
        } else {
            inserir_simbolo(tabela, raizAST->valor, raizAST->tipo_dado,
                        SYM_VARIAVEL,
                        raizAST->linha, 1); // Global
        }
        
        imprimir_tabela_simbolos(*tabela);
    }

    else if (raizAST->type == NODE_TYPE_ATRIBUICAO)
    {
        Symbol* id = buscar_simbolo(tabela, raizAST->filhos[0]->valor);
        if (!id)
        {
            printf("ID '%s' não existe semelhante ao mundial do Palmeiras!\n", raizAST->filhos[0]->valor);
            exit(0);
        }

        verifica_tipagem(raizAST->filhos[1], id->tipo, tabela);
        verifica_tipo_expressao(raizAST->filhos[1]);

        if (expressao_logica == true)
        {
            printf("Permitido apenas operadores aritméticos em atribuições! Linha: %ld\n", raizAST->linha);
            exit(0);
        }

        if (tipo_igual == false)
        {
            printf("O tipo da expressão não coincide com o tipo da variável! Linha: %ld\n", raizAST->linha);
            exit(0);
        }
        
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

