/*****************************************************************************/
/*                                                                           */
/*  Projekt      : TEAM-COMPLETION                                           */
/*                                                                           */
/*  Modul        : exp_class_nont_exp                                        */
/*                                                                           */
/*  Author       : Martin Kronenburg, 1993                                   */
/*                                                                           */
/*  Beschreibung : In diesem Modul werden alle Funktionen zur Verfuegung ge- */
/*                 stellt, die bei der Klassifizierung der Experten, die     */
/*                 nicht im Team waren erfoederlich sind.                    */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>
#include "defines.h"
#include "error.h"
#include "polynom.h"
#include "vartree.h"
#include "term.h"
#include "termpair.h"
#include "scanner.h"
#include "domain.h"
#include "referee.h"
#include "database.h"
#include "expert.h"
#include "exp_class_nont_exp.h"
#include "team.h"
#define __j(name) {#name , name},

/*****************************************************************************/
/*                                                                           */
/*                            Globale Variablen                              */
/*                                                                           */
/*****************************************************************************/
int           domain_suited;
int           winner_suited;

int           j_not_in_team;
int           not_judged;

/*****************************************************************************/
/*                                                                           */
/*                 Forward-Deklarationen interner Funktionen                 */
/*                                                                           */
/*****************************************************************************/

static void exp_in_eignungsliste_einfuegen ( short exp_nr, double eignung );

/*****************************************************************************/
/*                                                                           */
/*                  Hilfsfunktionen fuer ClassifyNonTeamExperts              */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  initialisiere_eignungsliste                              */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Die globalen Daten der Eignungsliste fuer Experten werden*/
/*		    initialisiert, d.h. die bisherigen Daten werden geloescht*/
/*                                                                           */
/*  Globale Var. :  PraeOrderLength                                          */
/*                  PraeOrderStart                                           */
/*                                                                           */
/*  Externe Var. :   keine                                                   */
/*                                                                           */
/*****************************************************************************/
void initialisiere_eignungsliste ( void )
{
  PraeOrderLength = 0;
  PraeOrderStart = LISTENENDE;
} /* Ende von initialisiere_eignungsliste */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  exp_in_eignungsliste_einfuegen                           */
/*                                                                           */
/*  Parameter    :  Expertennummer                                           */
/*		    Eignung dieses Experten 				     */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Der angegebene Experte wird gemaess seiner angegebenen   */
/*		    Eignung in die sortierte Liste PraeOrderList eingetragen.*/
/*                                                                           */
/*  Globale Var. :  PraeOrderList                                            */
/*                  PraeOrderLength                                          */
/*                  PraeOrderStart                                           */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

static void exp_in_eignungsliste_einfuegen ( short exp_nr, double eignung )
{
  short ptr;
  short vor_ptr; /* zeigt auf das vorige Element */

  PraeOrderList[PraeOrderLength].exp_nr = exp_nr;

  if ( !PraeOrderLength )
  {
    PraeOrderList[0].ptr = LISTENENDE;
    PraeOrderStart = 0;
  }
  else
  {
    ptr = vor_ptr = PraeOrderStart;
    do
    {
      if ( ExpertGlobalInfo[PraeOrderList[ptr].exp_nr].eignung > eignung )
      {
	vor_ptr = ptr;
	ptr = PraeOrderList[ptr].ptr;
      }
      else
      {
	if ( ptr == vor_ptr )
	{
	  PraeOrderStart = PraeOrderLength;
	}
	else
	{
	  PraeOrderList[vor_ptr].ptr = PraeOrderLength;
	}
	
	PraeOrderList[PraeOrderLength].ptr = ptr;
	break;
      }
    } while ( ptr != LISTENENDE );

    if ( ptr == LISTENENDE )
    {
      PraeOrderList[vor_ptr].ptr = PraeOrderLength;
      PraeOrderList[PraeOrderLength].ptr = LISTENENDE;
    }
  }
  PraeOrderLength++;
} /* Ende von exp_in_eignungsliste_einfuegen */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  exp_aus_eignungsliste_lesen                              */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  Nummer des Experten, der in der Liste an erster Stelle   */
/*		    steht, d.h. der am besten geeignet ist.                  */
/*                                                                           */
/*  Beschreibung :  Falls die Liste nicht leer ist, wird der Experte zu-     */
/*		    rueckgeliefert, der an erster Stelle steht; dies sollte  */
/*		    eigentlich der sein, der zuvor als am besten geeignet be-*/
/*		    urteilt wurde. Dieser wird zurueckgegeben und dann ge-   */
/*		    loescht, d.h. der naechste Zugriff mit dieser Funktion   */
/* 		    auf diese Liste liefert den naechst besseren Experten!   */
/*                  Zu beachten ist, dass die Laenge der Liste nicht ange-   */
/*                  passt wird, sondern nur der Startkopf wird verschoben.   */
/*                  Es koennen also einmal vergebene Plaetze nur dann wieder */
/*                  fuer neue Eintraege geleert werden, indem man die gesamte*/
/*                  Liste neu initialisiert, also loescht. Diese Einschraen- */
/*                  kung genuegt fuer diese Zwecke hier und ist natuerlich   */
/*                  effizienter. 					     */
/*                                                                           */
/*  Globale Var. :  PraeOrderList                                            */
/*                  PraeOrderLength                                          */
/*                  PraeOrderStart                                           */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

short exp_aus_eignungsliste_lesen ( void )
{
  short bester_exp;

  if ( PraeOrderStart == LISTENENDE )
  {
    return LISTENENDE;
  }

  bester_exp = PraeOrderList[PraeOrderStart].exp_nr;

  /* Loeschen des bisher besten Experten : */
  PraeOrderStart = PraeOrderList[PraeOrderStart].ptr;

  return bester_exp;
} /* Ende von exp_aus_eignungsliste_lesen */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  ClassifyNonTeamExperts                                   */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Diese Komponente ordnet alle Experten, die in der Kon-   */
/*  		    figurationsdatei aufgefuehrt sind und nicht im letzten   */
/*                  Team waren, in einer geordneten Liste an. Aus dieser     */
/*		    geordneten Liste kann der Leiter dann stets den ersten   */
/*   		    Experten zur Beurteilung auswaehlen. Hier wird sozusagen */
/*                  eine Vorauswahl, ein Eignungstest, fuer die Experten     */
/*                  durchgefuehrt. Dieser Eignungstest geschieht nach 2 Kri- */
/*                  terien :						     */
/*                  a) Daten des LZG : d.h., ob der Experte bei einer rele-  */
/*                     vanten Domaene (domain_suited aufaddieren) oder beim  */
/*                     Sieger als geeignet aufgefuehrt ist (winner_suited).  */
/*          	    b) Wartezeit des Experten, d.h. wie lange der Experte    */
/*                     nicht mehr im Team bzw nicht mehr beurteilt wurde.    */
/*                     Nicht beurteilt -> not_judged aufaddieren             */
/*		       beurteilt, aber nicht im Team : j_not_in_team         */
/*                  Wenn ein Experte eins der Kriterien erfuellt, wird je-   */
/*		    weils eine bestimmte Konstante aufaddiert.               */
/*                                                                           */
/*  Globale Var. :  ExpertGlobalInfo                                         */
/* 		    ExpertTeamInfo       				     */
/*                                                                           */
/*  Externe Var. :  WinnerHost                                               */
/*                  TeamDomCount					     */
/*                  DomainTeamInfo                                           */
/*                                                                           */
/*****************************************************************************/

void ClassifyNonTeamExperts( void )
{
  short i;
  short j;

  double guete;

  if ( PlanDocum )
  {
    fprintf( plan_doc,  "\n\nKlassifizierung der Experten, die nicht im Team waren :\n");
  }

  /***************************************************************************/
  /* Betrachtet werden nun die in der Konfigurationsdatei angegeben Experten,*/
  /* die nicht im letzten Zyklus eingesetzt waren und nicht bereits im Team  */
  /* sind, als Domaenenstart-Experten z.B. .                                 */
  /***************************************************************************/
  for ( i=0; i<ExpertCount; i++ )
  {     /* Sonderbehandlung der STATIC +INITIAL nicht notwendig -> diese     */
	/* waren wenn auch im letzten Team -> fallen direkt mit der naechsten*/
	/* Abfrage weg.                                                      */
    if ( exp_im_alten_team( i ) || exp_im_neuen_team( i ) )
    {
      continue;
    }
    
    /* Dieser Experte ist also in der Konfigurationsdatei aufgefuehrt und    */
    /* war nicht im letzten Team eingesetzt und ist noch nicht im neuen.     */
    ExpertGlobalInfo[i].eignung = (double)ExpertGlobalInfo[i].wartekonto;

    if ( PlanDocum )
    {
      fprintf( plan_doc,  "    \nEingestuft wird der Experte : %20s\n", get_exp_name( i ) );
      fprintf( plan_doc,  "    Bisheriges Wartekonto : %d\n", ExpertGlobalInfo[i].wartekonto );
    }
      

    /*******************/
    /* LZG - Kriterien */
    /*******************/
      /***********************************/
      /* Eignung f"ur relevante Dom"anen */
      /***********************************/
    for ( j=0; j<TeamDomCount; j++ )
    {
      if ( exp_bei_dom_geeignet( i, DomainTeamInfo[j].dom_nr, &guete ) )
      {
        if ( PlanDocum )
	{
	  fprintf( plan_doc,  "    Bei Domaene %20s geeignet --> %2f Eignungspunkte.\n", 
		   DomainGlobalInfo[DomainTeamInfo[j].dom_nr].dom_name, domain_suited * guete );
	}
        ExpertGlobalInfo[i].eignung += (double)domain_suited * guete; 
      }
    } /* Ende von for */
    
      /******************************/
      /* Teamf"ahigkeit beim Sieger */
      /******************************/
    if ( exp_bei_exp_geeignet( i, ExpertTeamInfo[WinnerHost].exp_nr, &guete ) )
    {
      if ( PlanDocum )
      {
	fprintf( plan_doc,  "    beim Sieger geeignet --> %2f Eignungspunkte.\n", winner_suited * guete);
      }
      ExpertGlobalInfo[i].eignung += winner_suited * guete;
    }

    /**********************/
    /* Wartezeitkriterien */
    /**********************/
    if ( !ExpertGlobalInfo[i].beurteilt )
    {
      ExpertGlobalInfo[i].eignung += not_judged;
      ExpertGlobalInfo[i].wartekonto += not_judged;
      if ( PlanDocum )
      {
	fprintf( plan_doc,  "    Nicht beurteilt wurden --> %d Punkte aufs Wartekonto\n", not_judged );
      }
    }
    else
    {
      ExpertGlobalInfo[i].eignung += j_not_in_team;
      ExpertGlobalInfo[i].wartekonto += j_not_in_team;
      if ( PlanDocum )
      {
	fprintf( plan_doc,  "    Beurteilt, aber nicht im Team --> %d Punkte aufs Wartekonto\n", j_not_in_team );
      }
    }


    if ( PlanDocum )
    {
      fprintf( plan_doc,  "    GESAMTEIGNUNG DES EXPERTEN : %2f\n", ExpertGlobalInfo[i].eignung );
    }
    exp_in_eignungsliste_einfuegen( i, ExpertGlobalInfo[i].eignung );
  } /* Ende von for ( i ... */

  if ( PlanDocum )
  {
    fprintf( plan_doc,  "\n   Es ergibt sich folgende Eignungsliste : \n");
    prae_orderedliste_aus(); 
  }
  
} /* Ende von ClassifyNonTeamExperts*/
