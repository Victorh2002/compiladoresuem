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
%token <str> t_int t_float t_char t_vetorabri t_vetorfecha

/* valores de atribuição para tipos*/
%token <str> t_num t_palavra t_palavranum t_decimal t_varname 

/* Tokens de repetição e condicionais */
%token <str> t_for t_while t_if t_else t_switch t_chaveabri t_chavefecha t_parentesabri t_parentesfecha t_pontvirgula 

/* Tokens classe e função */
%token <str> t_class t_func t_id

/* token de espacamento  novalinha, tabulação  e espaço em branco*/
%token t_espaco t_novalinha

%right t_igual
%left  t_mais t_menos
%left  t_asteristico t_barra

%type <no_ast> valor expressao lista_comandos comandos declaracoes declaracao declaracao_funcao variavel
%type <str> tipo programa //operadores tipos valorespermitidos controle classefuncao

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
        declaracoes {
            ASTNode* filhos_do_programa[] = {$1};
             // Se a lista for vazia ($1 é NULL), o nó programa não terá filhos.
            int num_filhos = ($1 != NULL) ? 1 : 0;

            // Criamos o nó raiz e o atribuímos à variável global.
            raiz_ast = criar_no(NODE_TYPE_PROGRAMA, "Programa", filhos_do_programa, num_filhos, NULL);
        }
    declaracoes:
        %empty {$$ = NULL;} | 
        declaracoes declaracao {
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
    declaracao:
        declaracao_funcao { $$ = $1; }
    declaracao_funcao:
        tipo t_id t_parentesabri t_parentesfecha t_chaveabri lista_comandos t_chavefecha {
            // NESTE PONTO:
            // $2 é o nome da função (o lexema de t_id, ex: "main")
            // $6 é o corpo inteiro da função! É o ASTNode* que sua regra 'lista_comandos' retornou.

            // Agora você cria um nó específico para a função
            ASTNode* filhos_da_funcao[] = {$6};
            int num_filhos = ($6 != NULL) ? 1 : 0;
            
            // Criamos um nó para a declaração da função, usando o nome dela ($2)
            $$ = criar_no(NODE_TYPE_FUNCAO_DECL, $2, filhos_da_funcao, num_filhos, NULL);

            // Liberamos o nome da função que não será mais usado diretamente
            // free($2); // O criar_no já fez uma cópia com strdup
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
        valor {$$ = $1;} |
        variavel t_igual expressao {
            // Ação para criar o nó de atribuição...
            // $1 é o NÓ do identificador retornado pela regra 'variavel'
            ASTNode* filhos[] = {$1, $3};
            $$ = criar_no(NODE_TYPE_ATRIBUICAO, "=", filhos, 2, NULL);
        } |
        expressao t_mais valor {
            // $2 é o lexema de T_MAIS (ex: "+")
            ASTNode *filhos[] = {$1, $3};
            $$ = criar_no(NODE_TYPE_OPERACAO_BINARIA, $2, filhos, 2, NULL);
            if ($2) free($2);
        }
    tipo:
        t_int {$$ = $1;} |
        t_float {$$ = $1;} |
        t_char {$$ = $1;}
    variavel:
        t_id {
            $$ = criar_no(NODE_TYPE_IDENTIFICADOR, $1, NULL, 0, NULL);
            if ($1) free($1); // Libera o str do token
        }
    valor:
        t_num {
            $$ = criar_no(NODE_TYPE_NUMERO, $1, NULL, 0, NULL);
            if ($1) free($1); // Libera o str do token, pois criar_no fez strdup
        } |
        t_decimal {
            $$ = criar_no(NODE_TYPE_NUMERO, $1, NULL, 0, NULL);
            if ($1) free($1); // Libera o str do token, pois criar_no fez strdup
        } |
        variavel {$$ = $1;} |
        t_parentesabri expressao t_parentesfecha { $$ = $2; }

%%