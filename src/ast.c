#include "ast.h"

// Função auxiliar para criar nós
ASTNode* criar_no(NodeType type, const char* valor, ASTNode* filhos[], int tamanho, ASTNode* proximo) {
    ASTNode* no = (ASTNode*) malloc(sizeof(ASTNode));
    if (!no) {
        printf("Falha ao alocar memória para nó da AST");
        exit(1); // Ou trate o erro de forma mais robusta
    }
    no->type = type;
    no->valor = valor ? strdup(valor) : NULL; // Copia o valor se existir
    if (tamanho && filhos)
    {
        no->filhos = (ASTNode**) malloc(tamanho * sizeof(ASTNode));
        for (int i = 0; i < tamanho; i++)
        {
            no->filhos[i] = filhos[i];
        }
        no->child_count = tamanho;
    }
    no->proximo_comando = proximo;
    return no;
}

void imprimir_ast(ASTNode *no, int nivel) {
    if (no == NULL) {
        return;
    }

    // Imprime indentação
    for (int i = 0; i < nivel; i++) {
        printf("  ");
    }

    switch (no->type) {
        case NODE_TYPE_PROGRAMA:
            printf("Programa\n");
            break;
        case NODE_TYPE_NUMERO:
            printf("Numero: %s\n", no->valor);
            break;
        case NODE_TYPE_IDENTIFICADOR:
            printf("ID: %s\n", no->valor);
            break;
        case NODE_TYPE_OPERACAO_BINARIA:
            printf("Operacao: %s\n", no->valor); // Imprime o operador (ex: "+")
            break;
        case NODE_TYPE_FUNCAO_DECL:
            printf("Declaração de Funcao: %s\n", no->valor); // Imprime o operador (ex: "+")
            break;
        case NODE_TYPE_ATRIBUICAO:
            printf("Atribuicao: %s\n", no->valor); // Imprime o operador (ex: "+")
            break;
        // Adicione outros casos
        default:
            printf("Nó Desconhecido\n");
    }

    if (no->child_count > 0) {
        //printf("child count %d\n", no->child_count);
        for (int i = 0; i < no->child_count; i++)
        {
            imprimir_ast(no->filhos[i], nivel + 1);
        }
        
    }

    imprimir_ast(no->proximo_comando, nivel);
}