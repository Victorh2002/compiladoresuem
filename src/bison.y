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

/* =================================================================== */
/* --- Declarações de Tokens --- */
/* =================================================================== */

/* Tokens que carregam um valor string (do lexer) */
%token <str> t_id t_num t_decimal
%token <str> t_int t_float t_char
%token <str> t_mais t_menos t_asteristico t_barra t_and t_or
%token <str> t_class t_func t_palavra

/* Tokens simples que não carregam valor */
%token t_igual t_pontvirgula t_virgula t_return
%token t_parentesabri t_parentesfecha t_chaveabri t_chavefecha
%token t_vetorabri t_vetorfecha
%token t_for t_while t_if t_else t_switch

/* =================================================================== */
/* --- Precedência e Associatividade de Operadores --- */
/* =================================================================== */
/* Menor precedência (declarado primeiro) para maior precedência */
%right t_igual
%left  t_mais t_menos
%left  t_asteristico t_barra

/* =================================================================== */
/* --- Tipos dos Não-Terminais --- */
/* =================================================================== */
/* Regras que retornam um ponteiro para um nó da AST */
%type <no_ast> declaracoes declaracao declaracao_funcao declaracao_variavel
%type <no_ast> lista_comandos comandos expressao valor variavel
%type <no_ast> lista_inicializacao lista_elementos
%type <no_ast> lista_parametros parametro comando_return

/* Regra que retorna o lexema (string) do tipo */
%type <str> tipo

/* =================================================================== */
/* --- Regras da Gramática --- */
/* =================================================================== */

/* Regra inicial: um programa é uma lista de declarações de alto nível. */

%%
    programa:
        declaracoes {
            ASTNode* filhos_do_programa[] = {$1};
            int num_filhos = ($1 != NULL) ? 1 : 0;
            raiz_ast = criar_no(NODE_TYPE_PROGRAMA, "Programa", filhos_do_programa, num_filhos, NULL);
        }

/* Constrói uma lista ligada de declarações. */

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

/* Uma declaração pode ser uma função ou uma variável (no futuro, uma classe, etc.). */

    declaracao:
        declaracao_funcao {$$ = $1;} |
        declaracao_variavel {$$ = $1;}

/* Regra para uma declaração de função completa. */

    declaracao_funcao:
        tipo t_id t_parentesabri lista_parametros t_parentesfecha t_chaveabri lista_comandos t_chavefecha {
            // $2: nome, $4: parâmetros, $7: corpo

            int num_filhos = 0;
            if ($4 != NULL) {
                num_filhos++;
            }
            if ($7 != NULL) {
                num_filhos++;
            }
            ASTNode* filhos[num_filhos];
            num_filhos = 0;

            if ($4 != NULL) {
                filhos[num_filhos] = $4;
                num_filhos++;
            }
            if ($7 != NULL) {
                filhos[num_filhos] = $7;
                num_filhos++;
            }
            
            // Criamos um nó para a declaração da função, usando o nome dela ($2)
            $$ = criar_no(NODE_TYPE_FUNCAO_DECL, $2, filhos, num_filhos, NULL);

            if($1) free($1); 
            if($2) free($2);
        }

/* Regras para os diferentes tipos de declaração de variável. */

    declaracao_variavel:
        // ex: int x;
        tipo t_id t_pontvirgula {
            // Criamos um nó do tipo VAR_DECL, passando o tipo e o nome
            $$ = criar_no_declaracao($1, $2);

            // Liberamos as strings que já foram copiadas/usadas
            if ($1) free($1);
            if ($2) free($2);
        } |
        // ex: int x = 10;
        tipo t_id t_igual expressao t_pontvirgula {
            // Criamos o nó de declaração e passamos a árvore da expressão
            // como um "filho" do nó de declaração.
            ASTNode* filhos[] = {$4};
            $$ = criar_no_declaracao_com_valor($1, $2, filhos[0]);

            // Liberamos as strings que já foram copiadas/usadas
            if ($1) free($1);
            if ($2) free($2);
        } |
        // ex: int v[10];
        tipo t_id t_vetorabri t_num t_vetorfecha t_pontvirgula {            
            // Criamos o nó e depois o modificamos
            ASTNode* no_vetor = criar_no_declaracao($1, $2);
            no_vetor->is_array = 1; // Marca como vetor
            no_vetor->array_size = atoi($4); // Converte a string do tamanho para inteiro

            $$ = no_vetor;

            if ($1) free($1);
            if ($2) free($2);
            if ($4) free($4); // Libera a string do tamanho
        } |
        // ex: int v[] = [1, 2];
        tipo t_id t_vetorabri t_vetorfecha t_igual lista_inicializacao t_pontvirgula {
            // $1 é o tipo (char*), $2 é o nome (char*),
            // $6 é a lista ligada de nós da AST com os valores da lista

            $$ = criar_no_declaracao_vetor($1, $2, $6);

            if ($1) free($1);
            if ($2) free($2);
        }

/* Regras para parsear listas de inicialização de vetores: [el1, el2, ...] */

    lista_inicializacao:
        t_vetorabri lista_elementos t_vetorfecha {$$ = $2;}
    lista_elementos:
         // Caso base: uma lista com um único elemento
        expressao {
            // O primeiro elemento se torna a cabeça da lista
            $$ = $1;
            // Importante: garantir que o ponteiro para o próximo seja nulo
            if ($1) {
                $1->proximo_comando = NULL;
            }
        } |
        // Caso recursivo: uma lista já existente, uma vírgula, e uma nova expressão
        lista_elementos t_virgula expressao {
            // Pega a lista existente ($1) e anexa a nova expressão ($3) no final.
            // Podemos reutilizar a mesma lógica da sua 'lista_comandos'.
            ASTNode *lista = $1;
            ASTNode *novo_elemento = $3;
            
            // Garante que o novo elemento é o fim da sua própria "sub-lista"
            if (novo_elemento) {
                novo_elemento->proximo_comando = NULL;
            }

            if (lista == NULL) {
                $$ = novo_elemento;
            } else {
                ASTNode *atual = lista;
                while (atual->proximo_comando != NULL) {
                    atual = atual->proximo_comando;
                }
                atual->proximo_comando = novo_elemento;
                $$ = lista; // Retorna a cabeça da lista original
            }
        }

/* Regras para parsear listas de parâmetros de função: (p1, p2, ...) */

    parametro:
        tipo t_id {
            $$ = criar_no_declaracao($1, $2);
            if ($1) free($1);
            if ($2) free($2);
        }
    lista_parametros:
        %empty { $$ = NULL; } | // Uma função pode não ter parâmetros 
        parametro { // O caso base: apenas um parâmetro
            $$ = $1;
            if ($1) $1->proximo_comando = NULL;
        } | 
        lista_parametros t_virgula parametro { // Caso recursivo
            // Lógica para adicionar à lista ligada, igual à que você já usa
            ASTNode *lista = $1;
            ASTNode *novo_elemento = $3;
            
            // Garante que o novo elemento é o fim da sua própria "sub-lista"
            if (novo_elemento) {
                novo_elemento->proximo_comando = NULL;
            }

            if (lista == NULL) {
                $$ = novo_elemento;
            } else {
                ASTNode *atual = lista;
                while (atual->proximo_comando != NULL) {
                    atual = atual->proximo_comando;
                }
                atual->proximo_comando = novo_elemento;
                $$ = lista; // Retorna a cabeça da lista original
            }
        }

/* Uma lista de comandos é o corpo de uma função. */

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

/* Um comando pode ser uma declaração, uma expressão ou um return. */

    comandos:
        declaracao_variavel {$$ = $1;} |
        expressao t_pontvirgula {$$ = $1;} |
        comando_return { $$ = $1; }
    comando_return:
        // Caso 1: return com uma expressão, ex: return x + 5;
        t_return expressao t_pontvirgula {
            ASTNode* filhos[] = {$2};
            $$ = criar_no(NODE_TYPE_RETURN, "return", filhos, 1, NULL);
        }
        // Caso 2: return sem nada, ex: return;
        | t_return t_pontvirgula {
            $$ = criar_no(NODE_TYPE_RETURN, "return", NULL, 0, NULL);
        }

/* Regras de Expressão (precedência definida no topo) */

    expressao:
        valor {$$ = $1;} |
        variavel t_igual expressao {
            ASTNode* filhos[] = {$1, $3};
            $$ = criar_no(NODE_TYPE_ATRIBUICAO, "=", filhos, 2, NULL);
        } |
        expressao t_mais valor {
            ASTNode *filhos[] = {$1, $3};
            $$ = criar_no(NODE_TYPE_OPERACAO_BINARIA, $2, filhos, 2, NULL);
            if ($2) free($2);
        } |
        expressao t_menos valor {
            ASTNode *filhos[] = {$1, $3};
            $$ = criar_no(NODE_TYPE_OPERACAO_BINARIA, $2, filhos, 2, NULL);
            if ($2) free($2);
        } |
        expressao t_asteristico valor {
            ASTNode *filhos[] = {$1, $3};
            $$ = criar_no(NODE_TYPE_OPERACAO_BINARIA, $2, filhos, 2, NULL);
            if ($2) free($2);
        } |
        expressao t_barra valor {
            ASTNode *filhos[] = {$1, $3};
            $$ = criar_no(NODE_TYPE_OPERACAO_BINARIA, $2, filhos, 2, NULL);
            if ($2) free($2);
        }
        
/* Um tipo de dado. */

    tipo:
        t_int {$$ = $1;} |
        t_float {$$ = $1;} |
        t_char {$$ = $1;}

/* Uma variável é um identificador. */

    variavel:
        t_id {
            $$ = criar_no(NODE_TYPE_IDENTIFICADOR, $1, NULL, 0, NULL);
            if ($1) free($1); // Libera o str do token
        }

/* Um valor pode ser um número, uma variável, ou uma expressão entre parênteses. */

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
        t_parentesabri expressao t_parentesfecha { $$ = $2; } |
        lista_inicializacao {$$ = criar_no_literal_vetor($1);}

%%