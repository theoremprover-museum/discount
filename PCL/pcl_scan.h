/*-------------------------------------------------------------------------

File        : pcl_scan.h    

Autor       : Stephan Schulz

Inhalt      : Definitionen zum Scanner 

Aenderungen : <1>  2.4.1991 Uerbernahme von scan.h 
              <2> 31.8.1994 Neue Box, Macro isidchar()

-------------------------------------------------------------------------*/

#ifndef _pcl_scan

#define _pcl_scan


#include "pcl_strings.h"




/*-----------------------------------------------------------------------*/
/*                 Macros und Konstanten                                 */
/*-----------------------------------------------------------------------*/

#define isidchar(ch) (isalnum(ch) || (ch) == '_')


/*-----------------------------------------------------------------------*/
/*             Typdeklarationen fuer den Scanner                         */
/*-----------------------------------------------------------------------*/


typedef enum 
{
   NoToken,          /* nichts       */  
   Identifier,       /* Obertyp fuer die beiden folgenden, fuer accep,check */
   ident,            /* Identifier   */
   idnum,            /* mit Nummer   */
#ifdef LEARN_VERSION
   colonident,       /* Z.B. Goals:, Domain: */
#endif
   string,           /* Anything included in "" */
   number,           /* fuer AcceptTok: idnum mit textval ""  */
   openbracket,      /* "("          */
   closebracket,     /* ")"          */
   equ,              /* "="          */
   hyphen,           /* "-"          */
   r_arrow,          /* "->"         */
   comma,            /* ","          */
   fullstop,         /* "."          */
   comment,          /* "#" oder ..  */ 
   colon             /* ":"          */
} TokenType;



typedef struct token
{
   TokenType     token;
   char*         literal;
   long          litmem;
   char*         textval;
   long          textmem;
   unsigned long numval;
} Token, *Token_p;




/*-----------------------------------------------------------------------*/
/*          Deklaration exportierter Funktionen und Variablen            */
/*-----------------------------------------------------------------------*/


void InitScanner(FILE *S_in,char* S_name);
void NextToken();

#ifdef __GNUC__
volatile
#endif
void RdErr(char* message);
void PrintToken(Token_p tok);

extern Token akttoken; 


#endif

/*-----------------------------------------------------------------------*/
/*                      Ende des Files                                   */
/*-----------------------------------------------------------------------*/


