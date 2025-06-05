#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Enum para tipos de nós (exemplo)
typedef enum {
    NODE_TYPE_NUMERO,
    NODE_TYPE_IDENTIFICADOR,
    NODE_TYPE_OPERACAO_BINARIA,
    NODE_TYPE_PROGRAMA,
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
            printf("%s", filhos[i]->valor);
        }
        no->child_count = tamanho - 1;
    }
    no->proximo_comando = proximo;
    return no;
}

/*
// Protótipo da função de impressão (será definida depois)
void imprimir_ast(ASTNode *no, int nivel) {
    if (no == NULL) {
        return;
    }

    // Imprime indentação
    for (int i = 0; i < nivel; i++) {
        printf("  ");
    }

    // Imprime informações do nó
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
        // Adicione outros casos
        default:
            printf("Nó Desconhecido\n");
    }

    // Chama recursivamente para os filhos (se houver)
    if (no->filho_esquerda) {
        imprimir_ast(no->filho_esquerda, nivel + 1);
    }
    if (no->filho_direita) {
        imprimir_ast(no->filho_direita, nivel + 1);
    }
    // Se for uma lista de comandos, percorre a lista
    if (no->proximo_comando) { // (Ajustar: proximo_comando é mais para o nível de lista_comandos)
        // Se a impressão é estritamente hierárquica, o proximo_comando
        // seria impresso no mesmo nível pelo chamador da lista.
        // Ou, se um nó de "Programa" ou "Bloco" tem uma lista de filhos:
        // ASTNode* atual = no->primeiro_comando_do_bloco;
        // while(atual) {
        //    imprimir_ast(atual, nivel + (é_filho_direto ? 1 : 0));
        //    atual = atual->proximo_comando_na_lista;
        // }
        // Para o exemplo atual, `lista_comandos` é encadeada via `proximo_comando`:
        if (no->type == NODE_TYPE_PROGRAMA || nivel == 0) { // Apenas para o nó raiz ou um nó de programa
             imprimir_ast(no->proximo_comando, nivel); // Imprime o próximo comando no mesmo nível
        }
    }
}*/