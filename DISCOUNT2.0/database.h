
/*
//=============================================================================
//      Projekt:        TEAM-COMPLETION
//
//      Header:         database
//
//      Author:         Werner Pitz, 1991
//
//      Beschreibung:   Einlesen einer Aufgabenstellung
//-----------------------------------------------------------------------------
//      Typdefinition fuer F_Info von database.c in database.h uebertragen
//      15.01.1993, Martin Kronenburg
//
//      $Log: database.h,v $
//      Revision 1.1  1991/09/25  12:02:10  pitz
//      Initial revision
//
//
//=============================================================================
*/

#ifndef     __DATABASE
#define     __DATABASE

#include "scanner.h"

/*****************************************************************************/
/*                                                                           */
/*                            Typdefinitionen                                */
/*                                                                           */
/*****************************************************************************/

/* Die folgenden Datenstrukturen werden beim Bestimmen aller Substitutionen  */
/* zweiter Ordnung zwischen den Funktionssymbolen einer Domaenenspezifikation*/
/* und den Funktionssymbolen aus der Aufgabenstellung benoetigt.             */

/* Jedes Funktionssymbol aus der Aufgabenstellung, also eins mit dem gematcht*/
/* werden kann, bekommt eine solche Struktur zugeordnet. In ihr ist die Num- */
/* mer des Funktionssymbols gemaess Signatur aus der Aufgabenstellung abge-  */
/* legt und eine Marke, die angibt, ob dieses Funktionssymbol in dem aktuell */
/* betrachteten Substitution bereits belegt ist.                             */
typedef struct { function fktnr;
		 bool     belegt;
	       } SubstituierendesFSymb;

/*****************************************************************************/
/*                                                                           */
/*                            externe Variablen                              */
/*                                                                           */
/*****************************************************************************/

/* Anzahl der in diesem Zyklus betrachteten Domaenen; konnten alle angegebe- */
/* nen Domaene betrachtet werden, so hat diese Variable einen negativen Wert.*/
extern  short           LookedDomains;

/* In der folgenden Matrix werden nach Stelligkeiten sortiert die Funktions- */
/* symbole aus der Aufgabenstellung abgelegt. Der erste Index gibt zugleich  */
/* die Stelligkeit der darunter abgespeicherten Funktionssymbole an.         */
SubstituierendesFSymb    SubstMatrix[MAXARITY][MAXFUNCTION];

/* In dem folgenden Feld wird fuer jede Stelligkeit abgelegt, wieviele Funk- */
/* tionssymbole es in der Aufgabenstellung eben mit dieser Stelligkeit gibt. */

extern short            ArityLength[MAXARITY];


               /************************************************/
               /* Wenn der Database-Experte eingesetzt wird,   */
               /* setzt dieser diese Variable auf true.        */
               /* Falls der Experte, der nach Erkennen einer   */
               /* Dom"ane auf dem Prozessor des Database-Exper-*/
               /* ten weiterarbeitet, die Beweisaufgabe l"osen */
               /* kann, zeigt diese Variable an, da"s f"ur den */
               /* Reproduktionslauf als erstes der Database-   */
               /* Experte eingesetzt werden mu"s.              */
               /* Dies wird in der Funktion Completed in       */
               /* complet.c abgefragt.                         */
               /************************************************/
extern bool              database_config;

/* Nummer der aktuell zu bearbeitenden Domaene */


extern short             AktDomNr;
extern Symbol            SuchModus;
extern bool              no_rule_equ;


/*****************************************************************************/
/*                                                                           */
/*                        exportierte Funktionen                             */
/*                                                                           */
/*****************************************************************************/


bool    FindRules ( void );

bool    FindAndTestAllMatches ( void );

void    DatabaseExpert ( long sec, short dom_anz, long *dom_nr, Symbol such_modus );

#endif


