#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "ast.h"
#include "symbol_table.h"
#include "semantico.h"

bool erro_semantico = false;
bool tipo_igual = true;
bool expressao_logica = false;
char* tipo_funcao_atual = NULL;
bool encontrou_return = false;
bool concatenacao_string = true;
extern long linha;

void verificar_chamada_especial(ASTNode* no_chamada, TabelaDeSimbolos* tabela) {
    const char* nome_funcao = no_chamada->valor;

    // --- Regras para a função especial 'scanf' ---
    if (strcmp(nome_funcao, "scanf") == 0) {
        
        // REGRA: scanf deve ter exatamente um argumento.
        if (no_chamada->child_count != 1) {
            fprintf(stderr, "Erro Semântico (linha %ld): A função 'scanf' espera exatamente 1 argumento, mas recebeu %d.\n",
                    no_chamada->linha, no_chamada->child_count);
            exit(1); // Para a execução
        }

        if (no_chamada->filhos[0]->type != NODE_TYPE_IDENTIFICADOR)
        {
            fprintf(stderr, "Erro Semântico (linha %ld): A função 'scanf' aceita apenas variáveis no seu argumento!\n", no_chamada->linha);
            exit(1); // Para a execução
        }

        Symbol* id = buscar_simbolo(tabela, no_chamada->filhos[0]->valor);

        if (!id)
        {
            fprintf(stderr, "Erro Semântico (linha %ld): A variável '%s' não existe!\n", no_chamada->linha, no_chamada->filhos[0]->valor);
            exit(1); // Para a execução
        }
        
        return; // A verificação para scanf passou.
    }

    // --- Regras para a função especial 'printf' ---
    if (strcmp(nome_funcao, "printf") == 0) {

        // REGRA: printf também deve ter exatamente um argumento.
        if (no_chamada->child_count != 1) {
            fprintf(stderr, "Erro Semântico (linha %ld): A função 'printf' espera exatamente 1 argumento, mas recebeu %d.\n",
                    no_chamada->linha, no_chamada->child_count);
            exit(1); // Para a execução
        }

        verifica_concatenacao_string(no_chamada);

        if (concatenacao_string == false)
        {
            fprintf(stderr, "Erro Semântico (linha %ld): A função 'printf' aceita apenas o simbolo '+' no seu argumento para a concatenaçao de string!\n", no_chamada->linha);
            exit(1); // Para a execução
        }
        
        return; // A verificação para printf passou.
    }
}

void verifica_parametros_chamada_funcao(ASTNode* raizAST, Param* parametros, TabelaDeSimbolos* tabela) {
    if (raizAST == NULL) return;

    Param* param = parametros;

    // Definir o índice inicial dos argumentos no AST
    int start = 0;

    if (raizAST->type == NODE_TYPE_METHOD_CALL) {
        // Em chamadas de método, filho 0 é o objeto, argumentos começam em 1
        start = 1;
    } else if (raizAST->type == NODE_TYPE_FUNCAO_CALL) {
        // Em chamadas de função, filhos são argumentos a partir do 0
        start = 0;
    } else {
        printf("Tipo de chamada desconhecido para verificação de parâmetros\n");
        exit(1);
    }

    int argumento_index = 1; // Para mensagens de erro, contar argumentos começando de 1

    for (int i = start; i < raizAST->child_count; i++) {
        if (param == NULL) {
            printf("A chamada tem mais argumentos do que os parâmetros esperados!\n");
            exit(1);
        }

        tipo_igual = true; // reset flag
        expressao_logica = false;

        verifica_tipagem(raizAST->filhos[i], param->tipo, tabela);
        verifica_tipo_expressao(raizAST->filhos[i]);

        if (!tipo_igual) {
            printf("Erro: tipo do argumento %d não confere com o tipo do parâmetro. Esperado: '%s'\n", argumento_index, param->tipo);
            exit(1);
        }

        if (expressao_logica) {
            printf("Erro: Expressão lógica usada em argumento do parâmetro %d\n", argumento_index);
            exit(1);
        }

        param = param->proximo;
        argumento_index++;
    }

    if (param != NULL) {
        printf("A chamada tem menos argumentos do que os parâmetros esperados!\n");
        exit(1);
    }
}

void verifica_concatenacao_string (ASTNode* raizAST) {
    if (raizAST == NULL) {
        return;
    }

    if (raizAST->type == NODE_TYPE_OPERACAO_BINARIA)
    {
        if (strcmp(raizAST->valor, "+") != 0)
        {
            concatenacao_string = false;
        }
    }

    for (int i = 0; i < raizAST->child_count; i++)
    {      
        verifica_concatenacao_string(raizAST->filhos[i]);
    }    
}

void verifica_tipo_expressao(ASTNode* raizAST) {
    if (raizAST == NULL) {
        return;
    }

    if (raizAST->type == NODE_TYPE_OPERACAO_BINARIA)
    {
        if (strcmp(raizAST->valor, "+") != 0 && strcmp(raizAST->valor, "-") != 0 && strcmp(raizAST->valor, "*") != 0 && strcmp(raizAST->valor, "/") != 0)
        {
            expressao_logica = true;
        }
    }

    for (int i = 0; i < raizAST->child_count; i++)
    {      
        verifica_tipo_expressao(raizAST->filhos[i]);
    }    
}

void verifica_tipagem(ASTNode* raizAST, char* tipo_esperado, TabelaDeSimbolos* tabela) {
    if (raizAST == NULL) return;

    // Se for chamada de método ou função, verifica parâmetros e usa tipo retorno
    if (raizAST->type == NODE_TYPE_METHOD_CALL || raizAST->type == NODE_TYPE_FUNCAO_CALL) {
        // Buscar símbolo do método/função (para obter tipo retorno e parâmetros)
        Symbol* sym_metodo = buscar_simbolo(tabela, raizAST->valor);
        if (!sym_metodo) {
            printf("Método/função '%s' não encontrado.\n", raizAST->valor);
            exit(1);
        }

        // Verifica parâmetros da chamada
        verifica_parametros_chamada_funcao(raizAST, sym_metodo->parametros, tabela);

        // Agora compara o tipo de retorno da função/método com o tipo esperado
        if (!(strcmp(sym_metodo->tipo, tipo_esperado) == 0 || 
             (strcmp(sym_metodo->tipo, "int") == 0 && strcmp(tipo_esperado, "float") == 0))) 
        {
            printf("Tipo de retorno '%s' da chamada '%s' não coincide com o tipo esperado '%s'. Linha: %ld\n", 
                   sym_metodo->tipo, raizAST->valor, tipo_esperado, raizAST->linha);
            exit(1);
        }

        // Continua a verificar os filhos para validar a expressão
        for (int i = 0; i < raizAST->child_count; i++) {
            verifica_tipagem(raizAST->filhos[i], NULL, tabela); // Passa NULL pq tipo já foi validado na chamada
        }

        return; // Já validou tudo aqui
    }

    if (raizAST->type == NODE_TYPE_MEMBER_ACCESS) {
        // Buscar símbolo do método/função (para obter tipo retorno e parâmetros)
        Symbol* sym_metodo = buscar_simbolo(tabela, raizAST->valor);
        if (!sym_metodo) {
            printf("Atributo '%s' não encontrado.\n", raizAST->valor);
            exit(1);
        }

        // Agora compara o tipo de retorno da função/método com o tipo esperado
        if (!(strcmp(sym_metodo->tipo, tipo_esperado) == 0 || 
             (strcmp(sym_metodo->tipo, "int") == 0 && strcmp(tipo_esperado, "float") == 0))) 
        {
            printf("Tipo do atributo '%s' não coincide com o tipo esperado '%s'. Linha: %ld\n", raizAST->valor, tipo_esperado, raizAST->linha);
            exit(1);
        }

        // Continua a verificar os filhos para validar a expressão
        for (int i = 0; i < raizAST->child_count; i++) {
            verifica_tipagem(raizAST->filhos[i], NULL, tabela); // Passa NULL pq tipo já foi validado na chamada
        }

        return; // Já validou tudo aqui
    }

    // Se for identificador, verifica se o tipo bate
    if (raizAST->type == NODE_TYPE_IDENTIFICADOR) {
        Symbol* sym = buscar_simbolo(tabela, raizAST->valor);
        if (!sym) {
            printf("Identificador '%s' não encontrado.\n", raizAST->valor);
            exit(1);
        }

        if (sym->nome_pai)
        {
            printf("Variável '%s' pertence a classe '%s'.\n", raizAST->valor, sym->nome_pai);
            exit(1);
        }

        if (tipo_esperado != NULL) {
            if (!(strcmp(sym->tipo, tipo_esperado) == 0 ||
                 (strcmp(sym->tipo, "int") == 0 && strcmp(tipo_esperado, "float") == 0))) {
                printf("Tipo do identificador '%s' (%s) não coincide com o tipo esperado '%s'. Linha: %ld\n",
                       raizAST->valor, sym->tipo, tipo_esperado, raizAST->linha);
                exit(1);
            }
        }
    }

    // Se for número, verifica tipo numérico
    if (raizAST->type == NODE_TYPE_NUMERO) {
        bool is_float = (strchr(raizAST->valor, '.') != NULL);
        if (tipo_esperado != NULL) {
            if (is_float && strcmp(tipo_esperado, "float") != 0) {
                printf("Número decimal usado em tipo não float. Linha: %ld\n", raizAST->linha);
                exit(1);
            }
            if (!is_float && (strcmp(tipo_esperado, "int") != 0 && strcmp(tipo_esperado, "float") != 0)) {
                printf("Número inteiro usado em tipo não numérico. Linha: %ld\n", raizAST->linha);
                exit(1);
            }
        }
    }

    // Se for string, verifica tipo string/char
    if (raizAST->type == NODE_TYPE_STRING) {
        size_t len = strlen(raizAST->valor);
        if (tipo_esperado != NULL) {
            if ((strcmp(tipo_esperado, "char") == 0 && len > 1) ||
                (strcmp(tipo_esperado, "string") != 0 && strcmp(tipo_esperado, "char") != 0)) {
                printf("String usada em tipo incompatível. Linha: %ld\n", raizAST->linha);
                exit(1);
            }
        }
    }

    // Para os outros nós, apenas segue para os filhos com o tipo esperado (se existir)
    for (int i = 0; i < raizAST->child_count; i++) {
        verifica_tipagem(raizAST->filhos[i], tipo_esperado, tabela);
    }
}


void teste(ASTNode* raizAST, ASTNode* raizASTpai, TabelaDeSimbolos* tabela) {
    if (raizAST == NULL) {
        return;
    }

    int abriu_escopo = 0;

    // Inserir classes e funções no escopo global
    if (raizAST->type == NODE_TYPE_CLASSE_DECL) {
        if (strcmp(raizAST->valor, "main") == 0 || strcmp(raizAST->valor, "scanf") == 0 || strcmp(raizAST->valor, "printf") == 0)
        {
            printf("Classe não pode ter o nome de main, scanf ou printf!\n");
            exit(1);
        }
        
        inserir_simbolo(tabela, raizAST->valor, raizAST->tipo_dado,
                        SYM_CLASSE,
                        raizAST->linha, 1, 0, 0, NULL); // Global

        printf("\n==> DEBUG: Entrando em Classe '%s' (nível %d)\n",
               raizAST->valor,
               tabela->nivel_escopo_atual);
        imprimir_tabela_simbolos(*tabela);
    }
    else if (raizAST->type == NODE_TYPE_FUNCAO_DECL) {
        Symbol* func;

        if (strcmp(raizAST->valor, "main") == 0 && strcmp(raizAST->tipo_dado, "int") != 0)
        {
            printf("Função main só pode ser do tipo 'int'!\n");
            exit(1);
        }

        if (strcmp(raizAST->valor, "scanf") == 0 || strcmp(raizAST->valor, "printf") == 0)
        {
            printf("Funções não podem ter o nome de scanf ou printf!\n");
            exit(1);
        }
        
        if (raizASTpai != NULL && raizASTpai->type == NODE_TYPE_CLASSE_DECL) {
            func = inserir_simbolo(tabela, raizAST->valor, raizAST->tipo_dado,
                                  SYM_FUNCAO, raizAST->linha, 1, 0, 0, raizASTpai->valor);
        } else {
            func = inserir_simbolo(tabela, raizAST->valor, raizAST->tipo_dado,
                                  SYM_FUNCAO, raizAST->linha, 1, 0, 0, NULL);
        }
        if (!func) return;

        entrar_escopo(tabela);
        abriu_escopo = 1;

        tipo_funcao_atual = func->tipo;  // Salva tipo da função atual
        encontrou_return = false;

        // Inserir parâmetros: filhos com mesma linha da função
        for (int i = 0; i < raizAST->child_count; i++) {
            ASTNode* filho = raizAST->filhos[i];
            if (filho->linha == raizAST->linha && filho->type == NODE_TYPE_VAR_DECL) {
                Param* p = criar_parametro(filho->valor, filho->tipo_dado);
                adicionar_parametro(func, p);
            }
        }

        printf("\n==> DEBUG: Função '%s' com parâmetros inseridos (nível %d)\n", func->nome, tabela->nivel_escopo_atual);
        imprimir_tabela_simbolos(*tabela);
    }
    else if (raizAST->type == NODE_TYPE_IF || raizAST->type == NODE_TYPE_WHILE) {
        entrar_escopo(tabela);
        abriu_escopo = 1;

        printf("\n==> DEBUG: Entrando em bloco (if/while), nível %d\n", tabela->nivel_escopo_atual);
        imprimir_tabela_simbolos(*tabela);
    }
    else if (raizAST->type == NODE_TYPE_VAR_DECL) {
        Symbol* id = buscar_simbolo(tabela, raizAST->valor);

        if (id) {
            printf("Variável, atributo, função ou método com nome '%s' já existe!\n", id->nome);
            exit(1);
        }

        if (strcmp(raizAST->valor, "main") == 0 || strcmp(raizAST->valor, "scanf") == 0 || strcmp(raizAST->valor, "printf") == 0)
        {
            printf("Variável não pode ter o nome de main, scanf ou printf!\n");
            exit(1);
        }

        if (strcmp(raizAST->tipo_dado, "int") != 0 &&
            strcmp(raizAST->tipo_dado, "float") != 0 &&
            strcmp(raizAST->tipo_dado, "char") != 0 &&
            strcmp(raizAST->tipo_dado, "string") != 0) {

            Symbol* classe = buscar_simbolo(tabela, raizAST->tipo_dado);
            if (!classe) {
                printf("Classe '%s' não existe!\n", raizAST->tipo_dado);
                exit(1);
            }
        }

        if (tabela->nivel_escopo_atual != -1) {
            if (raizASTpai != NULL && raizASTpai->type == NODE_TYPE_CLASSE_DECL) {
                inserir_simbolo(tabela, raizAST->valor, raizAST->tipo_dado,
                                SYM_VARIAVEL,
                                raizAST->linha, 0, 0, 0, raizASTpai->valor);
            } else {
                inserir_simbolo(tabela, raizAST->valor, raizAST->tipo_dado,
                                SYM_VARIAVEL,
                                raizAST->linha, 0, 0, 0, NULL);
            }
        } else {
            if (raizASTpai != NULL && raizASTpai->type == NODE_TYPE_CLASSE_DECL) {
                inserir_simbolo(tabela, raizAST->valor, raizAST->tipo_dado,
                                SYM_VARIAVEL,
                                raizAST->linha, 1, 0, 0, raizASTpai->valor);
            } else {
                inserir_simbolo(tabela, raizAST->valor, raizAST->tipo_dado,
                                SYM_VARIAVEL,
                                raizAST->linha, 1, 0, 0, NULL);
            }
        }

        if (raizAST->child_count > 0) {
            verifica_tipagem(raizAST->filhos[0], raizAST->tipo_dado, tabela);
            verifica_tipo_expressao(raizAST->filhos[0]);

            if (expressao_logica == true) {
                printf("Permitido apenas operadores aritméticos em atribuições! Linha: %ld\n", raizAST->linha);
                exit(1);
            }

            if (tipo_igual == false) {
                printf("O tipo da expressão não coincide com o tipo da variável! Linha: %ld\n", raizAST->linha);
                exit(1);
            }
        }

        imprimir_tabela_simbolos(*tabela);
    }
    else if (raizAST->type == NODE_TYPE_FUNCAO_CALL) {
        // Chamada de função simples (sem objeto)
        Symbol* func = buscar_simbolo(tabela, raizAST->valor);
        if (!func && (strcmp(raizAST->valor, "scanf") != 0 && strcmp(raizAST->valor, "printf") != 0)) {
            printf("Erro: função '%s' não encontrada\n", raizAST->valor);
            exit(1);
        }
        if (func && func->nome_pai != NULL) {
            // função pertence a uma classe, não pode ser chamada diretamente sem objeto
            printf("Erro: função/método '%s' pertence à classe '%s' e não pode ser chamada sem um objeto desse tipo\n",
                func->nome, func->nome_pai);
            exit(1);
        }

        if (strcmp(raizAST->valor, "scanf") == 0 || strcmp(raizAST->valor, "printf") == 0)
        {
            verificar_chamada_especial(raizAST, tabela);
        } else
        {
            verifica_parametros_chamada_funcao(raizAST, func->parametros, tabela);
        }
    }
    else if (raizAST->type == NODE_TYPE_METHOD_CALL) {
        Symbol* membro = buscar_simbolo(tabela, raizAST->valor);
        ASTNode* objeto_node = raizAST->filhos[0];
        Symbol* objeto = buscar_simbolo(tabela, objeto_node->valor);

        if (!membro) {
            printf("Erro: membro/método '%s' não encontrado\n", raizAST->valor);
            exit(1);
        }
        if (!objeto) {
            printf("Erro: objeto '%s' não encontrado\n", objeto_node->valor);
            exit(1);
        }
        if (membro->kind != SYM_FUNCAO) {
            printf("Erro: '%s' não é um método\n", membro->nome);
            exit(1);
        }
        if (objeto->kind != SYM_VARIAVEL) {
            printf("Erro: '%s' não é uma variável (objeto)\n", objeto->nome);
            exit(1);
        }
        if (membro->nome_pai == NULL) {
            printf("Erro: método '%s' não pertence a nenhuma classe\n", membro->nome);
            exit(1);
        }
        if (strcmp(membro->nome_pai, objeto->tipo) != 0) {
            printf("Erro: método '%s' pertence à classe '%s', mas objeto é do tipo '%s'\n",
                membro->nome, membro->nome_pai, objeto->tipo);
            exit(1);
        }
        verifica_parametros_chamada_funcao(raizAST, membro->parametros, tabela);
    }
    else if (raizAST->type == NODE_TYPE_MEMBER_ACCESS) {
        Symbol* membro = buscar_simbolo(tabela, raizAST->valor);
        ASTNode* objeto_node = raizAST->filhos[0];
        Symbol* objeto = buscar_simbolo(tabela, objeto_node->valor);

        if (!membro) {
            printf("Erro: membro '%s' não encontrado\n", raizAST->valor);
            exit(1);
        }
        if (!objeto) {
            printf("Erro: objeto '%s' não encontrado\n", objeto_node->valor);
            exit(1);
        }
        if (membro->kind != SYM_VARIAVEL) {
            printf("Erro: '%s' não é uma variável (atributo)\n", membro->nome);
            exit(1);
        }
        if (objeto->kind != SYM_VARIAVEL) {
            printf("Erro: '%s' não é uma variável (objeto)\n", objeto->nome);
            exit(1);
        }
        if (membro->nome_pai == NULL) {
            printf("Erro: atributo '%s' não pertence a nenhuma classe\n", membro->nome);
            exit(1);
        }
        if (strcmp(membro->nome_pai, objeto->tipo) != 0) {
            printf("Erro: atributo '%s' pertence à classe '%s', mas objeto é do tipo '%s'\n",
                membro->nome, membro->nome_pai, objeto->tipo);
            exit(1);
        }
    }
    else if (raizAST->type == NODE_TYPE_ATRIBUICAO) {
        ASTNode* lado_esquerdo = raizAST->filhos[0];
        ASTNode* lado_direito = raizAST->filhos[1];

        int is_array = 0;
        int campo_vetor = -1;

        if (lado_esquerdo->type == NODE_TYPE_ARRAY_ACCESS) {
            campo_vetor = atoi(lado_esquerdo->filhos[1]->valor);
            lado_esquerdo = lado_esquerdo->filhos[0];
            is_array = 1;
        }

        Symbol* id = buscar_simbolo(tabela, lado_esquerdo->valor);
        if (!id) {
            printf("ID '%s' não existe!\n", lado_esquerdo->valor);
            exit(1);
        }

        int tamanho_vetor = id->array_size;

        if (campo_vetor >= tamanho_vetor && is_array == 1) {
            printf("Você está tentando acessar o campo '%d' inexistente no vetor '%s', tamanho é: %d\n",
                   campo_vetor, id->nome, tamanho_vetor);
            exit(1);
        }

        // Verificar se variável que está sendo atribuída pertence a uma classe e está sendo usada corretamente
        if (id->nome_pai != NULL) {
            // Atribuição direta só pode ser feita se estiver dentro da classe ou via objeto do tipo correto
            if ((lado_esquerdo->filhos == NULL) || (lado_esquerdo->type != NODE_TYPE_MEMBER_ACCESS && lado_esquerdo->filhos[0]->type != NODE_TYPE_METHOD_CALL)) {
                printf("Erro: acesso direto a variável membro '%s' da classe '%s' não permitido sem objeto\n",
                       id->nome, id->nome_pai);
                exit(1);
            }
        }

        // Se lado direito é chamada de função ou método, verificar parâmetros
        if (lado_direito->type == NODE_TYPE_FUNCAO_CALL) {
            Symbol* func = buscar_simbolo(tabela, lado_direito->valor);
            if (!func) {
                printf("Função '%s' não encontrada para verificação de parâmetros\n", lado_direito->valor);
                exit(1);
            }
            if (func->nome_pai != NULL) {
                printf("Erro: função/método '%s' pertence à classe '%s' e não pode ser chamada sem um objeto\n",
                       func->nome, func->nome_pai);
                exit(1);
            }
            verifica_parametros_chamada_funcao(lado_direito, func->parametros, tabela);
        }
        else if (lado_direito->type == NODE_TYPE_METHOD_CALL) {
            Symbol* membro = buscar_simbolo(tabela, lado_direito->valor);
            ASTNode* objeto_node = lado_direito->filhos[0];
            Symbol* objeto = buscar_simbolo(tabela, objeto_node->valor);

            if (!membro) {
                printf("Membro/método '%s' não encontrado\n", lado_direito->valor);
                exit(1);
            }
            if (!objeto) {
                printf("Objeto '%s' não encontrado\n", objeto_node->valor);
                exit(1);
            }
            if (membro->kind != SYM_FUNCAO) {
                printf("'%s' não é um método\n", membro->nome);
                exit(1);
            }
            if (objeto->kind != SYM_VARIAVEL) {
                printf("'%s' não é uma variável (objeto)\n", objeto->nome);
                exit(1);
            }
            if (membro->nome_pai == NULL) {
                printf("Método '%s' não pertence a nenhuma classe\n", membro->nome);
                exit(1);
            }
            if (strcmp(membro->nome_pai, objeto->tipo) != 0) {
                printf("Método '%s' pertence à classe '%s', mas objeto é do tipo '%s'\n",
                       membro->nome, membro->nome_pai, objeto->tipo);
                exit(1);
            }
            verifica_parametros_chamada_funcao(lado_direito, membro->parametros, tabela);
        }

        verifica_tipagem(lado_direito, id->tipo, tabela);
        verifica_tipo_expressao(lado_direito);

        if (expressao_logica == true) {
            printf("Permitido apenas operadores aritméticos em atribuições! Linha: %ld\n", raizAST->linha);
            exit(1);
        }

        if (tipo_igual == false) {
            printf("O tipo da expressão não coincide com o tipo da variável! Linha: %ld\n", raizAST->linha);
            exit(1);
        }
    }
    else if (raizAST->type == NODE_TYPE_RETURN) {
        if (tipo_funcao_atual == NULL) {
            printf("Return fora de função! Linha: %ld\n", raizAST->linha);
            exit(1);
        }

        encontrou_return = true;

        if (raizAST->child_count > 0) {
            ASTNode* retorno = raizAST->filhos[0];
            verifica_tipagem(retorno, tipo_funcao_atual, tabela);
        } else {
            printf("Return vazio não suportado! Linha: %ld\n", raizAST->linha);
            exit(1);
        }
    }

    // Travessia recursiva dos filhos
    for (int i = 0; i < raizAST->child_count; i++) {
        teste(raizAST->filhos[i], raizAST, tabela);
    }

    // Se sair de função, verifica se teve return
    if (raizAST->type == NODE_TYPE_FUNCAO_DECL) {
        if (!encontrou_return) {
            printf("Erro: A função '%s' não possui return! Linha: %ld\n", raizAST->valor, raizAST->linha);
            exit(1);
        }
        tipo_funcao_atual = NULL;
        encontrou_return = false;
    }

    if (abriu_escopo) {
        sair_escopo(tabela);
        printf("\n==> DEBUG: Saindo de escopo (nível agora %d)\n", tabela->nivel_escopo_atual);
    }
}
