#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <llvm-c/Core.h>
#include <llvm-c/Types.h>

#include "ast.h"
#include "symbol_table.h"
#include "semantico.h"
#include "codegen.h"
#include "bison.tab.h"

extern long linha;
extern FILE *yyin;
extern FILE *yyout;
extern char *yytext;
ASTNode *raiz_ast = NULL;

void yyerror (char* const s){
    fprintf(stderr, "Erro de Sintaxe na Linha %ld: '%s'\n", linha, s);
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
        TabelaDeSimbolos tabela;
        inicializar_tabela(&tabela);
        
        teste(raiz_ast, NULL, &tabela);

        imprimir_tabela_simbolos(tabela);

        GeradorDeCodigo gerador = inicializar_codegen("compilador");

        gerar_codigo(raiz_ast, &gerador, &tabela, NULL);

        printf("\n--- LLVM IR Gerado (impresso no console) ---\n");

        // Esta função imprime todo o conteúdo do módulo no terminal
        LLVMDumpModule(gerador.modulo);

        LLVMDisposeBuilder(gerador.builder);
        LLVMDisposeModule(gerador.modulo);

    } else { // Erro
        printf("Parsing falhou.\n");
    }
    return 0;
}