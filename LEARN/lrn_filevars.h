/*-------------------------------------------------------------------------

File        : lrn_filevars.h

Autor       : Stephan Schulz

Inhalt      : Funktionen und Datentypen zum Parsen von Variablen, die
              in Files gespeichert werden.

Aenderungen : <1> 15.12.196 neu

-------------------------------------------------------------------------*/

#ifndef _lrn_filevars

#define _lrn_filevars

#include "pcl_doio.h"
#include "pcl_scan.h"


/*-----------------------------------------------------------------------*/
/*                   Typdeklarationen                                    */
/*-----------------------------------------------------------------------*/

/* Describes a single variable. Only one of {b,l,s}store should be
   different from NULL. This describes the type of the file variable
   as well as the C-Variable where the value shoud be stored. */

typedef struct
{
   char*  var;
   BOOL*  bstore;
   long*  lstore;
   char** sstore;
}FVarCell;


/*-----------------------------------------------------------------------*/
/*                 Deklaration exportierter Funktionen                   */
/*-----------------------------------------------------------------------*/

long ReadFileVariables(char* file, FVarCell vars[]);


#endif

/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/





