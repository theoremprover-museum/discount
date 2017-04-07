/*-------------------------------------------------------------------------

File       : pcl_prologout.h

Autor      : Stephan Schulz

Inhalt     : Definitionen zur Beweisausgabe in Prolog-Darstellung
             (f"ur Gruppe Dahn in Berlin)

Aenderungen: <1> 20.4.1992 neu

-------------------------------------------------------------------------*/


#include "pcl_eqchains.h"


#ifndef _pcl_prologout

#define _pcl_prologout


/*-----------------------------------------------------------------------*/
/*               Deklaration exportierter Funktionen                     */
/*-----------------------------------------------------------------------*/


void P_PrintEqChain(EqChain_p prt,ExStep type,long ax_lem_no);


#endif

/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/

