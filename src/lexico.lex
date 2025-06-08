%option noyywrap
%{
    #include <string.h>
    #include "ast.h"
    #include "bison.tab.h" // Essencial para conhecer os tokens

    // Macro para facilitar a atribuição de lexemas
    #define lexeno(val) (yylval.str = strdup(val))
    
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

%{

/* --- Palavras-chave --- */

%}

"return"    { return t_return; }
"int"       { lexeno(yytext); return t_int;    }
"float"     { lexeno(yytext); return t_float;  }
"char"      { lexeno(yytext); return t_char;   }
"class"     { return t_class;  }
"for"       { return t_for;    }
"while"     { return t_while;  }
"if"        { return t_if;     }
"else"      { return t_else;   }
"switch"    { return t_switch; }
"and"       { return t_and;    }
"or"        { return t_or;     }

%{

/* --- Operadores e Pontuação --- */
/* Para estes, o parser só precisa saber o tipo, não o valor "texto" */

%}

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

[ \t]+          { /* Ignora espaços e tabs */ }
\r\n|\n|\r     { linha++; }

.               { fprintf(stderr, "Erro Lexico: Caractere invalido '%c' na linha %ld\n", *yytext, linha); }

%%