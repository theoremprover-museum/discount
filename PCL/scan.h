
/*************************************************************************/
/*                                                                       */
/*   File:        scan.h                                                 */
/*                                                                       */
/*   Autor:       Stephan Schulz                                         */
/*                                                                       */
/*   Inhalt:      Definitionen zum Scanner                               */
/*                                                                       */
/*   Aenderungen: <1> 11.2.1991  neu                                     */
/*                <2> 14.2.1991 Datentypen, include                      */
/*                <3> 20.2.1991 PrintToken neu                           */
/*                                                                       */
/*************************************************************************/

#include "pcl_strings.h"



/*----------------------------------------------------------------------------*/
/*                   Datentypen fuer den Scanner                              */
/*----------------------------------------------------------------------------*/



typedef enum 
{
   NoToken,          /* nichts       */  
   Identifier,       /* Obertyp fuer die beiden folgenden, fuer accep,check */
   ident,            /* Identifier   */
   idnum,            /* mit Nummer   */
   openbracket,      /* "("          */
   closebracket,     /* ")"          */
   number,           /* Unsigned int */
   equ,              /* "="          */
   r_arrow,          /* "->"         */
   assign,           /* "<=="        */
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





/*----------------------------------------------------------------------------*/
/*         Exportierte Funktionen und Variable                                */
/*----------------------------------------------------------------------------*/


void InitScanner(FILE *S_in,char* S_name);
void NextToken();

#ifdef __GNUC__
volatile
#endif
void RdErr(char* message);
void PrintToken(Token_p tok);

extern Token akttoken; 


/*----------------------------------------------------------------------------*/
/*                         Ende des Files                                     */
/*----------------------------------------------------------------------------*/


