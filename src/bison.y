%{
    #include "ast.h"
    #include <stdio.h>
    #include <string.h>
    
    int yylex (void);
    void yyerror (char const *);
    extern FILE *yyout;
    extern ASTNode *raiz_ast;
%}

%define parse.error verbose

%union {
    char* str;
    ASTNode* no_ast;
}

/* =================================================================== */
/* --- Declarações de Tokens --- */
/* =================================================================== */

/* Tokens que carregam um valor string (do lexer) */
%token <str> t_id t_num t_decimal
%token <str> t_int t_float t_char t_string
%token <str> t_mais t_menos t_asteristico t_barra t_and t_or t_maior t_menor t_maiorigual t_menorigual t_igualigual t_diferente t_negacao
%token <str> t_class t_palavra

/* Tokens simples que não carregam valor */
%token t_igual t_pontvirgula t_virgula t_return
%token t_parentesabri t_parentesfecha t_chaveabri t_chavefecha
%token t_vetorabri t_vetorfecha
%token t_while t_if t_else

/* =================================================================== */
/* --- Precedência e Associatividade de Operadores --- */
/* =================================================================== */
/* Menor precedência (declarado primeiro) para maior precedência */

%right t_igual                                      // 1. Atribuição (a mais fraca)
%left  t_or                                         // 2. OU lógico
%left  t_and                                        // 3. E lógico
%left  t_igualigual t_diferente                     // 4. Igualdade (==, !=)
%left  t_maior t_menor t_maiorigual t_menorigual    // 5. Comparações (<, >, <=, >=)
%left  t_mais t_menos                               // 6. Soma e Subtração
%left  t_asteristico t_barra                        // 7. Multiplicação e Divisão
%right t_negacao                                    // 8. Negação unária '!' (a mais forte)

/* =================================================================== */
/* --- Tipos dos Não-Terminais --- */
/* =================================================================== */
/* Regras que retornam um ponteiro para um nó da AST */
%type <no_ast> declaracoes declaracao declaracao_funcao declaracao_variavel
%type <no_ast> lista_comandos comandos expressao valor variavel
%type <no_ast> lista_inicializacao lista_elementos lista_membros membro declaracao_classe
%type <no_ast> lista_parametros parametro comando_return comando_if bloco_comandos comando_while chamada_funcao

/* Regra que retorna o lexema (string) do tipo */
%type <str> tipo class

/* =================================================================== */
/* --- Regras da Gramática --- */
/* =================================================================== */

/* Regra inicial: um programa é uma lista de declarações de alto nível. */

%%
    programa:
        declaracoes {
            raiz_ast = criar_no_programa($1);
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
        declaracao_variavel {$$ = $1;} |
        declaracao_classe {$$ = $1;}

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
            $$ = criar_no_funcao($1, $2, filhos, num_filhos, @1.first_line);

            if($1) free($1); 
            if($2) free($2);
        }

/* Regras para os diferentes tipos de declaração de variável. */

    declaracao_variavel:
        // ex: int x;
        tipo t_id t_pontvirgula {
            // Criamos um nó do tipo VAR_DECL, passando o tipo e o nome
            $$ = criar_no_declaracao($1, $2, @1.first_line);

            // Liberamos as strings que já foram copiadas/usadas
            if ($1) free($1);
            if ($2) free($2);
        } |
        // ex: int x = 10;
        tipo t_id t_igual expressao t_pontvirgula {
            // Criamos o nó de declaração e passamos a árvore da expressão
            // como um "filho" do nó de declaração.
            ASTNode* filhos[] = {$4};
            $$ = criar_no_declaracao_com_valor($1, $2, filhos[0], @1.first_line);

            // Liberamos as strings que já foram copiadas/usadas
            if ($1) free($1);
            if ($2) free($2);
        } |
        // ex: int v[10];
        tipo t_id t_vetorabri t_num t_vetorfecha t_pontvirgula {            
            // Criamos o nó e depois o modificamos
            ASTNode* no_vetor = criar_no_declaracao($1, $2, @1.first_line);
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

            $$ = criar_no_declaracao_vetor($1, $2, $6, @1.first_line);

            if ($1) free($1);
            if ($2) free($2);
        }

/* Regra principal para a declaração de classe */

    declaracao_classe:
        class t_id t_chaveabri lista_membros t_chavefecha {
            // $2 é o nome da classe (char*)
            // $4 é a lista ligada de nós dos membros (ASTNode*)
            $$ = criar_no_classe($2, $4, @1.first_line);
            if ($2) free($2);
        }
    
/* Regra para a lista de membros dentro da classe */

    lista_membros:
        %empty { $$ = NULL; } | 
        lista_membros membro {
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

/* Regra que define o que pode existir dentro de uma */
    membro:
        declaracao_variavel { $$ = $1; } | // Reutiliza a regra de declaração de variável! 
        declaracao_funcao   { $$ = $1; } // Reutiliza a regra de declaração de função!

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
            $$ = criar_no_declaracao($1, $2, @1.first_line);
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
        comando_return {$$ = $1;} |
        comando_if {$$ = $1;} |
        comando_while {$$ = $1;}
    comando_if:
        t_if t_parentesabri expressao t_parentesfecha bloco_comandos {
            ASTNode* filhos[] = {$3, $5}; // Filho 0: condição, Filho 1: corpo do if
            $$ = criar_no(NODE_TYPE_IF, "if", filhos, 2, NULL, @1.first_line);
        } |
        // Forma 2: if-else  
        t_if t_parentesabri expressao t_parentesfecha bloco_comandos t_else bloco_comandos {
            ASTNode* filhos[] = {$3, $5, $7}; // Filho 0: condição, Filho 1: corpo do if, Filho 2: corpo do else
            $$ = criar_no(NODE_TYPE_IF, "if-else", filhos, 3, NULL, @1.first_line);
        }
    comando_return:
        // Caso 1: return com uma expressão, ex: return x + 5;
        t_return expressao t_pontvirgula {
            ASTNode* filhos[] = {$2};
            $$ = criar_no(NODE_TYPE_RETURN, "return", filhos, 1, NULL, @1.first_line);
        }
        // Caso 2: return sem nada, ex: return;
        | t_return t_pontvirgula {
            $$ = criar_no(NODE_TYPE_RETURN, "return", NULL, 0, NULL, @1.first_line);
        }
    comando_while:
        t_while t_parentesabri expressao t_parentesfecha bloco_comandos {
            // Muito parecido com o 'if' simples
            ASTNode* filhos[] = {$3, $5}; // Filho 0: condição, Filho 1: corpo do loop
            $$ = criar_no(NODE_TYPE_WHILE, "while", filhos, 2, NULL, @1.first_line);
        }
    bloco_comandos:
        t_chaveabri lista_comandos t_chavefecha {
            // O resultado é a própria lista de comandos
            $$ = $2;
        }

/* Regras de Expressão (precedência definida no topo) */

    expressao:
        valor {$$ = $1;} |
        variavel t_igual expressao {
            ASTNode* filhos[] = {$1, $3};
            $$ = criar_no(NODE_TYPE_ATRIBUICAO, "=", filhos, 2, NULL, @1.first_line);
        } |
        expressao t_mais expressao {
            ASTNode *filhos[] = {$1, $3};
            $$ = criar_no(NODE_TYPE_OPERACAO_BINARIA, $2, filhos, 2, NULL, @1.first_line);
            if ($2) free($2);
        } |
        expressao t_menos expressao {
            ASTNode *filhos[] = {$1, $3};
            $$ = criar_no(NODE_TYPE_OPERACAO_BINARIA, $2, filhos, 2, NULL, @1.first_line);
            if ($2) free($2);
        } |
        expressao t_asteristico expressao {
            ASTNode *filhos[] = {$1, $3};
            $$ = criar_no(NODE_TYPE_OPERACAO_BINARIA, $2, filhos, 2, NULL, @1.first_line);
            if ($2) free($2);
        } |
        expressao t_barra expressao {
            ASTNode *filhos[] = {$1, $3};
            $$ = criar_no(NODE_TYPE_OPERACAO_BINARIA, $2, filhos, 2, NULL, @1.first_line);
            if ($2) free($2);
        } |
        expressao t_maior expressao {
            ASTNode *filhos[] = {$1, $3};
            $$ = criar_no(NODE_TYPE_OPERACAO_BINARIA, $2, filhos, 2, NULL, @1.first_line);
            if ($2) free($2);
        } |
        expressao t_menor expressao {
            ASTNode *filhos[] = {$1, $3};
            $$ = criar_no(NODE_TYPE_OPERACAO_BINARIA, $2, filhos, 2, NULL, @1.first_line);
            if ($2) free($2);
        } |
        expressao t_maiorigual expressao {
            ASTNode *filhos[] = {$1, $3};
            $$ = criar_no(NODE_TYPE_OPERACAO_BINARIA, $2, filhos, 2, NULL, @1.first_line);
            if ($2) free($2);
        } |
        expressao t_menorigual expressao {
            ASTNode *filhos[] = {$1, $3};
            $$ = criar_no(NODE_TYPE_OPERACAO_BINARIA, $2, filhos, 2, NULL, @1.first_line);
            if ($2) free($2);
        } |
        expressao t_igualigual expressao {
            ASTNode *filhos[] = {$1, $3};
            $$ = criar_no(NODE_TYPE_OPERACAO_BINARIA, $2, filhos, 2, NULL, @1.first_line);
            if ($2) free($2);
        } |
        expressao t_diferente expressao {
            ASTNode *filhos[] = {$1, $3};
            $$ = criar_no(NODE_TYPE_OPERACAO_BINARIA, $2, filhos, 2, NULL, @1.first_line);
            if ($2) free($2);
        } |
        expressao t_and expressao {
            ASTNode *filhos[] = {$1, $3};
            $$ = criar_no(NODE_TYPE_OPERACAO_BINARIA, $2, filhos, 2, NULL, @1.first_line);
            if ($2) free($2);
        } |
        expressao t_or expressao {
            ASTNode *filhos[] = {$1, $3};
            $$ = criar_no(NODE_TYPE_OPERACAO_BINARIA, $2, filhos, 2, NULL, @1.first_line);
            if ($2) free($2);
        } |
        t_negacao expressao {
            ASTNode *filhos[] = {$2};
            $$ = criar_no(NODE_TYPE_OPERACAO_UNARIA, $1, filhos, 1, NULL, @1.first_line);
            if ($2) free($1);
        } |
        t_menos expressao {
            ASTNode *filhos[] = {$2};
            $$ = criar_no(NODE_TYPE_OPERACAO_UNARIA, $1, filhos, 1, NULL, @1.first_line);
            if ($2) free($1);
        }
        
/* Um tipo de dado. */

    tipo:
        t_int {$$ = $1;} |
        t_float {$$ = $1;} |
        t_char {$$ = $1;} |
        t_string {$$ = $1;} |
        t_id {$$ = $1;}

    class:
        t_class {$$ = $1;}

/* Uma variável é um identificador. */

    variavel:
        t_id {
            $$ = criar_no(NODE_TYPE_IDENTIFICADOR, $1, NULL, 0, NULL, @1.first_line);
            if ($1) free($1); // Libera o str do token
        }

/* Um valor pode ser um número, uma variável, ou uma expressão entre parênteses. */

    valor:
        t_num {
            $$ = criar_no(NODE_TYPE_NUMERO, $1, NULL, 0, NULL, @1.first_line);
            if ($1) free($1); // Libera o str do token, pois criar_no fez strdup
        } |
        t_decimal {
            $$ = criar_no(NODE_TYPE_NUMERO, $1, NULL, 0, NULL, @1.first_line);
            if ($1) free($1); // Libera o str do token, pois criar_no fez strdup
        } |
        t_palavra {
            $$ = criar_no(NODE_TYPE_STRING, $1, NULL, 0, NULL, @1.first_line);
            if ($1) free($1); // Libera o str do token, pois criar_no fez strdup
        } |
        variavel {$$ = $1;} |
        t_parentesabri expressao t_parentesfecha { $$ = $2; } |
        lista_inicializacao {$$ = criar_no_literal_vetor($1);} |
        chamada_funcao {$$ = $1;}

    chamada_funcao:
    // Caso 1: Chamada sem argumentos. Ex: teste()
    t_id t_parentesabri t_parentesfecha {
        // $1 é o nome da função (char*)
        $$ = criar_no(NODE_TYPE_FUNCAO_CALL, $1, NULL, 0, NULL, @1.first_line);
        if ($1) free($1);
    }
    // Caso 2: Chamada com argumentos. Ex: soma(10, x)
    | t_id t_parentesabri lista_elementos t_parentesfecha {
        // $1 é o nome da função (char*)
        // $3 é a lista ligada de nós das expressões dos argumentos
        $$ = criar_no_chamada_funcao($1, $3);
        if ($1) free($1);
    }

%%