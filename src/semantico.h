#ifndef SEMANTICO_H
#define SEMANTICO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "symbol_table.h"

void verifica_tipagem(ASTNode* raizAST, char* tipo, TabelaDeSimbolos* tabela);
void teste(ASTNode* raizAST, TabelaDeSimbolos* tabela);

#endif