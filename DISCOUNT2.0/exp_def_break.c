/*****************************************************************************/
/*                                                                           */
/*  Projekt      : TEAM-COMPLETION                                           */
/*                                                                           */
/*  Modul        : exp_def_break                                             */
/*                                                                           */
/*  Author       : Martin Kronenburg, 1993                                   */
/*                                                                           */
/*  Beschreibung : In diesem Modul werden alle Funktionen zur Verfuegung ge- */
/*                 stellt, die fuer das Festsetzen der Abbruchkriterien bei  */
/*                 der Expertenbeurteilung benoetigt werden.                 */
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
#include "exp_prepare_t_select.h"
#include "exp_def_break.h"
#include "team.h"
#define __j(name) {#name , name},

/*****************************************************************************/
/*                                                                           */
/*                            Globale Variablen                              */
/*                                                                           */
/*****************************************************************************/
int           min_inspected_x;
int           exp_part;
int           extra_exp1;
int           loser_part;
int           extra_exp2;
double        loser_percent;
int           extra_exp3;
int           min_better_exp;

/* Die folgenden Variablen werden in DefineBreakOffs erlaeutert              */
short      MaxLookAtExp;
short      MaxBetterExp;

/* Anzahl der Experten, die bisher beurteilt wurden.                         */
short      LookAtExp;

/* Anzahl der Experten, die bisher beurteilt wurden und besser als die Loser */
/* waren.                                                                    */
short      BetterExp;

/*****************************************************************************/
/*                                                                           */
/*                 Forward-Deklarationen interner Funktionen                 */
/*                                                                           */
/*****************************************************************************/

static bool loser_ganz_schlecht ( void );

static short anzahl_zu_betrachtender_exp_best ( void );

static short anzahl_besserer_exp_best ( void );

/*****************************************************************************/
/*                                                                           */
/*                   Hilfsfunktionen fuer DefineBreakOffs                    */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  loser_ganz_schlecht                                      */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  true, wenn alle Loser unter einer bestimmten Schranke    */
/*                        liegen;   					     */
/*                  false sonst						     */
/*                                                                           */
/*  Beschreibung :  Es werden die absoluten Beurteilungen aller Loser        */
/*		    getestet, dabei wird stets eine Schranke berechnet.      */
/*                  Diese Schranke wird wie folgt berechnet:                 */
/*                    falls die Verlieregrenze  >= 0 ist :                   */
/*                     loser_percent * verlierergrenze                       */
/*                    falls die Verlieregrenze < 0 ist :                     */
/*                     verlierergrenze/loser_percent                         */
/*                  leiegen alle Loser unterhalb dieser Schranke, so wird    */
/*                  true zur"uckgeliefert.                                   */
/*                  Gibt es keine Loser wird false zur"uckgeliefert.         */
/*                                                                           */
/*  Globale Var. :  ExpLoserListLength                                       */
/*                  ExpLoserList                                             */
/*                  ExpertGlobalInfo					     */
/*                                                                           */
/*  Externe Var. :  CycleCount                                               */
/*                                                                           */
/*****************************************************************************/

static bool loser_ganz_schlecht ( void )
{
  short   i;
  double  schranke;

  if( !ExpLoserListLength )
  {
    return false;
  }

  for ( i=0; i<ExpLoserListLength; i++ )
  {
    /**************************/
    /* Festlegen der Schranke */
    /**************************/
    if( ExpertGlobalInfo[ExpLoserList[i]].verlierergrenze >= 0 )
    {
      schranke = ExpertGlobalInfo[ExpLoserList[i]].verlierergrenze * loser_percent;
    }
    else
    {
      schranke = ExpertGlobalInfo[ExpLoserList[i]].verlierergrenze / loser_percent;
    }

    /***********************/
    /* Testen der Schranke */
    /***********************/
    if ( ExpertGlobalInfo[ExpLoserList[i]].abs_bewertung[CycleCount-1] > schranke )
    {
      return false;
    }
  }

  return true;
} /* Ende von loser_ganz_schlecht */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  anzahl_zu_betrachtender_exp_best                         */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  Anzahl der Experten.die betrachtet werden muessen        */
/*                                                                           */
/*  Beschreibung :  siehe die Beschreibung von DefineBreakOffs               */
/*                                                                           */
/*  Globale Var. :  ExpLoserListLength                                       */
/*                                                                           */
/*  Externe Var. :  ExpertCount                                              */
/*                  CycleCount                                               */
/*                  HostCount                                                */
/*                                                                           */
/*****************************************************************************/

static short anzahl_zu_betrachtender_exp_best ( void )
{
  short anzahl = min_inspected_x;

  anzahl += (ExpertCount/exp_part) * extra_exp1 +
	    (ExpLoserListLength/loser_part) * extra_exp2;
  
  if ( loser_ganz_schlecht() )
  {
    anzahl += extra_exp3;
  }
  
  /* Im ersten Zyklus muss sichergestellt sein, dass jeder Rechner besetzt   */
  /* werden kann; deswegen muessen mindestens HostCount Experten vor dem     */
  /* ersten Zyklus betrachtet werden.                                        */
  /* Ansonsten wird stets wenigstens einer betrachtet!                       */
  if ( !CycleCount )
  {
    return max( HostCount, anzahl );
  }

  return max( 1, anzahl );
} /* Ende von anzahl_zu_betrachtender_exp_best */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  anzahl_besserer_exp_best                                 */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  Mindestanzahl der besseren Experten                      */
/*                                                                           */
/*  Beschreibung :  Bestimmt die Anzahl der Experten, die besser sein        */
/*                  muessen als die Loser, damit keine weiteren Experten mehr*/
/*                  beurteilt werden.                                        */
/*                  Dies ist zur Zeit die Haelfte der Anzahl der Loser.      */
/*                  Aber mindestens einer.           		 	     */
/*                                                                           */
/*  Globale Var. :  ExpLoserListLength                                       */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

static short anzahl_besserer_exp_best ( void )
{
  return max( min_better_exp, ExpLoserListLength );
} /* Ende von anzahl_besserer_exp_best */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  DefineBreakOffs                                          */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Diese Komponente legt die Abbruchkriterien fest fuer die */
/*                  Schleife, in der die Experten beurteilt und in eine ge-  */
/*                  ordnete Liste eingetragen werden.      		     */
/*  		    Fuer den Abbruch koennen mehrere Kriterien herangezogen  */
/*                  werden; zur Zeit bestimmt diese Komponente folgende Werte*/
/*                  a) wieviele Experten maximal betrachtet werden           */
/*                  b) wieviele Experten besser sein muessen als die Loser   */
/*                     des letzten Zyklus, damit abgebrochen wird            */
/*                  Diese Werte werden in den globalen Variablen (s.u.) abge-*/
/*                  legt. 						     */
/*                  zu a) Es wird von einer minimalen Anzahl von Experten    */
/*                        ausgegangen, die auf jeden Fall betrachtet werden :*/
/*                        min_inspected_x.    				     */
/*                        Fuer je exp_part zur Verfuegung stehender Experten */
/*                        erhoeht sich diese Anzahl um extra_exp1;           */
/*                        Fuer je loser_part Experten in der Loser-Liste     */
/*                        erhoeht sich die Zahl um extra_exp2;               */
/*                        Wenn alle Loser schlechter als DOWN_LIMIT beurteilt*/
/*		          wurden, erhoeht sich die Zahl um extra_exp3        */
/*                    Dies wird in der Funktion   			     */
/*                    anzahl_zu_betrachtender_exp_best realisiert            */
/*                  zu b) siehe Beschreibung von anzahl_besserer_exp_best    */
/*                                                                           */
/*                  Ausserdem werden die globalen Zaehlvariablen LookAtExp   */
/*   		    und BetterExp auf Null gesetzt.                          */
/*                                                                           */
/*  Globale Var. :  MaxLookAtExp                                             */
/*                  MaxBetterExp					     */
/*                  LookAtExp                                                */
/*                  BetterExp                                                */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

void DefineBreakOffs ( void )
{

  MaxLookAtExp = anzahl_zu_betrachtender_exp_best();

  MaxBetterExp = anzahl_besserer_exp_best();

  LookAtExp = BetterExp = 0;

  if ( PlanDocum )
  {
    fprintf( plan_doc,  "\n\nFestlegen der Abbruchkriterien  :\n");
    fprintf( plan_doc,  "    Beurteilt werden maximal %d Experten.\n", MaxLookAtExp);
    fprintf( plan_doc,  "    %d Experten muessen mindestens besser sein als der beste Loser.\n", MaxBetterExp );
  }
} /* Ende von DefineBreakOffs */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  abbruch_daten_aktualisieren                              */
/*                                                                           */
/*  Parameter    :  Bewertung des zuletzt beurteilten Experten               */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Die beiden globalen Variablen LookAtExp und BetterExp    */
/*                  werden in Abhaengigkeit der uebergebenen Bewertung aktu- */
/*                  alisiert.                                                */
/*                                                                           */
/*  Globale Var. :  LookAtExp                                                */
/*		    BetterExp						     */
/*                  BestLoser          					     */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

void abbruch_daten_aktualisieren ( double bewertung )
{
  LookAtExp++;

  if ( ( BestLoser != NOLOSER ) && ( bewertung > BestLoser ) )
  {
    BetterExp++;
  }
  
} /* Ende von abbruch_daten_aktualisieren */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  TestFurtherExperts                                       */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  true, falls weiter Experten beurteilt werden solle       */
/*                  false sonst                                              */
/*                                                                           */
/*  Beschreibung :  Diese Komponente ueberprueft, ob die Abbruchkriterien    */
/*                  erfuellt sind oder noch weiter Experten bewertet werden  */
/*                  sollen.                                                  */
/*                                                                           */
/*  Globale Var. :  MaxLookAtExp                                             */
/*                  MaxBetterExp					     */
/*                  LookAtExp                                                */
/*                  BetterExp    					     */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

bool TestFurtherExperts ( void )
{
  if ( ( MaxLookAtExp == LookAtExp ) || ( MaxBetterExp == BetterExp ) )
  {
    return false;
  }
  
  return true;
} /* Ende von TestFurtherExperts */
