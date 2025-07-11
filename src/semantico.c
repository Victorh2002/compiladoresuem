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

void teste(ASTNode* raizAST, ASTNode* raizASTpai, TabelaDeSimbolos* tabela) {
    if (raizAST == NULL) {
        return;
    }

    int abriu_escopo = 0;

    // Inserir classes e funções no escopo global
    if (raizAST->type == NODE_TYPE_CLASSE_DECL) {
        inserir_simbolo(tabela, raizAST->valor, raizAST->tipo_dado,
                        raizAST->type == NODE_TYPE_CLASSE_DECL ? SYM_CLASSE : SYM_FUNCAO,
                        raizAST->linha, 1, 0, 0, NULL); // Global

        printf("\n==> DEBUG: Entrando em %s '%s' (nível %d)\n",
               raizAST->type == NODE_TYPE_CLASSE_DECL ? "Classe" : "Função",
               raizAST->valor,
               tabela->nivel_escopo_atual);
        imprimir_tabela_simbolos(*tabela);
    }

    else if (raizAST->type == NODE_TYPE_FUNCAO_DECL) {
        Symbol* func;
        if (raizASTpai->type == NODE_TYPE_CLASSE_DECL)
        {
            func = inserir_simbolo(tabela, raizAST->valor, raizAST->tipo_dado, SYM_FUNCAO, raizAST->linha, 1, 0, 0, raizASTpai->valor);
        } else
        {
            func = inserir_simbolo(tabela, raizAST->valor, raizAST->tipo_dado, SYM_FUNCAO, raizAST->linha, 1, 0, 0, NULL);
        }
        if (!func) return;

        entrar_escopo(tabela);
        abriu_escopo = 1;

        // Inserir parâmetros: filhos com mesma linha da função
        for (int i = 0; i < raizAST->child_count; i++) {
            ASTNode* filho = raizAST->filhos[i];
            if (filho->linha == raizAST->linha && filho->type == NODE_TYPE_VAR_DECL) {
                Param* p = criar_parametro(filho->valor, filho->tipo_dado);
                adicionar_parametro(func, p);
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

    else if (raizAST->type == NODE_TYPE_VAR_DECL) {
        Symbol* id = buscar_simbolo(tabela, raizAST->valor);

        if (id)
        {
            printf("Variável, atributo, função ou método com nome '%s' já existe!\n", id->nome);
            exit(0);
        }

        if (strcmp(raizAST->tipo_dado, "int") != 0 && strcmp(raizAST->tipo_dado, "float") != 0 && strcmp(raizAST->tipo_dado, "char") != 0 && strcmp(raizAST->tipo_dado, "string") != 0)
        {
            Symbol* classe = buscar_simbolo(tabela, raizAST->tipo_dado);

            if (!classe)
            {
                printf("Classe '%s' não existe!\n", raizAST->tipo_dado);
                exit(0);
            }
        }

        if (!id && raizAST->is_array == 0)
        {
            if (tabela->nivel_escopo_atual != -1) {
                if (raizASTpai->type == NODE_TYPE_CLASSE_DECL)
                {
                    inserir_simbolo(tabela, raizAST->valor, raizAST->tipo_dado,
                            SYM_VARIAVEL,
                            raizAST->linha, 0, 0, 0, raizASTpai->valor); // Local
                } else
                {
                    inserir_simbolo(tabela, raizAST->valor, raizAST->tipo_dado,
                            SYM_VARIAVEL,
                            raizAST->linha, 0, 0, 0, NULL); // Local
                }
            } else {
                if (raizASTpai->type == NODE_TYPE_CLASSE_DECL)
                {
                    inserir_simbolo(tabela, raizAST->valor, raizAST->tipo_dado,
                            SYM_VARIAVEL,
                            raizAST->linha, 1, 0, 0, raizASTpai->valor); // Local
                } else
                {
                    inserir_simbolo(tabela, raizAST->valor, raizAST->tipo_dado,
                            SYM_VARIAVEL,
                            raizAST->linha, 1, 0, 0, NULL); // Local
                }
            }

            if (raizAST->child_count > 0)
            {
                verifica_tipagem(raizAST->filhos[0], raizAST->tipo_dado, tabela);
                verifica_tipo_expressao(raizAST->filhos[0]);

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

            imprimir_tabela_simbolos(*tabela);
        }

        if (!id && raizAST->is_array == 1) {
            if (tabela->nivel_escopo_atual != -1) {
                if (raizASTpai->type == NODE_TYPE_CLASSE_DECL)
                {
                    inserir_simbolo(tabela, raizAST->valor, raizAST->tipo_dado,
                            SYM_VARIAVEL,
                            raizAST->linha, 0, 0, 0, raizASTpai->valor); // Local
                } else
                {
                    inserir_simbolo(tabela, raizAST->valor, raizAST->tipo_dado,
                            SYM_VARIAVEL,
                            raizAST->linha, 0, 0, 0, NULL); // Local
                }
            } else {
                if (raizASTpai->type == NODE_TYPE_CLASSE_DECL)
                {
                    inserir_simbolo(tabela, raizAST->valor, raizAST->tipo_dado,
                            SYM_VARIAVEL,
                            raizAST->linha, 1, 0, 0, raizASTpai->valor); // Local
                } else
                {
                    inserir_simbolo(tabela, raizAST->valor, raizAST->tipo_dado,
                            SYM_VARIAVEL,
                            raizAST->linha, 1, 0, 0, NULL); // Local
                }
            }

            verifica_tipagem(raizAST->filhos[0], raizAST->tipo_dado, tabela);
            verifica_tipo_expressao(raizAST->filhos[0]);

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
    }

    else if (raizAST->type == NODE_TYPE_ATRIBUICAO)
    {
        ASTNode* filho = raizAST->filhos[0];
        int is_array = 0;
        int campo_vetor;

        if (filho->type == NODE_TYPE_MEMBER_ACCESS)
        {
            Symbol* membro = buscar_simbolo(tabela, filho->valor);
            Symbol* objeto = buscar_simbolo(tabela, filho->filhos[0]->valor);

            if (!membro)
            {
                printf("Membro não existe\n");
                exit(0);
            }

            if (!objeto)
            {
                printf("Objeto não existe\n");
                exit(0);
            }
            
            if (membro->nome_pai != NULL)
            {
                if (strcmp(membro->nome_pai, objeto->tipo) != 0)
                {
                    printf("Classe do objeto e membro não são iguais\n");
                    exit(0);
                }
            } else
            {
                printf("Membro não pertence a nenhuma classe\n");
                exit(0);
            }
            
        }

        if (filho->type == NODE_TYPE_ARRAY_ACCESS)
        {
            campo_vetor = atoi(filho->filhos[1]->valor);
            filho = filho->filhos[0];
            is_array = 1;
        }
        
        Symbol* id = buscar_simbolo(tabela, filho->valor);
        if (!id)
        {
            printf("ID '%s' não existe semelhante ao mundial do Palmeiras!\n", filho->valor);
            exit(0);
        }

        int tamanho_vetor = id->array_size;

        if (campo_vetor >= tamanho_vetor && is_array == 1)
        {
            printf("Você está tentando acessar o campo '%d' inexistente no vetor, tamanho do vetor '%s' é: %d\n", campo_vetor, id->nome, tamanho_vetor);
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
        teste(raizAST->filhos[i], raizAST, tabela);
    }

    // Sair do escopo se foi aberto
    if (abriu_escopo) {
        // Buscar e imprimir
        sair_escopo(tabela);
        printf("\n==> DEBUG: Saindo de escopo (nível agora %d)\n", tabela->nivel_escopo_atual);
    }
}

