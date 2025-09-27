%{
#include "B_header.h"
#include "B_2.h"
int yyerror(const char *s);
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunneeded-internal-declaration"
%}

%option noyywrap

DIGIT [0-9]
NUMBER {DIGIT}+
NORMALCHAR   [ -!#-&+,-~]
ESCAPESEQ    \\(O|e|\(|\)|t|\*|\'|\"|n)
ALPHA	[A-Za-z_\b]
IDENTIFIER {ALPHA}+

%%



{NUMBER}	{yylval.num = atoi(yytext); return NUMBER;}

"auto"                   { yylval.str = strdup(yytext); return AUTO; }
"extrn"                  { yylval.str = strdup(yytext); return EXTRN; }
"return"                 { yylval.str = strdup(yytext); return RETURN; }
"while"                  { yylval.str = strdup(yytext); return WHILE; }
"if"                     { yylval.str = strdup(yytext); return IF; }
"else"                   { yylval.str = strdup(yytext); return ELSE; }
"goto"                   { yylval.str = strdup(yytext); return GOTO; }

{IDENTIFIER}		{yylval.name = strdup(yytext); return IDENTIFIER;}

\'({NORMALCHAR}|{ESCAPESEQ})\'   {
    // strip the surrounding quotes
    char c;
    if (yytext[1] == '\\') {
        // escape sequence
        switch (yytext[2]) {
            case 'O': c = '\0'; break;
            case 'e': c = EOF; break;
            case '(': c = '('; break;
            case ')': c = ')'; break;
            case 't': c = '\t'; break;
            case '*': c = '*'; break;
            case '\'': c = '\''; break;
            case '"': c = '\"'; break;
            case 'n': c = '\n'; break;
        }
    } else {
        // normal char
        c = yytext[1];
    }
    yylval.c = c;
    return CHARCONST;
}

\"({NORMALCHAR}|{ESCAPESEQ})*\"   {
    // strip quotes
    int len = yyleng - 2;
    char *buf = malloc(len + 1);
    int i, j = 0;
    for (i = 1; i < yyleng - 1; i++) {
        if (yytext[i] == '\\') {
            switch (yytext[++i]) {
                case 'O': buf[j++] = '\0'; break;
                case 'e': buf[j++] = EOF; break;
                case '(': buf[j++] = '('; break;
                case ')': buf[j++] = ')'; break;
                case 't': buf[j++] = '\t'; break;
                case '*': buf[j++] = '*'; break;
                case '\'': buf[j++] = '\''; break;
                case '"': buf[j++] = '\"'; break;
                case 'n': buf[j++] = '\n'; break;
            }
        } else {
            buf[j++] = yytext[i];
        }
    }
    buf[j] = '\0';
    yylval.str = buf;
    return STRING;
}

","      { return ','; }
";"      { return ';'; }
"{"      { return '{'; }
"}"      { return '}'; }
"("      { return LPAREN; }
")"      { return ')'; }
"="      { return ASSIGN; }
"+"      { return PLUS; }
"-"      { return MINUS; }
"*"      { return MUL; }
"%"      { return MOD; }
"/"      { return DIV; }
"!"      { return NEG; }
"!="     { return NE; }
"<"      { return LT; }
">"      { return GT; }
"=="     { return EQ; }
">="     { return GT_EQ;}
"<="     { return LT_EQ;}
"]"      { return ']'; }
"["      { return '['; }
"|"      { return '|'; }
"&"      { return '&'; }
[ \t\n]+ { /* skip whitespace */ }


%%


/*
// Matching all valid char in B
[ -!#-&+,-~]
// Matching both valid and special char
(?:[ -!#-&+,-~]|\*(?:[Oe()t*'"n]))

// Char in B:

char ::= normal-char | escape-seq

normal-char ::= 

escape-seq ::= "*" escape-code

escape-code ::= "O"    (* null character *)
              | "e"    (* end-of-file *)
              | "("    (* literal '(' *)
              | ")"    (* literal ')' *)
              | "t"    (* tab *)
              | "*"    (* literal '*' *)
              | "'"    (* literal single quote *)
              | '"'    (* literal double quote *)
              | "n"    (* newline *)
*/
