%option noyywrap
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"

int lineno = 1;
%}

/* token ID NUM INT VOID IF ELSE RET LE LT GT GE EQ NE */

letter     [a-zA-Z]
digit      [0-9]
identifier {letter}+
number     {digit}+
whitespace [ \t\r]+
newline    \n

%%
"int"        {return INT;}
"void"       {return VOID;}
"if"         {return IF;}
"else"       {return ELSE;}
"return"     {return RET;}

{identifier} {yylval = newParseTreeNode();
              yylval->strval = (char *) myalloc(strlen(yytext)+1);
              strcpy(yylval->strval,yytext);
              return ID;}

{number}     {yylval = newParseTreeNode();
              yylval->numval = atoi(yytext);
              return NUM;}

"<="         {return LE;} 
"<"          {return LT;} 
">="         {return GE;} 
">"          {return GT;} 
"=="         {return EQ;} 
"!="         {return NE;} 

"+"          {return '+';}           
"-"          {return '-';} 
"*"          {return '*';} 
"/"          {return '/';} 
"="          {return '=';} 
 
";"          {return ';';}
","          {return ',';}
"("          {return '(';}
")"          {return ')';}
"["          {return '[';}
"]"          {return ']';}
"{"          {return '{';}
"}"          {return '}';}

"/*"         {char c;
              int done = 0;
              do {
                while ( (c = input()) != '*')
                  if (c == '\n')
                    lineno++;
                while ( (c = input()) == '*') ;
                if (c == '\n')
                  lineno++;
                if (c == '/')
                  done = 1;
                } while (!done);}

{newline}    {lineno++;}

{whitespace} {}

.            {fprintf(stderr,
                "Lexical error!  Line %d, unexpected character \"%s\".\n",
                lineno, yytext);
              exit(1);}

%%
