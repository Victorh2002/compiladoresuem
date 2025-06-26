#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "symbol_table.h"
#include "semantico.h"
#include "bison.tab.h"

extern long linha;
extern FILE *yyin;
extern FILE *yyout;
extern char *yytext;
Symbol* tabela_de_simbolos = NULL; 
ASTNode *raiz_ast = NULL;

void yyerror (char const s){
    fprintf(stderr, "Erro de Sintaxe na Linha %ld: Token inesperado '%s'\n", linha, yytext);
}

int main(int argc, char *arqv[]){
	for(int i = 1; i < argc ; i++){
		if( strcmp(arqv[i], "-e") == 0 && i<argc){
			yyin = fopen(arqv[i+1],"r");
			if(!yyin){
				printf("Não foi possível abrir o arquivo %s\n",arqv[i+1]);
				exit(-1);
			}
			i = i+1;
		}else if(strcmp(arqv[i],"-s") == 0 && i<argc){
			yyout = fopen(arqv[i+1],"w");
			i = i+1;
		}
	}

    if (yyparse() == 0) { // Sucesso
        printf("Parsing concluído com sucesso.\n");
        if (raiz_ast) {
            imprimir_ast(raiz_ast, 0);
            // free_ast(raiz_ast);
        }
        preenche_tabela(raiz_ast, NULL);
        imprimir_tabela_simbolos();

    } else { // Erro
        printf("Parsing falhou.\n");
    }
    return 0;
}