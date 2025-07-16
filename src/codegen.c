#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "codegen.h"
#include "symbol_table.h"

#include <llvm-c/Core.h>
#include <llvm-c/Types.h>


GeradorDeCodigo inicializar_codegen(const char* nome_modulo) {
    GeradorDeCodigo codegen;
    codegen.modulo = LLVMModuleCreateWithName(nome_modulo);
    codegen.builder = LLVMCreateBuilder();
    printf("--- Definindo tipos LLVM ---\n");
    codegen.tipo_int = LLVMInt32Type();
    printf("Tipo 'int' definido como i32.\n");
    codegen.tipo_float = LLVMFloatType();
    printf("Tipo 'float' definido como float.\n");
    codegen.tipo_char = LLVMInt8Type();
    printf("Tipo 'char' definido como i8.\n");
    codegen.tipo_string = LLVMPointerType(codegen.tipo_char, 0);
    printf("Tipo 'string' definido como i8* (ponteiro para char).\n");
    return codegen;
}

LLVMTypeRef* obter_tipos_llvm_dos_parametros(GeradorDeCodigo* gerador, Param* primeiro_param, int* contagem_params) {
    int contador = 0;
    Param* param_atual = primeiro_param;
    while (param_atual != NULL) {
        contador++;
        param_atual = param_atual->proximo;
    }
    *contagem_params = contador;
    if (contador == 0) return NULL;
    LLVMTypeRef* tipos_llvm = malloc(contador * sizeof(LLVMTypeRef));
    if (!tipos_llvm) { exit(1); }
    param_atual = primeiro_param;
    for (int i = 0; i < contador; i++) {
        if (strcmp(param_atual->tipo, "int") == 0) {
            tipos_llvm[i] = gerador->tipo_int;
        } else if (strcmp(param_atual->tipo, "float") == 0) {
            tipos_llvm[i] = gerador->tipo_float;
        } else if (strcmp(param_atual->tipo, "char") == 0) {
            tipos_llvm[i] = gerador->tipo_char;
        } else if (strcmp(param_atual->tipo, "string") == 0) {
            tipos_llvm[i] = gerador->tipo_string;
        } else {
            fprintf(stderr, "Erro Codegen: Tipo de parâmetro '%s' ainda não suportado.\n", param_atual->tipo);
            tipos_llvm[i] = gerador->tipo_int;
        }
        param_atual = param_atual->proximo;
    }
    return tipos_llvm;
}

LLVMValueRef buscar_valor_llvm(LLVMSymbol* cabeca, const char* nome) {
    LLVMSymbol* atual = cabeca;
    while (atual != NULL) {
        if (strcmp(atual->nome, nome) == 0) {
            return atual->valor_llvm;
        }
        atual = atual->proximo;
    }
    return NULL;
}

void inserir_valor_llvm(LLVMSymbol** cabeca, const char* nome, LLVMValueRef valor) {
    LLVMSymbol* novo = malloc(sizeof(LLVMSymbol));
    novo->nome = strdup(nome);
    novo->valor_llvm = valor;
    novo->proximo = *cabeca;
    *cabeca = novo;
}

LLVMValueRef gerar_codigo(ASTNode* raiz, GeradorDeCodigo* gerador, TabelaDeSimbolos* tabela, LLVMSymbol* tabela_codegen) {
    if (raiz == NULL) {
        return NULL;
    }

    if (raiz->type == NODE_TYPE_PROGRAMA) {

        for (int i = 0; i < raiz->child_count; i++) {
            gerar_codigo(raiz->filhos[i], gerador, tabela, tabela_codegen);
        }
    }
    else if (raiz->type == NODE_TYPE_FUNCAO_DECL) {
        LLVMTypeRef tipo_retorno = NULL;
        if (strcmp(raiz->tipo_dado, "int") == 0) {
            tipo_retorno = gerador->tipo_int;
        } else if (strcmp(raiz->tipo_dado, "float") == 0) {
            tipo_retorno = gerador->tipo_float;
        } else if (strcmp(raiz->tipo_dado, "char") == 0) {
            tipo_retorno = gerador->tipo_char;
        } else if (strcmp(raiz->tipo_dado, "string") == 0){
            tipo_retorno = gerador->tipo_string;
        } else {
            printf("Funções com tipo de classe não são suportados! (Por enquanto)");
            exit(1);
        }
        
        Symbol* id = buscar_simbolo(tabela, raiz->valor);
        if (!id) { exit(1); }

        int contagem_params = 0;
        LLVMTypeRef* params = obter_tipos_llvm_dos_parametros(gerador, id->parametros, &contagem_params);

        LLVMTypeRef tipo_funcao = LLVMFunctionType(tipo_retorno, params, contagem_params, false);
        LLVMValueRef funcao = LLVMAddFunction(gerador->modulo, raiz->valor, tipo_funcao);

        LLVMSymbol* tabela_funcao_atual = NULL;

        Symbol* simbolo_func = buscar_simbolo(tabela, raiz->valor);
        Param* param_ast = simbolo_func->parametros;
        for (unsigned int i = 0; i < LLVMCountParams(funcao); i++) {
            LLVMValueRef param_llvm = LLVMGetParam(funcao, i);
            LLVMSetValueName(param_llvm, param_ast->nome); 
            
            // Mapeia o nome do parâmetro ao seu valor LLVM
            inserir_valor_llvm(&tabela_funcao_atual, param_ast->nome, param_llvm);

            param_ast = param_ast->proximo;
        }

        LLVMBasicBlockRef entry_block = LLVMAppendBasicBlock(funcao, "entry");
        LLVMPositionBuilderAtEnd(gerador->builder, entry_block);
        
        for (int i = 0; i < raiz->child_count; i++) {
            gerar_codigo(raiz->filhos[i], gerador, tabela, tabela_funcao_atual); 
        }

        if (LLVMGetBasicBlockTerminator(LLVMGetLastBasicBlock(funcao)) == NULL) {
             LLVMBuildRet(gerador->builder, LLVMConstInt(tipo_retorno, 0, false));
        }

        if(params) free(params); 
        return funcao;

    } else if (raiz->type == NODE_TYPE_RETURN) {
        if (raiz->child_count > 0) {
            LLVMValueRef valor_retornado = gerar_codigo(raiz->filhos[0], gerador, tabela, tabela_codegen);
            return LLVMBuildRet(gerador->builder, valor_retornado);
        }
    } else if (raiz->type == NODE_TYPE_NUMERO) {
        int valor = atoi(raiz->valor);
        return LLVMConstInt(gerador->tipo_int, valor, false);
    } else if (raiz->type == NODE_TYPE_STRING) {
        if (strlen(raiz->valor) == 1) {
            return LLVMConstInt(gerador->tipo_char, (unsigned long long)raiz->valor[0], false);
        } else {
            return LLVMBuildGlobalStringPtr(gerador->builder, raiz->valor, "str_literal");
        }
    } else if (raiz->type == NODE_TYPE_IDENTIFICADOR) {
        return buscar_valor_llvm(tabela_codegen, raiz->valor);
    }

    return NULL;
}