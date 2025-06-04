%option noyywrap
%{
	#include <string.h>
    #include "bison.tab.h"
	extern long linha;

    #define lexeno(var)(yylval.str = strdup(yytext))
%}

numero [0-9]
decimal [0-9]*.[0-9]
espaco [" "\t]
novalinha [\n]
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
<comentario>{novalinha} {linha=linha+1; /* não retornar token, apenas incrementa a variável de controle*/}

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
<texto>. {;}

"=" { lexeno(yytext); return t_igual;}
"+" { lexeno(yytext); return t_mais;}
"-" { lexeno(yytext); return t_menos;}
"*" { lexeno(yytext); return t_asteristico;}
"/" { lexeno(yytext); return t_barra;}

int { lexeno(yytext); return t_int;}
float { lexeno(yytext); return t_float;}
char { lexeno(yytext); return t_char;}
class { lexeno(yytext); return t_class;}
"["  {lexeno(yytext); return t_vetorabri;}
"]" { lexeno(yytext); return t_vetorfecha;}

for {lexeno(yytext); return t_for;}
while {lexeno(yytext); return t_while;}
if {lexeno(yytext); return t_if;}
else {lexeno(yytext); return t_else;}
switch {lexeno(yytext); return t_switch;}

{numero}+ { lexeno(yytext);  return t_num;}
{decimal} { lexeno(yytext);  return t_decimal;}
{id} { lexeno(yytext); return t_id;} 
{novalinha} {linha=linha+1; /* não retornar token, apenas incrementa a variável de controle*/}
{espaco} /* Não faz nada, apenas consome*/

. { printf("\'%c\' (linha %d) eh um caractere misterio não usando na linguagem\n", *yytext, linha); }
%%

void yyerror (char const s){
	fprintf(stderr, "%s\n\n",s);
}

int main(int argc, char *arqv[]){
	for(int i = 1; i < argc ; i++){
		if( strcmp(arqv[i], "-e") == 0 && i<argc){
			yyin = fopen(arqv[i+1],"r");
			if(!yyin){
				printf("Não foi possível abrir o arquivo %s\n",arqv[i+1]);
				exit(-1);
			}
			i = i+1;
		}else if(strcmp(arqv[i],"-s") == 0 && i<argc){
			yyout = fopen(arqv[i+1],"w");
			i = i+1;
		}
	}
	return yyparse();
}
