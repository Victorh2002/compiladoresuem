%option noyywrap
%{
	#include <string.h>
    #include "ast.h"
    #include "bison.tab.h"

    #define lexeno(var)(yylval.str = strdup(yytext))
    long linha = 1;
%}

numero [0-9]+
decimal [0-9]+\.[0-9]+
espaco [" "\t]
id [a-zA-Z][a-zA-Z0-9_]*
aberturacomentario [/][*]
fechamentocomentario [*][/]

/*subscanner*/
%x comentario 
%x texto
%% /* definições de toke para o flex procurar*/
{aberturacomentario} {BEGIN(comentario);}
<comentario>{fechamentocomentario} {BEGIN(INITIAL); /*É um escape do sub scanner 'comentario' - fim de comentário*/}
<comentario>[^*\n]+ 
<comentario>"*"
<comentario>\r\n|\n|\r {linha++; /* não retornar token, apenas incrementa a variável de controle*/}

%{
    // Buffer para acumular o conteúdo da string
    char string_buf[1024];
    char *string_buf_ptr;
%}

\" {
    string_buf_ptr = string_buf; // Reseta o ponteiro do buffer
    BEGIN(texto);
   }
<texto>\" {
    *string_buf_ptr = '\0'; // Termina a string no buffer
    lexeno(string_buf);
    BEGIN(INITIAL);
    return t_palavra; // Ou um token específico como T_STRING_LITERAL
    }
<texto>[^\"\n]+ {
    // Acumula caracteres no buffer
    char *yptr = yytext;
    while ( *yptr && string_buf_ptr < string_buf + sizeof(string_buf) - 1) {
        *string_buf_ptr++ = *yptr++;
    }
    // Adicionar tratamento se o buffer estourar
}

"=" { printf("-> LEXER: Gerou token t_igual\n"); lexeno(yytext); return t_igual;}
"+" { printf("-> LEXER: Gerou token t_mais\n"); lexeno(yytext); return t_mais;}
"-" { lexeno(yytext); return t_menos;}
"*" { lexeno(yytext); return t_asteristico;}
"/" { lexeno(yytext); return t_barra;}
and  {lexeno(yytext); return t_and;}
or  {lexeno(yytext); return t_or;}

int { printf("-> LEXER: Gerou token t_int\n"); lexeno(yytext); return t_int;}
float { lexeno(yytext); return t_float;}
char { lexeno(yytext); return t_char;}
class { lexeno(yytext); return t_class;}
"["  {lexeno(yytext); return t_vetorabri;}
"]" { lexeno(yytext); return t_vetorfecha;}
"(" { printf("-> LEXER: Gerou token t_parentesabri\n"); lexeno(yytext); return t_parentesabri;}
")" { printf("-> LEXER: Gerou token t_parentesfecha\n"); lexeno(yytext); return t_parentesfecha;}
"{" { printf("-> LEXER: Gerou token t_chaveabri\n"); lexeno(yytext); return t_chaveabri;}
"}" { printf("-> LEXER: Gerou token t_chavefecha\n"); lexeno(yytext); return t_chavefecha;}
";" { printf("-> LEXER: Gerou token t_pontvirgula\n"); lexeno(yytext); return t_pontvirgula;}

for {lexeno(yytext); return t_for;}
while {lexeno(yytext); return t_while;}
if {lexeno(yytext); return t_if;}
else {lexeno(yytext); return t_else;}
switch {lexeno(yytext); return t_switch;}

\r\n|\n|\r {linha++; /* não retornar token, apenas incrementa a variável de controle*/}
{decimal} { printf("-> LEXER: Gerou token t_decimal\n"); lexeno(yytext);  return t_decimal;}
{numero} { printf("-> LEXER: Gerou token t_num\n"); lexeno(yytext);  return t_num;}
{id} { printf("-> LEXER: Gerou token t_id\n"); lexeno(yytext); return t_id;} 
{espaco}+ /* Não faz nada, apenas consome*/

. { printf("\'%c\' (linha %ld) eh um caractere misterio não usando na linguagem\n", *yytext, linha); }
%%
