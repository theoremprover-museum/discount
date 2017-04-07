


/*************************************************************************/
/*                                                                       */
/*   File:        pcl_miniprinter.h                                      */
/*                                                                       */
/*   Autor:       Stephan Schulz                                         */
/*                                                                       */
/*   Inhalt:      Deklarationen zu den Ausgabefunktionen fuer pcl.       */
/*                                                                       */
/*   Aenderungen: <12.4.1991 neu                                         */
/*                                                                       */
/*************************************************************************/

#include "pcl_defs.h"


#ifndef _pcl_miniprinter

#define _pcl_miniprinter


/*----------------------------------------------------------------------------*/
/*               Deklaration exportierter Funktionen                          */
/*----------------------------------------------------------------------------*/

void InitPrinter(FILE* S_out);

extern FILE* out;       /* Ausgabestrom, wird von InitPrinter gesetzt */


#endif


/*----------------------------------------------------------------------------*/
/*                         Ende des Files                                     */
/*----------------------------------------------------------------------------*/


