/*-------------------------------------------------------------------------

File        : lrn_fnames.h

Autor       : Stephan Schulz

Inhalt      : Deklarationen zu Funktionen, die mit Filenamen umgehen.

Aenderungen : <1> 20.8.1994 neu

-------------------------------------------------------------------------*/

#ifndef _lrn_fnames

#define _lrn_fnames

#include <sys/param.h>
#include "pcl_strings.h"


/*-----------------------------------------------------------------------*/
/*                   Typdeklarationen                                    */
/*-----------------------------------------------------------------------*/




/*-----------------------------------------------------------------------*/
/*                 Deklaration exportierter Funktionen                   */
/*-----------------------------------------------------------------------*/

String_p FileName(char* name);
char*    BaseName(char* name);
String_p NormName(char* name);
String_p DirNormName(char* name);

#endif


/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/





