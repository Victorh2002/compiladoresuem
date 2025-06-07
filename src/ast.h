#ifndef AST_H
#define AST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Enum para tipos de nós (exemplo)
typedef enum {
    NODE_TYPE_NUMERO,
    NODE_TYPE_IDENTIFICADOR,
    NODE_TYPE_OPERACAO_BINARIA,
    NODE_TYPE_PROGRAMA,
    NODE_TYPE_FUNCAO_DECL,
    NODE_TYPE_ATRIBUICAO,
    // Adicione outros tipos de nós conforme necessário
} NodeType;

// Estrutura para um nó da AST
typedef struct ASTNode {
    NodeType type;
    char *valor; // Para números (como string), identificadores, operadores
    int child_count;
    struct ASTNode **filhos; // Para operandos ou subárvores
    struct ASTNode *proximo_comando; // Para listas de comandos em um programa
} ASTNode;

// Função auxiliar para criar nós
ASTNode* criar_no(NodeType type, const char* valor, ASTNode* filhos[], int tamanho, ASTNode* proximo);

void imprimir_ast(ASTNode *no, int nivel);

#endif