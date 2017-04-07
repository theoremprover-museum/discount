/*-------------------------------------------------------------------------

File        : pcl_strings.c  

Autor       : Stephan Schulz

Inhalt      : Deklarationen zu String-Funktionen 

Aenderungen : <1> 15.2.1991 neu 
              <2> 18.2.1991 Allgemeine Stringdeskriptoren
	      <3> 20.8.1994 AllocString() neu, Kommentarbox

-------------------------------------------------------------------------*/


#include "pcl_mem.h"


#ifndef _pcl_strings

#define _pcl_strings




/*----------------------------------------------------------------------------*/
/*                   Typdeklarationen                                         */
/*----------------------------------------------------------------------------*/

typedef struct stringcell
{
   char*         str;
   unsigned long mem;
   unsigned long len;
} StringCell, *String_p;


/*-----------------------------------------------------------------------*/
/*                            Macros                                     */
/*-----------------------------------------------------------------------*/


/* Dieses bezieht sich auf C-Strings bzw characters! */

#define LOWER_STR(str) {char* p; for(p=(str);*p;p++) *p = isupper(*p)?\
			   tolower(*p) : *p;}


/*----------------------------------------------------------------------------*/
/*           Forward-Deklarationen exportierter Funktionen                    */
/*----------------------------------------------------------------------------*/

String_p  AllocStringCell();
void      FreeStringCell(String_p junk);
String_p  AllocString();
void      FreeString(String_p junk);


char* secure_strdup(char* str);
char* AppendString(String_p strdes, char* newpart);
char* ViewString(String_p strdes);
char* GetCopyOfString(String_p strdes);
char* AppendChar(String_p strdes, char newch);
char* SetString(String_p strdes, char* string);
long  StringLength(String_p strdes);
void  ResetString(String_p strdes);


#endif


/*----------------------------------------------------------------------------*/
/*                         Ende des Files                                     */
/*----------------------------------------------------------------------------*/


