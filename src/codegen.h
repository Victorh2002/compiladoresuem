#ifndef CODEGEN_H
#define CODEGEN_H

#include <llvm-c/Core.h>
#include <llvm-c/Types.h>
#include "ast.h" 
#include "symbol_table.h"

typedef struct {
    LLVMModuleRef  modulo;    // O contêiner para todo o código IR
    LLVMBuilderRef builder;   // A ferramenta para construir instruções

    LLVMTypeRef tipo_int;     // Representa o tipo i32 (inteiro de 32 bits)
    LLVMTypeRef tipo_float;   // Representa o tipo float (32 bits)
    LLVMTypeRef tipo_char;    // Representa o tipo i8 (um byte)
    LLVMTypeRef tipo_string;  // Representa o tipo i8* (ponteiro para char)

} GeradorDeCodigo;

typedef struct LLVMSymbol {
    char* nome;
    LLVMValueRef valor_llvm;
    struct LLVMSymbol *proximo;
} LLVMSymbol;

GeradorDeCodigo inicializar_codegen(const char* nome_modulo);
LLVMValueRef gerar_codigo(ASTNode* raiz, GeradorDeCodigo* gerador, TabelaDeSimbolos* tabela, LLVMSymbol* tabela_codegen);

#endif // CODEGEN_H