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
    NODE_TYPE_OPERACAO_UNARIA,
    NODE_TYPE_PROGRAMA,
    NODE_TYPE_FUNCAO_DECL,
    NODE_TYPE_ATRIBUICAO,
    NODE_TYPE_VAR_DECL,
    NODE_TYPE_ARRAY_LITERAL,
    NODE_TYPE_RETURN,
    NODE_TYPE_IF,
    NODE_TYPE_WHILE,
    NODE_TYPE_STRING,
    NODE_TYPE_FUNCAO_CALL,
    NODE_TYPE_CLASSE_DECL,
    // Adicione outros tipos de nós conforme necessário
} NodeType;

// Estrutura para um nó da AST
typedef struct ASTNode {
    NodeType type;
    char *tipo_dado;
    char *valor; // Para números (como string), identificadores, operadores
    int child_count;
    long linha;
    struct ASTNode **filhos; // Para operandos ou subárvores
    struct ASTNode *proximo_comando; // Para listas de comandos em um programa

    int is_array;       // Um booleano (0 ou 1) para sabermos se é um vetor
    int array_size;     // Para guardar o tamanho do vetor
} ASTNode;

// Função auxiliar para criar nós
ASTNode* criar_no(NodeType type, const char* valor, ASTNode* filhos[], int tamanho, ASTNode* proximo);

void imprimir_ast(ASTNode *no, int nivel);

void lista_para_vetor(ASTNode* no_pai, ASTNode* lista);

ASTNode* criar_no_declaracao(const char* tipo_dado, const char* nome_var, long linha);

ASTNode* criar_no_declaracao_com_valor(const char* tipo_dado, const char* nome_var, ASTNode* valor_inicial, long linha);

ASTNode* criar_no_declaracao_vetor(const char* tipo_dado, const char* nome_var, ASTNode* lista_inicializadores, long linha);

ASTNode* criar_no_literal_vetor(ASTNode* lista_elementos);

ASTNode* criar_no_funcao(const char* tipo_retorno, const char* nome_func, ASTNode* filhos[], int num_filhos, long linha);

ASTNode* criar_no_chamada_funcao(const char* nome_func, ASTNode* lista_argumentos);

ASTNode* criar_no_classe(const char* nome_classe, ASTNode* lista_membros, long linha);

ASTNode* criar_no_programa(ASTNode* lista_declaracoes);

#endif