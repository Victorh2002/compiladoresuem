#ifndef SEMANTICO_H
#define SEMANTICO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "symbol_table.h"

void preenche_tabela(ASTNode* raizAST, ASTNode* raizASTpai);
void contagem_escopo(ASTNode* raizAST, TabelaDeSimbolos tabela);
void teste(ASTNode* raizAST, TabelaDeSimbolos* tabela);

#endif