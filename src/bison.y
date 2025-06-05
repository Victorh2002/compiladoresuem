%{
  #include <stdio.h>
  #include "ast.h"
  int yylex (void);
  void yyerror (char const *);
  extern FILE *yyout;
  ASTNode *raiz_ast = NULL;

  long linha=1;
%}

%union {
    char* str;
}

/* operadores lógicos */
%token t_igual t_mais t_menos t_asteristico t_barra

/* tipos */
%token t_int t_float t_char t_vetorabri t_vetorfecha

/* valores de atribuição para tipos*/
%token t_num t_palavra t_palavranum t_decimal t_varname 

/* Tokens de repetição e condicionais */
%token t_for t_while t_if t_else t_switch t_chaveabri t_chavefecha t_parentesabri t_parentesfecha t_pontvirgula 

/* Tokens classe e função */
%token t_class t_func t_id

/* token de espacamento  novalinha, tabulação  e espaço em branco*/
%token t_espaco t_novalinha

%type <str> valor expressao operador lista_comandos comandos t_id t_mais t_num

/*
%% //Gramática deste ponto para baixo
inicio:
  %empty| inicio programa 
programa:
  operadores   {fprintf(yyout, "[%d] Achou um operador (%s)\n", linha, $1);} |
  tipos   {fprintf(yyout, "[%d] Achou um tipo (%s)\n", linha, $1);} | 
  valorespermitidos   {fprintf(yyout, "[%d] Achou um valorespermitidos (%s)\n", linha, $1);}| 
  controle  {fprintf(yyout, "[%d] Achou um controle (%s)\n", linha, $1);} | 
  classefuncao  {fprintf(yyout, "[%d] Achou um classefuncao_outras (%s)\n", linha, $1);}
operadores:
  t_igual | t_mais | t_menos | t_asteristico | t_barra 
tipos:
  t_int | t_float  | t_char | t_vetorabri | t_vetorfecha 
valorespermitidos:
  t_num | t_palavra | t_palavranum | t_decimal | t_varname 
controle:
  t_for | t_while | t_if | t_else | t_switch | t_chaveabri | t_chavefecha | t_parentesabri | t_parentesfecha | t_pontvirgula 
classefuncao:
  t_class | t_func | t_id
%%
*/

%%
    inicio:
        lista_comandos {
            raiz_ast = $1; // Guarda a raiz da AST construída
            printf("Parsing concluído com sucesso.\n");
            if (raiz_ast) {
                printf("Imprimindo AST:\n");
                imprimir_ast(raiz_ast, 0);
                // Aqui você também liberaria a memória da AST com uma função free_ast(raiz_ast);
            }
        }
    lista_comandos:
        %empty| lista_comandos comandos {
            // Adiciona o novo comando à lista (simplesmente encadeando)
            ASTNode *lista = $1;
            if (lista == NULL) {
                $$ = $2; // $2 é o novo comando
            } else {
                ASTNode *atual = lista;
                while (atual->proximo_comando != NULL) {
                    atual = atual->proximo_comando;
                }
                atual->proximo_comando = $2; // $2 é o novo comando
                $$ = lista;
            }
        }
    comandos:
        expressao {$$ = $1;} |
    expressao:
        valor {$$ = $1;}|
        expressao operador valor {
            // $2 é o lexema de T_MAIS (ex: "+")
            $$ = criar_no(NODE_TYPE_OPERACAO_BINARIA, $2, $1, $3, NULL);
            // Nota: $2 (de T_MAIS) também é um char* (sval), então precisamos liberá-lo
            // se não for usado diretamente pelo criar_no ou se criar_no fizer strdup.
            // Se criar_no faz strdup (como no exemplo), o $2 original pode ser liberado
            // aqui se o Flex o alocou. Se o Flex não alocou (ex: é um ponteiro para yytext),
            // não precisa liberar. Assumindo que T_MAIS <sval> fez strdup no Flex:
            if ($2) free($2);
        }
    valor:
        t_num {
            $$ = criar_no(NODE_TYPE_NUMERO, $1, NULL, NULL, NULL);
            if ($1) free($1); // Libera o sval do token, pois criar_no fez strdup
        } |
        t_id {
            $$ = criar_no(NODE_TYPE_IDENTIFICADOR, $1, NULL, NULL, NULL);
            if ($1) free($1); // Libera o sval do token
        }
    operador:
        t_mais {$$ = $1;}

%%

