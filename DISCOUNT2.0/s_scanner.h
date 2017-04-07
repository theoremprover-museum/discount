/*-------------------------------------------------------------------------

File        : s_scanner.h

Autor       : Stephan Schulz

Inhalt      : This file deals with  the implementation of a simple,
              general purpose scanner for use with new extensions to
	      the DISCOUNT systems. DISCOUNT's original scanner is
	      rather unwieldy and inflexible - this is intended as a
	      base for all further extensions. It will replace the
	      scanner in parseprk.[ch]. This file is, in part, based
	      on pcl_scan.h and the old scanner in parseprk.h

Aenderungen : <1> 4.12.1994 neu

-------------------------------------------------------------------------*/

#ifndef _s_scanner

#define _s_scanner

#include "defines.h"
#include "newmem.h"

/*-----------------------------------------------------------------------*/
/*                   Typdeklarationen                                    */
/*-----------------------------------------------------------------------*/



typedef enum
{
   NoToken,          /* nichts       */
   identifier,       /* Subsumes ident, idnum, positive, but _not_ */
		     /* number or negative */ 
   ident,            /* Any identier not terminating with a number */
   idnum,            /* Mixed identifier, terminating with a number */
   positive,         /* Identifier consisting only of the digits 0..9 */
   negative,         /* As positive, but with leading - */
   number,           /* Subsumes positive and negative */
   string,           /* Anything inclosed in double quotes "" */
   equal_sign,       /* "="          */
   openbracket,      /* "("          */
   closebracket,     /* ")"          */
   colon,            /* ":"          */
   comma,            /* ","          */
   semicolon,        /* ";"          */
   hyphen,           /* "-", hyphen directly followed by a digit will */
		     /*      be translated into a negative (subtype of */
		     /*      number) */ 
   dot,              /* "."          */
   comment           /* Comments will simply be skipped by */
		     /* NextRealToken(), NextToken() will only produce */
		     /* the token but not the text to gain performance */
		     /* and lessen the risk of memory leaks */
} TokenType; /* This list may be extended. You need to integrate new */
	     /* tokens into the function NextToken() and into the */
	     /* array "token_print_table" in s_scanner.c. This scanner */
	     /* at the moment supports only a one character */
	     /* look-ahead.*/ 




/*-----------------------------------------------------------------------*/
/*                           Macros                                      */
/*-----------------------------------------------------------------------*/

#define isidchar(ch) (isalnum(ch) || (ch) == '_')
#define ischar(ch)   ((ch)!=EOF)


/*-----------------------------------------------------------------------*/
/*          Deklaration exportierter Funktionen und Variablen            */
/*-----------------------------------------------------------------------*/


extern TokenType AktToken;           /* Type of the current token */
extern char      AktLiteral[LENGTH]; /* Input character  sequence that */
				     /* translated to AktToken - */
				     /* relevant for  composite */
				     /* tokens, as identifiers and */
				     /* numbers. LENGTH is defined in */
				     /* defines.h, and I use this */
				     /* rather than IDENTLENGTH */
				     /* because it represents the */
				     /* maximal length of _any_ token, */
				     /* not just Identifiers. This */
				     /* scanner at the moment allows */
				     /* all identifiers to be of */
				     /* LENGTH lenght - if you have */
				     /* more restrictive requirements */
				     /* in your code you have to check */
				     /* this for yourself. */
extern long      AktNum;             /* Numerical value that might be */
				     /* associated with akttoken, used */
				     /* for number and idnum types  */
extern char      ErrorSpace[];       /* Space to use with sprintf to */
				     /* produce error messages */
extern bool      ColonFollows;       /* An really UGLY hack - this */
				     /* variable is set if the next */
				     /* character after the current */
				     /* identifier is a colon (:). It */
				     /* can be used to determine */
				     /* whether an identifier is part */
				     /* of a frame (as in "Lemmas:") */
				     /* or belongs to a term - it is a */
				     /* kind of 1/2-token-look-ahead. */
				     /* It will only be set after */
				     /* scanning identifiers or */
				     /* numbers, it's value is */
				     /* undefined in all other cases */
				     /* (actually, it simply is nor */
				     /* reset at the moment). */ 


bool      FileExists(char* filename, bool silent);
void      InitScanner(char* filename); 
                 /* DISCOUNT does not support stdin and pipes, */
			/* and probably never will - so we can use a */
			/* filename for the input stream. */ 
void      EndScanner(); /* Closes file streams */
void      ScannerError(char* message);
TokenType NextToken();
TokenType NextRealToken();
bool      TestToken(TokenType token);
bool      TestIdent(TokenType token, char* literal);
void      CheckToken(TokenType token);
void      CheckIdent(TokenType token, char* literal);
void      AcceptToken(TokenType token);
void      AcceptIdent(TokenType token, char* literal);

bool      ParseBool();
double    ParseReal();

/* Functions and variables for the Source-Handler are not exported,
   but local to the module*/

#endif


/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/





