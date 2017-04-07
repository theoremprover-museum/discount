/*****************************************************************************/
/*                                                                           */
/*  Projekt      : TEAM-COMPLETION                                           */
/*                                                                           */
/*  Modul        : exp_def_next_cycletime                                    */
/*                                                                           */
/*  Author       : Martin Kronenburg, 1993                                   */
/*                                                                           */
/*  Beschreibung : In diesem Modul werden alle Funktionen zur Verfuegung ge- */
/*                 stellt, die bei der Bestimmung der naechsten Zyklusdauer  */
/*                 benoetigt werden.                                         */
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
#include "exp_def_next_cycletime.h"
#include "team.h"
#define __j(name) {#name , name},

/*****************************************************************************/
/*                                                                           */
/*                            Globale Variablen                              */
/*                                                                           */
/*****************************************************************************/
/* Die folgenden Variablen dienen zur Bestimmung der n"achsten Zyklusdauer   */
/* Ihre Bedeutung wird bei den Funktionen erkl"art.                          */
/* Festgelegt werden k"onnen sie in der Planungsdatei, Werden sie dort nicht */
/* vereinbart, werden die Default-Werte der entsprechenden Konstanten genom- */
/* men. Diese sind exp_def_next_cycletime.h definiert.                       */

/* verwendet in DefineNextCycleTime   */
int             timebase;   /* Kann auch in der Konfigurationsdatei definiert werden */
int             tp_part;
double          goalfaktor;
double          time_extra;
double          good_team_t_fac;
double          bad_team_t_fac;

/* verwendet in gut_beurteiltes_team  */
int             upper_limit1;
int             upper_limit2;

/* verwendet in schlecht_beurteiltes_team */
int             down_limit1;
int             down_limit2;

/* Die folgenden Variablen werden in der Konfigurationsdatei festgelegt.     */
bool            timebase_angegeben = false;
short           timemode = 0;
timeunit_type   timeunit = SECONDS;

/*****************************************************************************/
/*                                                                           */
/*                 Forward-Deklarationen interner Funktionen                 */
/*                                                                           */
/*****************************************************************************/

static bool gut_beurteiltes_team ( void );

static bool schlecht_beurteiltes_team ( void );

static bool zielorientiert ( void );

/*****************************************************************************/
/*                                                                           */
/*                  Hilfsfunktionen fuer DefineNextCycleTime                 */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  gut_beurteiltes_team                                     */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  true, falls eine der unten geannanten Bedingungen er-    */
/*                        fuellt ist.                                        */
/*                  false sonst                                              */
/*                                                                           */
/*  Beschreibung :  Folgende 2 Krieterien werden getestet :                  */
/*                  a) Liegt der Durchschnittswert der normierten absoluten  */
/*                     Beurteilungen der Experten, die bereits im letzten    */
/*                     Team waren, oberhalb von upper_limit1.                */
/*                  b) Liegt der Durchschnittswert der Beurteilungen der neu */
/*                     ins Team gekommenen Experten ueber upper_limit2.      */
/*                  Betrachtet werden diese Werte natuerlich nur, wenn ent-  */
/*                  weder CountOldExp oder CountNewExp ungleich Null sind.   */
/*                                                                           */
/*                                                                           */
/*  Globale Var. :  SumOldExp                                                */
/*                  CountOldExp                                              */
/*                  SumNewExp                                                */
/*                  CountNewExp                                              */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

static bool gut_beurteiltes_team ( void )
{
  if ( CountOldExp )
  {
    if ( SumOldExp / CountOldExp > upper_limit1 ) 
    {
      return true;
    }
  }

  if ( CountNewExp )
  {
    if ( SumNewExp / CountNewExp > upper_limit2 )
    {
      return true;
    }
  }
  return false;
} /* Ende von gut_beurteiltes_team */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  schlecht_beurteiltes_team                                */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  true, falls eine der unten genannten Bedingungen er-     */
/*                        fuellt ist,                                        */
/*                  false sonst                                              */
/*                                                                           */
/*  Beschreibung :  Folgende 2 Krieterien werden getestet :                  */
/*                  a) Liegt der Durchschnittswert der absoluten Beurteil-   */
/*                     ungen der Experten, die bereits im letzten Team waren,*/
/*                     unterhalb von down_limit1.                            */
/*                  b) Liegt der Durchschnittswert der Beurteilungen der neu */
/*                     ins Team gekommenen Experten unter down_limit2.       */
/*                                                                           */
/*                                                                           */
/*  Globale Var. :  SumOldExp                                                */
/*                  CountOldExp                                              */
/*                  SumNewExp                                                */
/*                  CountNewExp                                              */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

static bool schlecht_beurteiltes_team ( void )
{
  if ( CountOldExp )
  {
    if ( SumOldExp / CountOldExp < down_limit1 ) 
    {
      return true;
    }
  }
  if ( CountNewExp )
  {
    if ( SumNewExp / CountNewExp < down_limit2 )
    {
      return true;
    }
  }
  
  return false;
} /* Ende von schlecht_beurteiltes_team */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  zielorientiert                                           */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  true, wenn ein Ziel-Experte im neuen Team ist.           */
/*                  false sonst                                              */
/*                                                                           */
/*  Beschreibung :  Alle Experten, die im naechsten Team sind, werden darauf-*/
/*                  hin untersucht, ob sie als CP-Funktion GOALMATCH oder    */
/*                  GOALSIM haben.                                           */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

static bool zielorientiert ( void )
{
  short i;

  for( i=0; i<TeamExpCount; i++ )
  {
    if( exp_ist_spezialist( ExpertTeamInfo[i].exp_nr ) )
    {
      continue;
    } /* Ende von if exp_ist_spezialist */

    if( ziel_experte( ExpertTeamInfo[i].exp_nr ) )
    {
      return true;
    } /* Ende von if ziel_experte */
  } /* Ende von for */

  return false;
} /* Ende von zielorientiert */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  DefineNextCycleTime                                      */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  Zyklusdauer des naechsten Zyklus                         */
/*                                                                           */
/*  Beschreibung :  Die Dauer der naechsten Arbeitsphase wird wie folgt be-  */
/*                  stimmt.                                                  */
/*                  Wurde in der Konfigurationsdate die Zeitbasis mit dem    */
/*                  Wachstumsfaktor explizit angegeben, so erfolgt die Be-   */
/*                  rechnung gem"a"s diesen Angaben; ansonsten wird wie folgt*/
/*                  vorgegangen:                                             */
/*                                                                           */
/*                  Es wird von einer bestimmten zeitlichen Basis ausgegangen*/
/*                  Dies ist entweder die in der Konfigurationsdatei angege- */
/*                  bene Zeitdauer oder der Default-Wert timebase.           */
/*                  Dann wird zunaechst proportional zurMaechtigkeit der     */
/*                  Regel- und Gleichungsmenge diese Basis erhoeht.          */
/*                  ( Pro tp_part kommen time_extra Sekunden hinzu ).        */
/*                  Als naechstes wird ueberprueft, ob Ziel-Experten einge-  */
/*                  setzt sind. Ist dies der Fall, so wird die Zeit mit      */
/*                  einem speziellen Faktor multipliziert, der dies be-      */
/*                  ruecksichtigt.                                           */
/*                  Dann wird die Zeit noch in Abhaengigkeit von der Guete   */
/*                  des neuen Teams gesetzt, d.h. handelt es sich um ein gut */
/*                  beurteiltes Team wird die Zykluszeit mit dem Faktor      */
/*                  good_team_t_fac multipliziert; ist es ein schlech-       */
/*                  tes Team, so wird mit bad_team_t_fac multipliziert.      */
/*                                                                           */
/*  Globale Var. :                                                           */
/*                                                                           */
/*  Externe Var. :                                                           */
/*                                                                           */
/*****************************************************************************/

long DefineNextCycleTime ( void )
{
  double zyklusdauer;

  /********************************************************************/
  /* Sonderbehandlung, falls TIMEBASE im Konfigurationsfile angegeben */
  /********************************************************************/
  if( timebase_angegeben )
  {
    switch ( timemode )
    {
      case SCONSTANT:    return timebase;
      case SLINEAR:      return (CycleCount+1) * timebase;
      case SEXPONENTIAL: if( CycleCount == 0 )
			 {
			   return timebase;
			 }
			 else
			 {
			   return 2*CycleTime[CycleCount-1];
			 } 
      default:           return timebase;
    } /* Ende von switch */
  } /* Ende von if( timebase_angegeben ) */

  /*******************************/
  /* Berechnung durch den Leiter */
  /*******************************/
  zyklusdauer = (double)timebase + 
		(double)(akt_termpaare_anz_best() / tp_part) * time_extra;

  if ( PlanDocum )
  {
    fprintf( plan_doc, "\n\nFestlegung der naechsten Zykluszeit :\n");
    fprintf( plan_doc, "    Zeitbasis         : %d Sekunden\n", timebase );
    fprintf( plan_doc, "    Es liegen insgesamt %d Termpaare vor. Da pro %d Regeln %2f sec\n", 
	     akt_termpaare_anz_best(), tp_part, time_extra);
    fprintf( plan_doc, "    hinzukommen, ergibt sich eine Zyklusdauer von %d Sekunden ( gerundet )\n", (long)zyklusdauer );
    flush();
  }

  if ( zielorientiert() )
  {
    zyklusdauer *= goalfaktor;

    if ( PlanDocum )
    {
      fprintf( plan_doc, "\n    Es sind Ziel-Experten eingesetzt, daher wird die Zeitdauer\n");
      fprintf( plan_doc, "    mit %d multipliziert.\n", (int)goalfaktor);
      fprintf( plan_doc, "    Neue Zeit : %d\n", (long)zyklusdauer);
    } /* Ende if PlanDocum */
  } /* Ende if zielorientiert */
  
  if ( gut_beurteiltes_team() )
  {
    if ( PlanDocum )
    {
      fprintf( plan_doc,  "\n   Team wird als gut angesehen --> mehr Zeit ! Zyklusdauer : %d Sekunden\n",
	       (long)(zyklusdauer * good_team_t_fac) );
    }
    return max( 1, (long)(zyklusdauer * good_team_t_fac));
  }
  
  if ( schlecht_beurteiltes_team() )
  {
    if ( PlanDocum )
    {
      fprintf( plan_doc,  "\n   Team wird als schlecht angesehen --> weniger Zeit ! Zyklusdauer : %d Sekunden\n",
	       (long)(zyklusdauer * bad_team_t_fac) );
      
    }
    return max( 1, (long)(zyklusdauer * bad_team_t_fac) );
  }
  
  return max( 1, (long)zyklusdauer );
} /* Ende von DefineNextCycleTime */
