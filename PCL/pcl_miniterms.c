



/*************************************************************************/
/*                                                                       */
/*   File:        pcl_miniterms.c                                        */
/*                                                                       */
/*   Autor:       Stephan Schulz                                         */
/*                                                                       */
/*   Inhalt:      Ausgabefunktionen fuer Dummy-Terme                     */
/*                                                                       */
/*   Aenderungen: <1> 10.2.1992 neu                                      */
/*                                                                       */
/*************************************************************************/



#include "pcl_miniterms.h"




/*----------------------------------------------------------------------------*/
/*                        Exportierte Funktionen                              */
/*----------------------------------------------------------------------------*/

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void PrintTerm(char* prt)                               */
/*                    IN     char* prt                                        */
/*                                                                            */
/* Beschreibung     : Gibt den Term, auf die prt zeigt, aus.                  */
/*                                                                            */
/* Globale Variable : out                                                     */
/*                                                                            */
/* Seiteneffekte    : Ausgabe                                                 */
/*                                                                            */
/* Aenderungen      : <1> 15.5.1991 neu                                       */
/*                    <2> 10.7.1991 Aenderung fuer miniextract                */
/*                                                                            */
/******************************************************************************/

void PrintTerm(char* prt)
{
   fprintf(out,"%s",prt);
}

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void PrintTermPair(char* prt)                           */
/*                    IN     char* prt                                        */
/*                                                                            */
/* Beschreibung     : Gibt das Termpaar, auf das prt zeigt, aus.              */
/*                                                                            */
/* Globale Variable : out                                                     */
/*                                                                            */
/* Seiteneffekte    : Ausgabe                                                 */
/*                                                                            */
/* Aenderungen      : <1> 15.5.1991 neu                                       */
/*                    <2> 10.7.1991 Aenderung fuer miniextract                */
/*                                                                            */
/******************************************************************************/

void PrintTermPair(char* prt)
{
  fprintf(out,"%s",prt);
}



/*----------------------------------------------------------------------------*/
/*                         Ende des Files                                     */
/*----------------------------------------------------------------------------*/


