#include "symbol_table.h"

void inicializar_tabela(TabelaDeSimbolos* tabela) {
    tabela->nivel_escopo_atual = -1;
    tabela->globais = NULL;
    for (int i = 0; i < MAX_ESCOPOS; i++) {
        tabela->escopos[i] = NULL;
    }
}

void entrar_escopo(TabelaDeSimbolos* tabela) {
    if (tabela->nivel_escopo_atual < MAX_ESCOPOS - 1) {
        tabela->nivel_escopo_atual++;
        tabela->escopos[tabela->nivel_escopo_atual] = NULL;
    } else {
        fprintf(stderr, "Erro: Profundidade máxima de escopo excedida!\n");
        exit(1);
    }
}

void sair_escopo(TabelaDeSimbolos* tabela) {
    if (tabela->nivel_escopo_atual < 0) {
        fprintf(stderr, "Erro: Tentativa de sair de um escopo que não existe.\n");
        return;
    }

    Symbol* no_atual = tabela->escopos[tabela->nivel_escopo_atual];
    Symbol* proximo_no;
    while (no_atual != NULL) {
        proximo_no = no_atual->proximo;
        free(no_atual->nome);
        free(no_atual->tipo);
        free(no_atual);
        no_atual = proximo_no;
    }

    tabela->escopos[tabela->nivel_escopo_atual] = NULL;
    tabela->nivel_escopo_atual--;
}

Symbol* inserir_simbolo(TabelaDeSimbolos* tabela, char* nome, const char* tipo, SymbolKind kind, long linha, int is_global, int is_array, int array_size, char* nome_pai) {
    Symbol** lista = is_global ? &(tabela->globais) : &(tabela->escopos[tabela->nivel_escopo_atual]);
    Symbol* no_atual = *lista;

    while (no_atual != NULL) {
        if (strcmp(nome, no_atual->nome) == 0) {
            fprintf(stderr, "Erro Semântico: '%s' já declarada no escopo atual.\n", nome);
            return NULL;
        }
        no_atual = no_atual->proximo;
    }

    Symbol* novo_simbolo = (Symbol*) malloc(sizeof(Symbol));
    novo_simbolo->nome = strdup(nome);
    novo_simbolo->tipo = tipo ? strdup(tipo) : NULL;
    novo_simbolo->kind = kind;
    novo_simbolo->linha = linha;
    novo_simbolo->parametros = NULL;
    novo_simbolo->proximo = *lista;
    novo_simbolo->is_array = is_array;
    novo_simbolo->array_size = array_size;
    novo_simbolo->nome_pai = nome_pai ? strdup(nome_pai) : NULL;
    *lista = novo_simbolo;

    printf("--> Símbolo inserido: %s (Escopo %s)\n", nome, is_global ? "Global" : "Local");

    return novo_simbolo;
}


Symbol* buscar_simbolo(TabelaDeSimbolos* tabela, const char* nome) {
    for (int i = tabela->nivel_escopo_atual; i >= 0; i--) {
        Symbol* atual = tabela->escopos[i];
        while (atual != NULL) {
            if (strcmp(atual->nome, nome) == 0) return atual;
            atual = atual->proximo;
        }
    }

    Symbol* atual = tabela->globais;
    while (atual != NULL) {
        if (strcmp(atual->nome, nome) == 0) return atual;
        atual = atual->proximo;
    }

    return NULL;
}

const char* kind_para_string(SymbolKind kind) {
    switch (kind) {
        case SYM_VARIAVEL: return "Variavel";
        case SYM_PARAM: return "Parametro";
        case SYM_FUNCAO: return "Funcao";
        case SYM_CLASSE: return "Classe";
        default: return "Desconhecido";
    }
}

/**
 * @brief Imprime o conteúdo completo da tabela de símbolos no console.
 * Percorre todos os escopos, do global (nível 0) ao mais interno.
 */
void imprimir_tabela_simbolos(TabelaDeSimbolos tabela) {
    printf("\n.----------------------------------------------------------------------------------------------------------------------.\n");
    printf("|                                                Tabela de Símbolos                                                    |\n");
    printf("+---------+-----------------+-----------------+-----------------+------------+-------------+-------------+-------------+\n");
    printf("| Escopo  | Categoria       | Tipo            | Nome            | Linha      | Parametros  | Vetor       | Classe-Pai  |\n");
    printf("+---------+-----------------+-----------------+-----------------+------------+-------------+-------------+-------------+\n");

    int vazio = 1;
    char buffer_escopo[20];

    // Imprime escopos locais da pilha (do mais interno para o mais externo)
    for (int i = tabela.nivel_escopo_atual; i >= 0; i--) {
        Symbol* atual = tabela.escopos[i];
        
        // Define o nome do escopo para impressão
        if (i == 0) {
            // No seu modelo, o nível 0 da pilha pode ser o escopo da primeira função, não o global
            sprintf(buffer_escopo, "local_%d", i); 
        } else {
            sprintf(buffer_escopo, "local_%d", i);
        }

        while (atual != NULL) {
            vazio = 0;
            // --- LÓGICA DA NOVA COLUNA ---
            char tem_params[5] = "N/A"; // Valor padrão para não-funções
            char is_array[5] = "N/A";
            if (atual->kind == SYM_FUNCAO) {
                // Se for uma função, verifica se a lista de parâmetros não é nula
                strcpy(tem_params, atual->parametros ? "Sim" : "Nao");
            }
            if (atual->kind == SYM_VARIAVEL)
            {   
                strcpy(is_array, atual->is_array ? "Sim" : "Nao");
            }
            
            printf("| %-8s| %-16s| %-16s| %-16s| %-10ld | %-11s | %-11s | %-11s |\n",
                    buffer_escopo,
                    kind_para_string(atual->kind),
                    atual->tipo ? atual->tipo : "N/A",
                    atual->nome,
                    atual->linha,
                    tem_params,
                    is_array,
                    atual->nome_pai ? atual->nome_pai : "N/A"); // Imprime a nova coluna
            atual = atual->proximo;
        }
    }

    // Imprime o escopo global separado
    Symbol* atual = tabela.globais;
    while (atual != NULL) {
        vazio = 0;
        // --- LÓGICA DA NOVA COLUNA ---
        char tem_params[5] = "N/A";
        char is_array[5] = "N/A";
        if (atual->kind == SYM_FUNCAO) {
            strcpy(tem_params, atual->parametros ? "Sim" : "Nao");
        }
        if (atual->kind == SYM_VARIAVEL)
        {
            strcpy(is_array, atual->is_array ? "Sim" : "Nao");
        }

        printf("| Global  | %-16s| %-16s| %-16s| %-10ld | %-11s | %-11s | %-11s |\n",
                    kind_para_string(atual->kind),
                    atual->tipo ? atual->tipo : "N/A",
                    atual->nome,
                    atual->linha,
                    tem_params,
                    is_array,
                    atual->nome_pai ? atual->nome_pai : "N/A"); // Imprime a nova coluna
        atual = atual->proximo;
    }

    if (vazio) {
        printf("| (Tabela Vazia)                                                                                                       |\n");
    }

    printf("+---------+-----------------+-----------------+-----------------+------------+-------------+-------------+-------------+\n\n");
}

Param* criar_parametro(const char* nome, const char* tipo) {
    Param* p = (Param*) malloc(sizeof(Param));
    p->nome = strdup(nome);
    p->tipo = strdup(tipo);
    p->proximo = NULL;
    return p;
}

void adicionar_parametro(Symbol* func, Param* param) {
    if (!func->parametros) {
        func->parametros = param;
    } else {
        Param* atual = func->parametros;
        while (atual->proximo) {
            atual = atual->proximo;
        }
        atual->proximo = param;
    }
}

