// Arquivo: ast.c
#include "ast.h"

/**
 * @brief Cria um nó base da AST, inicializando todos os campos.
 * @param type O tipo do nó (da enumeração NodeType).
 * @param valor O valor textual do nó (ex: nome de variável, número, operador).
 * @param filhos Um array de ponteiros para os nós filhos.
 * @param num_filhos O número de filhos no array.
 * @param proximo Ponteiro para o próximo comando na lista sequencial.
 * @return Ponteiro para o novo nó alocado.
 */
ASTNode* criar_no(NodeType type, const char* valor, ASTNode* filhos[], int num_filhos, ASTNode* proximo, long linha) {
    ASTNode* novo_no = (ASTNode*) malloc(sizeof(ASTNode));
    if (!novo_no) {
        printf("ERRO: Falha ao alocar memória para nó da AST\n");
        exit(1);
    }

    // Inicializa todos os campos para valores padrão seguros
    novo_no->type = type;
    novo_no->valor = valor ? strdup(valor) : NULL;
    novo_no->tipo_dado = NULL;
    novo_no->proximo_comando = proximo;
    novo_no->is_array = 0;
    novo_no->array_size = 0;
    novo_no->child_count = 0;
    novo_no->linha = linha;
    novo_no->filhos = NULL;

    // Se houver filhos, aloca o array e os copia
    if (num_filhos > 0 && filhos != NULL) {
        novo_no->child_count = num_filhos;
        novo_no->filhos = (ASTNode**) malloc(num_filhos * sizeof(ASTNode*));
        if (!novo_no->filhos) {
            printf("ERRO: Falha ao alocar memória para filhos do nó\n");
            exit(1);
        }
        for (int i = 0; i < num_filhos; i++) {
            novo_no->filhos[i] = filhos[i];
        }
    }
    
    return novo_no;
}

/**
 * @brief Imprime a AST de forma recursiva com indentação para visualização.
 * @param no O nó raiz da árvore/sub-árvore a ser impressa.
 * @param nivel O nível de profundidade atual, para controlar a indentação.
 */
void imprimir_ast(ASTNode *no, int nivel) {
    if (no == NULL) {
        return;
    }

    // --- 1. Imprime o nó atual ---
    for (int i = 0; i < nivel; i++) {
        printf("|  ");
    }

    switch (no->type) {
        case NODE_TYPE_PROGRAMA:           
            printf("Programa\n"); 
            break;
        case NODE_TYPE_NUMERO:             
            printf("Numero: %s\n", no->valor); 
            break;
        case NODE_TYPE_IDENTIFICADOR:      
            printf("ID: %s\n", no->valor); 
            break;
        case NODE_TYPE_OPERACAO_BINARIA:   
            printf("Operacao: %s\n", no->valor); 
            break;
        case NODE_TYPE_FUNCAO_DECL:        
            printf("Funcao: %s (Tipo: %s, Linha: %ld)\n", no->valor, no->tipo_dado, no->linha); 
            break;
        case NODE_TYPE_ATRIBUICAO:         
            printf("Atribuicao: =\n"); 
            break;
        case NODE_TYPE_ARRAY_LITERAL:      
            printf("Literal de Vetor: []\n"); 
            break;
        case NODE_TYPE_VAR_DECL:
            printf("Declaracao Variavel: %s (Tipo: %s, Linha: %ld", no->valor, no->tipo_dado, no->linha);
            if (no->is_array) {
                printf(", Vetor[%d]", no->array_size);
            }
            printf(")\n");
            break;
        case NODE_TYPE_RETURN:
            printf("Return\n");
            break;
        case NODE_TYPE_IF:                     
            printf("Comando %s\n", no->valor); 
            break;
        case NODE_TYPE_WHILE:                     
            printf("Comando while\n"); 
            break;
        case NODE_TYPE_OPERACAO_UNARIA:   
            printf("Operacao Unaria: %s\n", no->valor); 
            break;
        case NODE_TYPE_STRING:             
            printf("string: %s\n", no->valor); 
            break;
        case NODE_TYPE_FUNCAO_CALL:
            printf("Chamada de Funcao: %s\n", no->valor);
            break;
        case NODE_TYPE_METHOD_CALL:
            printf("Chamada de Metodo: %s\n", no->valor);
            break;
        case NODE_TYPE_CLASSE_DECL:
            printf("Declaracao de Classe: %s (Linha: %ld) \n", no->valor, no->linha);
            break;
        case NODE_TYPE_ARRAY_ACCESS:
            printf("Acesso a Vetor: %s\n", no->valor); 
            break;
        case NODE_TYPE_MEMBER_ACCESS:
            printf("ID atributo: %s\n", no->valor); 
            break;
        default:                           
            printf("Nó Desconhecido (%d)\n", no->type); 
            break;
    }

    // --- 2. Imprime os filhos (recursão vertical) ---
    for (int i = 0; i < no->child_count; i++) {
        imprimir_ast(no->filhos[i], nivel + 1);
    }
}

void lista_para_vetor(ASTNode* no_pai, ASTNode* lista) {
    // Converte a lista ligada de argumentos em um array de filhos
    int contador = 0;
    ASTNode* no_atual = lista;
    while (no_atual != NULL) {
        contador++;
        no_atual = no_atual->proximo_comando;
    }

    no_pai->child_count = contador;
    if (contador > 0) {
        no_pai->filhos = malloc(contador * sizeof(ASTNode*));
        // ... (checagem de erro do malloc) ...

        no_atual = lista;
        for (int i = 0; i < contador; i++) {
            no_pai->filhos[i] = no_atual;
            ASTNode* proximo = no_atual->proximo_comando;
            no_atual->proximo_comando = NULL;
            no_atual = proximo;
        }
    }
}

/**
 * @brief Função de conveniência para criar um nó de declaração de variável simples.
 */
ASTNode* criar_no_declaracao(const char* tipo_dado, const char* nome_var, long linha) {
    ASTNode* novo_no = criar_no(NODE_TYPE_VAR_DECL, nome_var, NULL, 0, NULL, linha);
    novo_no->tipo_dado = tipo_dado ? strdup(tipo_dado) : NULL;
    return novo_no;
}

/**
 * @brief Função de conveniência para criar um nó de declaração de variável com valor inicial.
 */
ASTNode* criar_no_declaracao_com_valor(const char* tipo_dado, const char* nome_var, ASTNode* valor_inicial, long linha) {
    ASTNode* filhos[] = {valor_inicial};
    int num_filhos = (valor_inicial != NULL) ? 1 : 0;
    
    ASTNode* no_decl = criar_no(NODE_TYPE_VAR_DECL, nome_var, filhos, num_filhos, NULL, linha);
    no_decl->tipo_dado = tipo_dado ? strdup(tipo_dado) : NULL;

    return no_decl;
}

/**
 * @brief Cria um nó para declaração de um vetor, convertendo a lista de inicializadores em filhos.
 */
ASTNode* criar_no_declaracao_vetor(const char* tipo_dado, const char* nome_var, ASTNode* lista_inicializadores, long linha) {
    ASTNode* no_decl = criar_no(NODE_TYPE_VAR_DECL, nome_var, NULL, 0, NULL, linha);
    no_decl->tipo_dado = tipo_dado ? strdup(tipo_dado) : NULL;
    no_decl->is_array = 1;

    // Converte a lista ligada de inicializadores em um array de filhos
    int contador = 0;
    ASTNode* no_atual = lista_inicializadores;
    while (no_atual != NULL) {
        contador++;
        no_atual = no_atual->proximo_comando;
    }

    no_decl->array_size = contador;
    no_decl->child_count = contador;

    if (contador > 0) {
        no_decl->filhos = malloc(contador * sizeof(ASTNode*));
        if (!no_decl->filhos) { exit(1); }

        no_atual = lista_inicializadores;
        for (int i = 0; i < contador; i++) {
            no_decl->filhos[i] = no_atual;
            ASTNode* proximo = no_atual->proximo_comando;
            no_atual->proximo_comando = NULL; 
            no_atual = proximo;
        }
    }

    return no_decl;
}

/**
 * @brief Cria um nó para um literal de vetor, convertendo a lista de elementos em filhos.
 */
ASTNode* criar_no_literal_vetor(ASTNode* lista_elementos) {
    ASTNode* no_literal = criar_no(NODE_TYPE_ARRAY_LITERAL, "[]", NULL, 0, NULL, 0);
    no_literal->is_array = 1;

    // Lógica para contar e converter a lista para um array de filhos
    int contador = 0;
    ASTNode* no_atual = lista_elementos;
    while (no_atual != NULL) {
        contador++;
        no_atual = no_atual->proximo_comando;
    }

    no_literal->child_count = contador;
    no_literal->array_size = contador;
    
    if (contador > 0) {
        no_literal->filhos = malloc(contador * sizeof(ASTNode*));
        if (!no_literal->filhos) { exit(1); }

        no_atual = lista_elementos;
        for (int i = 0; i < contador; i++) {
            no_literal->filhos[i] = no_atual;
            ASTNode* proximo = no_atual->proximo_comando;
            no_atual->proximo_comando = NULL;
            no_atual = proximo;
        }
    }

    return no_literal;
}

/**
 * @brief Função de conveniência para criar um nó de declaração de função.
 */
ASTNode* criar_no_funcao(const char* tipo_retorno, const char* nome_func, ASTNode* filhos[], int num_filhos, long linha) {
    // Reutilizamos a função base para criar o nó e anexar os filhos (corpo, parâmetros)
    ASTNode* no_func = criar_no(NODE_TYPE_FUNCAO_DECL, nome_func, NULL, 0, NULL, linha);

    // Converte a lista ligada de argumentos em um array de filhos
    int contador = 0;
    int contador_filho[num_filhos];
    for (int i = 0; i < num_filhos; i++)
    {
        ASTNode* no_atual = filhos[i];
        contador_filho[i] = 0;
        while (no_atual != NULL) {
            contador++;
            contador_filho[i]++;
            no_atual = no_atual->proximo_comando;
        }
    }

    no_func->child_count = contador;
    if (contador > 0) {
        no_func->filhos = malloc(contador * sizeof(ASTNode*));

        ASTNode* no_atual = filhos[0];
        for (int i = 0; i < contador_filho[0]; i++) {
            no_func->filhos[i] = no_atual;
            ASTNode* proximo = no_atual->proximo_comando;
            no_atual->proximo_comando = NULL;
            no_atual = proximo;           
        }

        if (contador_filho[1])
        {
            no_atual = filhos[1];
            for (int i = contador_filho[0]; i < contador; i++) {
                no_func->filhos[i] = no_atual;
                ASTNode* proximo = no_atual->proximo_comando;
                no_atual->proximo_comando = NULL;
                no_atual = proximo;
            }
        }   
    }
    
    // E agora, preenchemos o campo específico do tipo de dado, que faltava antes.
    no_func->tipo_dado = tipo_retorno ? strdup(tipo_retorno) : NULL;

    return no_func;
}

ASTNode* criar_no_chamada_funcao(const char* nome_func, ASTNode* lista_argumentos) {
    // Cria o nó "pai" para a chamada de função
    ASTNode* no_chamada = criar_no(NODE_TYPE_FUNCAO_CALL, nome_func, NULL, 0, NULL, 0);

    lista_para_vetor(no_chamada, lista_argumentos);

    return no_chamada;
}

ASTNode* criar_no_classe(const char* nome_classe, ASTNode* lista_membros, long linha) {
    // Cria o nó "pai" para a declaração da classe
    ASTNode* no_classe = criar_no(NODE_TYPE_CLASSE_DECL, nome_classe, NULL, 0, NULL, linha);

    lista_para_vetor(no_classe, lista_membros);

    return no_classe;
}

ASTNode* criar_no_programa(ASTNode* lista_declaracoes) {
    ASTNode* no_programa = criar_no(NODE_TYPE_PROGRAMA, "Programa", NULL, 0, NULL, 0);
    lista_para_vetor(no_programa, lista_declaracoes);
    return no_programa;
}

ASTNode* criar_no_chamada_metodo(const char* nome_func, ASTNode* filhos[], int num_filhos) {
    // Cria o nó "pai" para a chamada de função
    ASTNode* no_chamada = criar_no(NODE_TYPE_METHOD_CALL, nome_func, NULL, 0, NULL, 0);

    // Converte a lista ligada de argumentos em um array de filhos
    int contador = 0;
    int contador_filho[num_filhos];
    for (int i = 0; i < num_filhos; i++)
    {
        ASTNode* no_atual = filhos[i];
        contador_filho[i] = 0;
        while (no_atual != NULL) {
            contador++;
            contador_filho[i]++;
            no_atual = no_atual->proximo_comando;
        }
    }

    no_chamada->child_count = contador;
    if (contador > 0) {
        no_chamada->filhos = malloc(contador * sizeof(ASTNode*));

        ASTNode* no_atual = filhos[0];
        for (int i = 0; i < contador_filho[0]; i++) {
            no_chamada->filhos[i] = no_atual;
            ASTNode* proximo = no_atual->proximo_comando;
            no_atual->proximo_comando = NULL;
            no_atual = proximo;           
        }

        if (contador_filho[1])
        {
            no_atual = filhos[1];
            for (int i = contador_filho[0]; i < contador; i++) {
                no_chamada->filhos[i] = no_atual;
                ASTNode* proximo = no_atual->proximo_comando;
                no_atual->proximo_comando = NULL;
                no_atual = proximo;
            }
        }   
    }

    return no_chamada;
}