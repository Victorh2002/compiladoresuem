%option noyywrap
%option yylineno
%{
    #include <string.h>
    #include "ast.h"
    #include "bison.tab.h" // Essencial para conhecer os tokens
    extern YYLTYPE yylloc;

    // Macro para facilitar a atribuição de lexemas
    #define lexeno(val) (yylval.str = strdup(val))

    /**
     * MACRO DE ATUALIZAÇÃO DE LOCALIZAÇÃO
     * Esta macro será chamada ANTES de cada 'return'. Ela manualmente define
     * a linha inicial e final do token atual para a linha que o 'yylineno' está marcando.
     * Esta é a correção definitiva para o problema do @1.first_line.
     */
    #define YY_USER_ACTION \
        yylloc.first_line = yylineno; \
        yylloc.last_line = yylineno;
    
    // Variáveis globais
    long linha = 1;
    char string_buf[1024];
    char *string_buf_ptr;
%}

/* --- Definições de Nomes (Aliases) --- */
numero  [0-9]+
decimal [0-9]+\.[0-9]+
id      [a-zA-Z_][a-zA-Z0-9_]*

/* --- Definição de Estados para Comentários e Strings --- */
%x comentario 
%x texto

%{
/* =================================================================== */
/* --- Regras Léxicas --- */
/* =================================================================== */

/* --- Controle de Estados (Comentários, Strings) --- */  
%}
%%
"/*"        { BEGIN(comentario); }
<comentario>"*/"        { BEGIN(INITIAL); }
<comentario>[^*\n]+     { /* Consome texto do comentário */ }
<comentario>"*"         { /* Consome asteriscos dentro do comentário */ }
<comentario>\r\n|\n|\r   { linha++; }

\"          { string_buf_ptr = string_buf; BEGIN(texto); }
<texto>\"   { 
    *string_buf_ptr = '\0';
    lexeno(string_buf);
    BEGIN(INITIAL);
    return t_palavra; // NOTA: Token corrigido
}
<texto>[^\"\n]+ {
    char *yptr = yytext;
    while (*yptr && string_buf_ptr < string_buf + sizeof(string_buf) - 1) {
        *string_buf_ptr++ = *yptr++;
    }
}
<texto>\r\n|\n|\r {
    fprintf(stderr, "Erro na linha %ld: String não terminada (encontrada quebra de linha).\n", linha);
    linha++;         // Incrementa o contador, pois consumimos uma quebra de linha
    BEGIN(INITIAL);  // ABORTA o estado <texto> e volta ao estado normal
}

%{

/* --- Palavras-chave --- */

%}

"return"    { return t_return;      }
"int"       { lexeno(yytext); return t_int;    }
"float"     { lexeno(yytext); return t_float;  }
"char"      { lexeno(yytext); return t_char;   }
"string"    { lexeno(yytext); return t_string; }
"class"     { return t_class;       }
"while"     { return t_while;       }
"if"        { return t_if;          }
"else"      { return t_else;        }
"or"        { return t_or;          }
"and"       { return t_and;         }

%{

/* --- Operadores e Pontuação --- */
/* Para estes, o parser só precisa saber o tipo, não o valor "texto" */

%}

">="        { lexeno(yytext); return t_maiorigual;  }
"<="        { lexeno(yytext); return t_menorigual;  }
">"         { lexeno(yytext); return t_maior;       }
"<"         { lexeno(yytext); return t_menor;       }
"=="        { lexeno(yytext); return t_igualigual;  }
"!="        { lexeno(yytext); return t_diferente;   }
"!"         { lexeno(yytext); return t_negacao;     }
"="         { return t_igual;       }
"+"         { lexeno(yytext); return t_mais; } // Ação mantida pois o valor é usado na AST
"-"         { lexeno(yytext); return t_menos; }
"*"         { lexeno(yytext); return t_asteristico; }
"/"         { lexeno(yytext); return t_barra; }
"["         { return t_vetorabri;     }
"]"         { return t_vetorfecha;    }
"("         { return t_parentesabri;  }
")"         { return t_parentesfecha; }
"{"         { return t_chaveabri;     }
"}"         { return t_chavefecha;    }
";"         { return t_pontvirgula;   }
","         { return t_virgula;       }
"."         { return t_ponto;         }

%{

/* --- Literais (Números e Identificadores) --- */
/* Estes precisam do lexeno pois o valor é importante */

%}

{decimal}   { lexeno(yytext); return t_decimal; }
{numero}    { lexeno(yytext); return t_num;     }
{id}        { lexeno(yytext); return t_id;      }

%{
    
/* --- Whitespace e Erros --- */

%}

[ \t]+         { /* Ignora espaços e tabs */ }
\n|\r|\r\n     { linha++; }

.               { fprintf(stderr, "Erro Lexico: Caractere invalido '%c' na linha %d\n", *yytext, yylineno); }

%%