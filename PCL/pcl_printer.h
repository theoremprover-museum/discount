

/*************************************************************************/
/*                                                                       */
/*   File:        pcl_printer.h                                          */
/*                                                                       */
/*   Autor:       Stephan Schulz                                         */
/*                                                                       */
/*   Inhalt:      Deklarationen zu den Ausgabefunktionen fuer pcl.       */
/*                                                                       */
/*   Aenderungen: <12.4.1991 neu                                         */
/*                                                                       */
/*************************************************************************/

#include "pcl_defs.h"


#ifndef _pcl_printer

#define _pcl_printer


/*----------------------------------------------------------------------------*/
/*               Deklaration exportierter Funktionen                          */
/*----------------------------------------------------------------------------*/

void InitPrinter(FILE* S_out);

extern FILE* out;       /* Ausgabestrom, wird von InitPrinter gesetzt */


#endif


/*----------------------------------------------------------------------------*/
/*                         Ende des Files                                     */
/*----------------------------------------------------------------------------*/


