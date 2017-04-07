/*****************************************************************************/
/*                                                                           */
/*  Projekt      : TEAM-COMPLETION                                           */
/*                                                                           */
/*  Modul        : exp_prepare_t_select                                      */
/*                                                                           */
/*  Author       : Martin Kronenburg, 1993                                   */
/*                                                                           */
/*  Beschreibung : In diesem Modul werden alle Funktionen zur Verfuegung ge- */
/*                 stellt, die bei der Planungsphase fuer den naechsten Zy-  */
/*                 klus, was die Experten anbetrifft, benoetigt werden.      */
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
#include "exp_prepare_t_select.h"
#include "exp_value_exp.h"
#include "exp_def_break.h"
#include "exp_class_nont_exp.h"
#include "expert.h"
#include "team.h"
#define __j(name) {#name , name},

/*****************************************************************************/
/*                                                                           */
/*                            Globale Variablen                              */
/*                                                                           */
/*****************************************************************************/
/* Bewertung des besten Losers; wird bei den Abbruchkriterien benoetigt.     */
double     BestLoser;

/*****************************************************************************/
/*                                                                           */
/*                 Forward-Deklarationen interner Funktionen                 */
/*                                                                           */
/*****************************************************************************/

static void exp_in_bewertungsliste_einfuegen ( short exp_nr, double bewertung );

static void JudgeNonWinners ( void );

/*****************************************************************************/
/*                                                                           */
/*                  Hilfsfunktionen fuer PrepareTeamSelection                */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  initialisiere_bewertungsliste                            */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Die globalen Daten der Bewertungsliste fuer Experten     */
/*		    werden initialisiert, d.h. die bisherigen Daten werden   */
/*                  geloescht. 						     */
/*                                                                           */
/*  Globale Var. :  OrderedExpStart                                          */
/*                  OrderedExpLength 					     */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/
void initialisiere_bewertungsliste ( void )
{
  OrderedExpLength = 0;
  OrderedExpStart = LISTENENDE;
} /* Ende von initialisiere_bewertungsliste */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  exp_in_bewertungsliste_einfuegen                         */
/*                                                                           */
/*  Parameter    :  Expertennummer                                           */
/*		    Bewertung des Experten  				     */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Der angegebene Experte wird gemaess seiner angegebenen   */
/*		    Bewertung in die sortierte Liste OrderedExpList einge-   */
/*                  tragen.						     */
/*                                                                           */
/*  Globale Var. :  OrderedExpList                                           */
/*                  OrderedExpLength                                         */
/*                  OrderedExpStart                                          */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

static void exp_in_bewertungsliste_einfuegen ( short exp_nr, double bewertung )
{
  short ptr;
  short vor_ptr; /* zeigt auf das vorige Element */

  OrderedExpList[OrderedExpLength].exp_nr = exp_nr;

  if ( !OrderedExpLength )
  {
    OrderedExpList[0].ptr = LISTENENDE;
    OrderedExpStart = 0;
  }
  else
  {
    ptr = vor_ptr = OrderedExpStart;
    do
    {
      if ( ExpertGlobalInfo[OrderedExpList[ptr].exp_nr].akt_bewertung > bewertung )
      {
	vor_ptr = ptr;
	ptr = OrderedExpList[ptr].ptr;
      }
      else
      {
	if ( ptr == vor_ptr )
	{
	  OrderedExpStart = OrderedExpLength;
	}
	else
	{
	  OrderedExpList[vor_ptr].ptr = OrderedExpLength;
	}
	
	OrderedExpList[OrderedExpLength].ptr = ptr;
	break;
      }
    } while ( ptr != LISTENENDE );

    if ( ptr == LISTENENDE )
    {
      OrderedExpList[vor_ptr].ptr = OrderedExpLength;
      OrderedExpList[OrderedExpLength].ptr = LISTENENDE;
    }
  }
  OrderedExpLength++;
} /* Ende von exp_in_bewertungsliste_einfuegen */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  exp_aus_bewertungsliste_lesen                            */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  Nummer des Experten, der in der Liste an erster Stelle   */
/*		    steht, d.h. der am besten bewertet wurde.                */
/*                                                                           */
/*  Beschreibung :  Falls die Liste nicht leer ist, wird der Experte zu-     */
/*		    rueckgeliefert, der an erster Stelle steht; dies sollte  */
/*		    eigentlich der sein, der zuvor am besten ducrh den Leiter*/
/*		    bewertet wurde. Dieser wird zurueckgegeben und dann ge-  */
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
/*  Globale Var. :  OrderedExpList                                           */
/*                  OrderedExpLength                                         */
/*                  OrderedExpStart                                          */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

short exp_aus_bewertungsliste_lesen ( void )
{
  short bester_exp;

  if ( OrderedExpStart == LISTENENDE )
  {
    return LISTENENDE;
  }

  bester_exp = OrderedExpList[OrderedExpStart].exp_nr;

  /* Loeschen des bisher besten Experten : */
  OrderedExpStart = OrderedExpList[OrderedExpStart].ptr;

  return bester_exp;
} /* Ende von exp_aus_bewertungsliste_lesen */



/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  exp_aus_bewertungsliste_loeschen                         */
/*                                                                           */
/*  Parameter    :  Nummer des Experten, der geloescht werden soll.          */
/*                                                                           */
/*  Returnwert   :  true, falls der Experte in der Liste vorkam und geloescht*/
/*                        wurde                                              */
/*                  false, sonst                                             */
/*                                                                           */
/*  Beschreibung :  Das Feld wird solange durchlaufen, bis der angegebene Ex-*/
/*                  perte erreicht ist. Ein Anpassung der Variablen          */
/*                  OrderedExpLength erfolgt NICHT!! Die geloeschte Feldkom- */
/*		    ponente kann ohne Initialisierung des gesamten Feldes    */
/*                  nicht mehr verwendet werden.                             */
/*                  Kommt der angegebene Experte in dem Feld nicht vor, so   */
/*                  passiert gar nichts.                                     */
/*                                                                           */
/*  Globale Var. :  OrderedExpList                                           */
/*		    OrderedExpStart					     */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

bool exp_aus_bewertungsliste_loeschen ( short exp_nr )
{
  short lauf_ptr;

  if ( OrderedExpStart == LISTENENDE )
  {
    return false;
  }

  if ( exp_nr == OrderedExpList[OrderedExpStart].exp_nr )
  {
    OrderedExpStart = OrderedExpList[OrderedExpStart].ptr;
  }

  lauf_ptr = OrderedExpStart;
  while ( OrderedExpList[lauf_ptr].ptr != LISTENENDE )
  {
    if ( OrderedExpList[OrderedExpList[lauf_ptr].ptr].exp_nr == exp_nr )
    {
      OrderedExpList[lauf_ptr].ptr = 
		  OrderedExpList[OrderedExpList[lauf_ptr].ptr].ptr;
      return true;
    }

    lauf_ptr =  OrderedExpList[lauf_ptr].ptr;
  }

  return false;
} /* Ende von exp_aus_bewertungsliste_loeschen */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  JudgeNonWinners                                          */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Es werden die Experten, die im letzten Team waren, aber  */
/* 		    nicht in die Siegerliste aufgenommen wurden, bewertet    */
/*		    und in die Liste OrderedExpList eingetragen. Bei der Ab- */
/*                  arbeitung der Loser-Liste wird noch die Bewertung des am */
/*                  besten bewerteten Losers fuer den Test der Abbruchkrite- */
/*		    rien gesichert.                                          */
/*                                                                           */
/*  Globale Var. :  ExpertGlobalInfo                                         */
/*                  ExpLoserList                                             */
/*                  ExpLoserListLength                                       */
/*                  ExpAverageList					     */
/*		    ExpAverageListLength				     */
/*		    BestLoser 						     */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

static void JudgeNonWinners ( void )
{
  short i;
  double akt_bew;


  /* Bewerten der Durchschnittstypen des letzten Zyklus                      */
  for ( i=0; i<ExpAverageListLength; i++)
  {
    akt_bew = ExpertGlobalInfo[ExpAverageList[i]].akt_bewertung 
	    = ValueExperts( ExpAverageList[i] );
    
    exp_in_bewertungsliste_einfuegen( ExpAverageList[i], akt_bew );
  }

  /* Bewerten der Verlierer des letzten Zyklus                               */
  if ( ExpLoserListLength )
  {
    BestLoser = ExpertGlobalInfo[ExpLoserList[0]].akt_bewertung
              = ValueExperts( ExpLoserList[0] );
    exp_in_bewertungsliste_einfuegen( ExpLoserList[0], BestLoser );

    for ( i=1; i<ExpLoserListLength; i++ )
    {
      akt_bew = ExpertGlobalInfo[ExpLoserList[i]].akt_bewertung
              = ValueExperts( ExpLoserList[i] );
      
      exp_in_bewertungsliste_einfuegen( ExpLoserList[i], akt_bew );

      if ( akt_bew > BestLoser )
      {
	BestLoser = akt_bew;
      }
    }
    if ( PlanDocum )
    {
      fprintf( plan_doc,  "    Die Beurteilung des besten Losers ist : %f\n", BestLoser );
    }
  }
  else 
  { 
    if ( PlanDocum )
    {
      fprintf( plan_doc,  "    Es gab keinen Loser.\n");
    }
    BestLoser = NOLOSER;
  }
} /* Ende von JudgeNonWinners */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  PrepareTeamSelection                                     */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  In dieser Komponente werden zunaechst die Experten bewer-*/
/*                  tet, die nach dem letzten Zyklus nicht in die Siegerliste*/
/*                  eingetragen wurden. Dann werden, solange die Abbruchkri- */
/*                  terien nicht erfuellt sind, die Experten aus der         */
/*                  PraeOrderList bewertet. Vorausgesetzt es gibt noch Exper-*/
/*                  ten in dieser Liste. Die bewerteten Experten werden      */
/*                  jeweils in der Liste OrderedExpList abgelegt.            */
/*                                                                           */
/*  Globale Var. :  OrderedExpList                                           */
/*                  OrderedExpStart                                          */
/*                  OrderedExpLength                                         */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

void PrepareTeamSelection ( void )
{
  short akt_exp;

  double  akt_bew;

  if ( PlanDocum )
  {
    fprintf( plan_doc,  "\n\nBeurteilung der Experten  :\n");
    fprintf( plan_doc,  "  Zuerst die Nicht-Sieger des letzten Zyklus :\n");
  }

  JudgeNonWinners();

  if ( PlanDocum )
  {
    fprintf( plan_doc,  "\n  Jetzt die Experten aus der Eignungsliste :\n");
  }

  while ( ( TestFurtherExperts() ) && 
	( ( akt_exp = exp_aus_eignungsliste_lesen() ) != LISTENENDE ) )
  {
    akt_bew = ExpertGlobalInfo[akt_exp].akt_bewertung = ValueExperts( akt_exp );

    exp_in_bewertungsliste_einfuegen( akt_exp, akt_bew );

    abbruch_daten_aktualisieren( akt_bew );
  }

  if ( PlanDocum )
  {
    fprintf( plan_doc,  "\n   Es ergibt sich folgende Bewertungsliste : \n");
    orderedliste_aus();
  }
} /* Ende von PrepareTeamSelection */
