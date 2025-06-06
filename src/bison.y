%{
    #include "ast.h"
    #include <stdio.h>
    
    int yylex (void);
    void yyerror (char const *);
    extern FILE *yyout;
    extern long linha;
    extern ASTNode *raiz_ast;
%}

%union {
    char* str;
    ASTNode* no_ast;
}

/* operadores lógicos */
%token <str> t_igual t_mais t_menos t_asteristico t_barra t_and t_or

/* tipos */
%token t_int t_float t_char t_vetorabri t_vetorfecha

/* valores de atribuição para tipos*/
%token <str> t_num t_palavra t_palavranum t_decimal t_varname 

/* Tokens de repetição e condicionais */
%token t_for t_while t_if t_else t_switch t_chaveabri t_chavefecha t_parentesabri t_parentesfecha t_pontvirgula 

/* Tokens classe e função */
%token <str> t_class t_func t_id

/* token de espacamento  novalinha, tabulação  e espaço em branco*/
%token t_espaco t_novalinha

%type <no_ast> valor expressao lista_comandos comandos
%type <str> operador

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
  t_igual | t_mais | t_menos | t_asteristico | t_barra | t_and | t_or
tipos:
  t_int | t_float  | t_char | t_vetorabri | t_vetorfecha 
valorespermitidos:
  t_num | t_palavra | t_palavranum | t_decimal | t_varname 
controle:
  t_for | t_while | t_if | t_else | t_switch | t_chaveabri | t_chavefecha | t_parentesabri | t_parentesfecha | t_pontvirgula 
classefuncao:
  t_class | t_func | t_id
%%*/

%%
    programa:
        lista_comandos {
            ASTNode* filhos_do_programa[] = {$1};
             // Se a lista for vazia ($1 é NULL), o nó programa não terá filhos.
            int num_filhos = ($1 != NULL) ? 1 : 0;

            // Criamos o nó raiz e o atribuímos à variável global.
            raiz_ast = criar_no(NODE_TYPE_PROGRAMA, "Programa", filhos_do_programa, num_filhos, NULL);
        }
    lista_comandos:
        %empty {$$ = NULL;} | 
        lista_comandos comandos {
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
        expressao t_pontvirgula {$$ = $1;} 
    expressao:
        valor {$$ = $1;}|
        expressao operador valor {
            // $2 é o lexema de T_MAIS (ex: "+")
            ASTNode *teste[] = {$1, $3};
            $$ = criar_no(NODE_TYPE_OPERACAO_BINARIA, $2, teste, 2, NULL);
            // Nota: $2 (de T_MAIS) também é um char* (str), então precisamos liberá-lo
            // se não for usado diretamente pelo criar_no ou se criar_no fizer strdup.
            // Se criar_no faz strdup (como no exemplo), o $2 original pode ser liberado
            // aqui se o Flex o alocou. Se o Flex não alocou (ex: é um ponteiro para yytext),
            // não precisa liberar. Assumindo que T_MAIS <str> fez strdup no Flex:
            if ($2) free($2);
        }
    valor:
        t_num {
            $$ = criar_no(NODE_TYPE_NUMERO, $1, NULL, 0, NULL);
            if ($1) free($1); // Libera o str do token, pois criar_no fez strdup
        } |
        t_id {
            $$ = criar_no(NODE_TYPE_IDENTIFICADOR, $1, NULL, 0, NULL);
            if ($1) free($1); // Libera o str do token
        }
    operador:
        t_mais {$$ = $1;}

%%