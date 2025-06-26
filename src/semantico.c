#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "symbol_table.h"
#include "semantico.h"

void preenche_tabela(ASTNode* raizAST, ASTNode* raizASTpai) {
    if (raizAST == NULL) {
        return;
    }
    
    switch (raizAST->type)
    {
        case NODE_TYPE_VAR_DECL:
            if (raizASTpai == NULL || strcmp("Programa",raizASTpai->valor) == 0)
            {
                insert_symbol(raizAST->valor, raizAST->tipo_dado, "global");
            } else {
                insert_symbol(raizAST->valor, raizAST->tipo_dado, raizASTpai->valor);
            }
            break;
        case NODE_TYPE_FUNCAO_DECL:
            insert_symbol(raizAST->valor, raizAST->tipo_dado, "global");
            break;
        case NODE_TYPE_CLASSE_DECL:
            insert_symbol(raizAST->valor, "class", "global");
            break;
        default:
            break;
    }
    
    for (int i = 0; i < raizAST->child_count; i++) {
        preenche_tabela(raizAST->filhos[i], raizAST);
    }
}