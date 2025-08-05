%{
#include "B.h"
int yyerror(char *s);
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunneeded-internal-declaration"
%}

%option noyywrap


identifier	[a-zA-Z]
symbol		[-+=;]
blanks		[ \t\n]

%%

"print"			{return print;}
"exit"			{return exit_command;}
{identifier}		{yylval.id = yytext[0]; return identifier;}
[0-9]+			{yylval.num = atoi(yytext); return number;}
{blanks}			{ }
{symbol}			{return yytext[0];}
.				{ECHO; yyerror ("unexpected character");}

%%
