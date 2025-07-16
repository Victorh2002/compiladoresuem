#ifndef SEMANTICO_H
#define SEMANTICO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "symbol_table.h"

void verificar_chamada_especial(ASTNode* no_chamada, TabelaDeSimbolos* tabela);
void verifica_parametros_chamada_funcao(ASTNode* raizAST, Param* parametros, TabelaDeSimbolos* tabela);
void verifica_tipo_expressao(ASTNode* raizAST);
void verifica_concatenacao_string (ASTNode* raizAST);
void verifica_tipagem(ASTNode* raizAST, char* tipo, TabelaDeSimbolos* tabela);
void teste(ASTNode* raizAST, ASTNode* raizASTpai, TabelaDeSimbolos* tabela);

#endif