%option noyywrap
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compiler.h"

int lineno = 1;

#define STATE1 1
#define STATE2 2
#define STATE3 3
#define STATE4 4
%}

/* token ID NUM INT VOID IF ELSE WHILE RET LE LT GT GE EQ NE */

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
"while"      {return WHILE;}
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

"/*"         {
              // state machine to handle nested loops, increments nested_count if it gets
              // another '/*' within so it knows to expect another '*/' pair before
              // breaking out.
              int state = STATE1;
              int nested_count = 0;
              while ( state != STATE4 ) {
                // get the next character
                char c = input();

                // increment the line number if we get a newline while processing
                if ( c == '\n' ) { lineno++; }
                switch(state) {
                case STATE1:
                  // this is the initial state.  If we get a '*', go to state that handles
                  // ending comments.  If we get a '/', then go into state that handles
                  // possible nested starting comments.  Otherwise, stay in this state
                  if ( c == '*' ) {
                    state = STATE2;
                  } else if ( c == '/' ) {
                    state = STATE3;
                  }
                  break;
                case STATE2:
                  // in this state, we have already encountered a *.  Go into same state if
                  // we get another '*'.  Go into STATE1 if we get a '/' if the
                  // nested_count is >0, otherwise go into ending state.  Finally, go back
                  // to STATE1 if we get another character
                  if ( c == '*' ) {
                    state = STATE2;
                  } else if ( c == '/' ) {
                    // if the count is greater than 0 (i.e. we have nested comments), then go
                    // back to starting state and decrement the counter.  Otherwise, go to
                    // ending state and finish.
                    if ( nested_count > 0 ) {
                      state = STATE1;
                    } else {
                      state = STATE4;
                    }

                    // decrement the nested_count counter
                    nested_count--;
                  } else {
                    state = STATE1;
                  }
                  break;
                case STATE3:
                  // in this state, we already encountered a '/'.  If we get another, stay
                  // in the same state.  If we get a '*', increment nested_counter and go
                  // back to starting state.  Otherwise, just go back to starting state.
                  if ( c == '/' ) {
                    state = STATE3;
                  } else if ( c == '*' ) {
                    nested_count++;
                    state = STATE1;
                  } else {
                    state = STATE1;
                  }
                  break;
                }
              }
            }



{newline}    {lineno++;}

{whitespace} {}

.            {fprintf(stderr,
                "Lexical error!  Line %d, unexpected character \"%s\".\n",
                lineno, yytext);
              exit(1);}

%%
