#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

ASTNode *raiz_ast = NULL;

int main() {
    ASTNode *teste1 = criar_no(NODE_TYPE_NUMERO, "teste1", NULL, NULL, NULL);
    ASTNode *teste2 = criar_no(NODE_TYPE_NUMERO, "teste2", NULL, NULL, NULL);
    ASTNode *teste3 = criar_no(NODE_TYPE_NUMERO, "teste3", NULL, NULL, NULL);
    ASTNode *teste4 = criar_no(NODE_TYPE_NUMERO, "teste4", NULL, NULL, NULL);
    ASTNode *teste[4] = {teste1, teste2, teste3, teste4};
    raiz_ast = criar_no(NODE_TYPE_PROGRAMA, "teste", teste, 4, NULL);
    imprimir_ast(raiz_ast->filhos[3], 0);
}