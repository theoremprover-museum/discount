/*--------------------------------------------------------------------

File:    parseprk.h
Autor:   Stephan Schulz

Inhalt: Deklarationen der Funktionen und Datentypen zum Einlesen des
        Kurzprotokolles f"ur die paralelle Vervollst"andigung. Es
	werden einige Funktionen aus pcl_scan recycled (aber
	geaendert). 

Aenderungen : <1> 22.11.1994 Header kompletiert

----------------------------------------------------------------------*/


#ifndef _parseprk

#define _parseprk

#include "s_scanner.h"

/*-----------------------------------------------------------------------*/
/*                         Typdeklarationen                              */
/*-----------------------------------------------------------------------*/


/* Struktur erweitert: Pro Zyklus werden zwei Schrittanzahlen in den Pro- */
/* tokollfile geschrieben --> Notwendig wegen des Database-Experten       */
/* Aenderung : MK, 12.04.1994 */
typedef struct cyclelistcell
{
   int                    master;
   long                   first_steps[MAXEXPERT];
   long                   second_steps[MAXEXPERT];
   int                    confi[MAXEXPERT];
   struct cyclelistcell*  next;
}CycleListCell, *CycleList_p;



/*-----------------------------------------------------------------------*/
/*                     Deklaration exportierter Funktionen               */
/*-----------------------------------------------------------------------*/


extern long ReadPrk(char* filename,CycleList_p *res);


#endif

/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/



