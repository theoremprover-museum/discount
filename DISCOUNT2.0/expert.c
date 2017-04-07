/*****************************************************************************/
/*                                                                           */
/*  Projekt      : TEAM-COMPLETION                                           */
/*                                                                           */
/*  Modul        : expert                                                    */
/*                                                                           */
/*  Author       : Martin Kronenburg, 1993                                   */
/*                                                                           */
/*  Beschreibung : In diesem Modul werden alle globalen Variablen zur Ver-   */
/*                 fuegung gestellt, die fuer die Planungsphase der Experten */
/*                 benoetigt werden, alle elementaren Hilfsfunktionen hier-  */
/*                 fuer sowie die globale Steuerungsfunktion                 */ 
/*                 SelectNextTeamExperts.                                    */
/*                                                                           */ 
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>
#include "defines.h"
#include "error.h"
#include "domain.h"
#include "referee.h"
#include "database.h"
#include "exp_class_nont_exp.h"
#include "exp_class_t_exp.h"
#include "exp_def_break.h"
#include "exp_def_next_cycletime.h"
#include "exp_form_next_t.h"
#include "exp_handle_domain_phase.h"
#include "exp_handle_static_exp.h"
#include "exp_prepare_t_select.h"
#include "exp_value_exp.h"
#include "exp_set_next_config.h"
#include "team.h"
#include "parseexp.h"



#define __j(name) {#name , name},

/*****************************************************************************/
/*                                                                           */
/*               Variablen, die auch extern verwendet werden                 */
/*             hauptsaechlich von den anderen Experten-Modulen               */
/*                                                                           */
/*****************************************************************************/

/* Liste der Experten, die vom Leiter als Verlierer des letzten Zyklus er-   */
/* kannt wurden.                                                             */
/* Es werden nur die Indizes der Experten gemaess der globalen Variablen     */
/* ExpertGlobalInfo abgelegt, also die Nummer des Experten.                  */
short      ExpLoserList[MAXEXPERT];
short      ExpLoserListLength;

/* Liste der Experten, die vom Leiter weder als Sieger noch als Verlierer des*/
/* letzten Zyklus klassifiziert wurden.                                      */
/* Es werden nur die Indizes der Experten gemaess der globalen Variablen     */
/* ExpertGlobalInfo abgelegt, also die Nummer des Experten.                  */
short      ExpAverageList[MAXEXPERT];
short      ExpAverageListLength;

/* true, falls alle globalen Felder initialisiert wurden.                    */
bool       ExperteInitialized = false;

/* Die folgenden Variablen dienen zur Verwaltung der Liste von Experten, in  */
/* die die Experten, die nicht in der Siegerliste sind, gemaess ihrer Eignung*/
/* sortiert eingetragen werden.                                              */
exp_listen_elt  PraeOrderList[MAXEXPERT];
short           PraeOrderLength = 0;
short           PraeOrderStart = LISTENENDE;

/* Die folgenden Variablen dienen zur Verwaltung der Experten, die aus der   */
/* PraeOrderListe ausgewaehlt und beurteilt wurden. Aus ihnen waehlt der Lei-*/
/* ter letztendlich die Experten fuer sein Team aus.                         */
exp_listen_elt  OrderedExpList[MAXEXPERT];
short           OrderedExpStart = LISTENENDE;
short           OrderedExpLength = 0;

/* Die folgenden Variablen werden bei der Bestimmung der naechsten Zyklus-   */
/* dauer benoetigt.                                                          */
/* Summe der absoluten Beurteilungen aller Experten, die im letzten Team wa- */
/* und auch wieder im neuen Team sind.                                       */
long            SumOldExp;
/* Anzahl der Experten, die im letzten Team waren und auch im neuen Team sind*/
short           CountOldExp;

/* Summe der Bewertungen durch den Leiter aller Experten, die neu ins Team   */
/* kommen, d.h. im Zyklus davor nicht im Team waren.                         */
double          SumNewExp;
/* Anzahl der Experten, die neu ins Team kommen.                             */
short           CountNewExp;

/* Nur wenn der aktuelle Prozess Leiter ist, ist garantiert, dass die folgen-*/
/* Variablen sinnvoll belegt sind.                                           */

/* Infos ueber alle Experten                                                 */
ExpertFrame  ExpertGlobalInfo[MAXEXPERT];

/* Infos "uber alle Reduktionsspezialisten */
ReduceFrame      RedSpecInfo[MAXREDSPEC];

/* Variablen fuer die Spezialisten */
ExpertFrame    Database;

/* Indizes der Expeten, die im naechsten Zyklus ins Team kommen              */
short            NewTeam[MAXEXPERT];

/* Anzahl der Plaetze, die in NewTeam bereits belegt sind.                   */
short            NewTeamLength;

/* Konfigurationen der Experten, die im aktuellen Zyklus eingesetzt sind     */
/* Die Indizierung entspricht der Durchnummerierung der Rechner, d.h. der in */
/* ExpertTeamInfo an der Stelle i abgelegte Experte arbeitet auf dem Rechner,*/
/* der im System die Nummer i hat.                                           */
ExpertConfig      ExpertTeamInfo[MAXEXPERT];

/* Anzahl der Experten, die im laufenden Zykluis eingesetzt sind/waren       */
short        TeamExpCount = 0;

/* Liste der Experten, die vom Leiter als Sieger des letzten Zyklus erkannt  */
/* kannt wurden.                                                             */
/* Es werden nur die Indizes der Experten gemaess der globalen Variablen     */
/* ExpertGlobalInfo abgelegt, also die Nummer des Experten.                  */
short      ExpWinnerList[MAXEXPERT];
short      ExpWinnerListLength = 0;

/* Anzahl der Zyklen, in denen das Team von der Zusammensetzung der Experten */
/* her unveraendert ist.                                                     */
short        TeamNotChanged = 0;

/* Die folgende Variable gibt an, ob der Sieger des letzten Zyklus auch im   */
/* naechsten Zyklus vertreten ist. Wenn ja, so arbeitet er auf demselben     */
/* Rechner weiter und die krit. Paare und krit. Ziele muessen nicht neu      */
/* bewertet werden. Ist der Sieger auch im neuen Team, so hat die folgende   */
/* Variable den Wert false, sonst true.                                      */
bool          neuer_leiter_exp;

/*****************************************************************************/
/*                                                                           */
/*                 Makros in der Rolle von Funktionsaufrufen                 */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*                           Typdefinitionen                                 */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*                          lokale Variablen                                 */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*                 Forward-Deklarationen interner Funktionen                 */
/*                                                                           */
/*****************************************************************************/

static void  exp_winnerlist_aus( void );

static void  exp_loserlist_aus( void );

static void  exp_averagelist_aus( void );

/*****************************************************************************/
/*                                                                           */
/*                       allgemeine Hilfsfunktionen                          */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  initialize_exp                                           */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  notwendigen Initialisierungen im Bereich der Experten    */
/*                  und der Spezialisten.                                    */
/*                                                                           */
/*  Globale Var. :  ExpertGlobalInfo                                         */
/*                  ExperteInitialized                                       */
/*                  Database                                                 */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

void initialize_exp ( void )
{
  int i;

  /*******************************/
  /* Initialisieren der Experten */
  /*******************************/
  for ( i=0; i<MAXEXPERT; i++ )
  {
    ExpertGlobalInfo[i].anz_einsaetze = 0;
    ExpertGlobalInfo[i].eignung = 0;
    ExpertGlobalInfo[i].wartekonto = 0;
    ExpertGlobalInfo[i].beurteilt = false;
    ExpertGlobalInfo[i].in_siegerliste = false;
    ExpertGlobalInfo[i].dom_start_exp = false;
  }
  /***********************************/
  /* Initialisieren der Spezialisten */
  /***********************************/
  Database.spezialist = false;
  Database.anz_einsaetze = 0;

  for ( i=0; i<MAXREDSPEC; i++ )
  {
    RedSpecInfo[i].anz_einsaetze = 0;
    RedSpecInfo[i].red_typ = UNDEFINED;
  }
} /* Ende von initialize_exp */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  ziel_experte                                             */
/*                                                                           */
/*  Parameter    :  Expertennummer                                           */
/*                                                                           */
/*  Returnwert   :  true, wenn angegebener Experte sich vor allem aufs Ziel  */
/*                        konzentriert.                                      */
/*                  false sonst                                              */
/*                                                                           */
/*  Beschreibung :  Es wird ueberprueft, ob die Zugehoerige CP-Funktion des  */
/*                  Experte GOALMATCH oder GOALSIM ist.                      */
/*                                                                           */
/*  Globale Var. :  ExpertGlobalInfo                                         */
/*                                                                           */
/*  Externe Var. :                                                           */
/*                                                                           */
/*****************************************************************************/

bool ziel_experte ( short exp_nr )
{
  if( ( ExpertGlobalInfo[exp_nr].cpfkt_nr == SX_GOALMATCH ) ||
      ( ExpertGlobalInfo[exp_nr].cpfkt_nr == SX_GOALSIM   ) )
  {
    return true;
  } /* Ende von if */

  return false;
} /* Ende von ziel_experte */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  exp_im_alten_team                                        */
/*                                                                           */
/*  Parameter    :  Expertennummer                                           */
/*                                                                           */
/*  Returnwert   :  true, wenn angegebener Experte im Team war/ist           */
/*                  false sonst                                              */
/*                                                                           */
/*  Beschreibung :  Es wird getestet, ob der letzte Eintrag unter den Ein-   */
/*                  saetzen des Experten mit der aktuellen Zyklusnummer      */
/*                  uebereinstimmt; wenn ja, ist der Experte im Team.        */
/*                  Zuvor wird getestet, ob der Experte ueberhaupt schon mal */
/*                  zum Einsatz kam.                                         */
/*                                                                           */
/*  Globale Var. :  ExpertGlobalInfo                                         */
/*                                                                           */
/*  Externe Var. :  CycleCount                                               */
/*                                                                           */
/*****************************************************************************/

bool exp_im_alten_team ( short exp_nr )
{
  ReduceFrame *ptr;

  /*************************************/
  /* Sonderbehandlung bei Spezialisten */
  /*************************************/
  if ( exp_ist_spezialist( exp_nr ) )
  {
    switch ( exp_nr )
    {
      case DATABASE : if ( Database.anz_einsaetze )
			if ( Database.einsaetze[Database.anz_einsaetze-1] == CycleCount )
			  return true;
			else
			  return false;
		      else
			return false;

      case REDUCE_1 : ptr = &(RedSpecInfo[0]);    
		      break;
      case REDUCE_2:  ptr = &(RedSpecInfo[1]);    
		      break;
      case REDUCE_3 : ptr = &(RedSpecInfo[2]);    
		      break;
      default       : return false;
    } /* Ende von switch */

    /*************************************/
    /* Testen der Reduktionsspezialisten */
    /*************************************/
    if ( ptr->anz_einsaetze )
    {
      if ( ptr->einsaetze[ptr->anz_einsaetze-1] == CycleCount )
      {
	return true;
      } /* Ende des 2. if */
      else
      {
	return false;
      }
    } /* Ende  des 1. if */
    else
    {
      return false;
    }
  } /* Ende von if ( exp_ist_spezialist( exp_nr ) ) */

  /*********************/
  /* Experte liegt vor */
  /*********************/
  if ( ExpertGlobalInfo[exp_nr].anz_einsaetze )
  {
    if ( ExpertGlobalInfo[exp_nr].einsaetze[ExpertGlobalInfo[exp_nr].anz_einsaetze-1] == CycleCount )
    {
      return true;
    }
    else
    {
      return false;
    }
  } /* Ende des 1. if */
  else
  {
    return false;
  }
} /* Ende von exp_im_alten_team */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  exp_im_neuen_team                                        */
/*                                                                           */
/*  Parameter    :  Expertennummer                                           */
/*                                                                           */
/*  Returnwert   :  true, wenn angegebener Experte bereits im neuen Team     */
/*                        des naechsten Zyklus ist.                          */
/*                  false sonst                                              */
/*                                                                           */
/*  Beschreibung :  Die Variable NewTeam wird untersucht.                    */
/*                                                                           */
/*  Globale Var. :  NewTeam                                                  */
/*                  NewTeamLength                                            */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

bool exp_im_neuen_team ( short exp_nr )
{
  short i;

  for( i=0; i<NewTeamLength; i++ )
  {
    if( NewTeam[i] == exp_nr )
    {
      return true;
    }
  } /* Ende von for */
  
  return false;
} /* Ende von exp_im_neuen_team */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  get_exp_name                                             */
/*                                                                           */
/*  Parameter    :  Nummer eines Experten                                    */
/*                                                                           */
/*  Returnwert   :  Name des Experten                                        */
/*                                                                           */
/*  Beschreibung :  Bestimmt den Namen des angegebenen Experten              */
/*                  ACHTUNG : ES WIRD EIN POINTER AUF DEN ANFANGSBEREICH DES */
/*                            NAMENS ZURUECKGELIEFERT, NICHT EINE KOPIE DES  */
/*                            NAMENS.                                        */
/*                                                                           */
/*  Globale Var. :  ExpertGlobalInfo                                         */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

char *get_exp_name( short exp_nr )
{
  if ( exp_nr < ExpertCount )
  {
    return ExpertGlobalInfo[exp_nr].exp_name;
  }

  if ( exp_nr == DATABASE )
  {
    return "DATABASE";
  }
  
  if( exp_nr == REDUCE_1 || exp_nr == REDUCE_2 || exp_nr == REDUCE_3 )
  {
    return RedSpecInfo[GetRedSpecIdx(exp_nr)].name;
  }
  
  return "UNKNOWN_EXPERT";
} /* Ende von get_exp_name */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  DomStartExp                                              */
/*                                                                           */
/*  Parameter    :  Expertennummer                                           */
/*                                                                           */
/*  Returnwert   :  true, wenn der angegebene Experte ein Experte im Start-  */
/*                        team einer Domaene ist, die sich im naechsten Zy-  */
/*                        klus in der Startphase befindet.                   */
/*                                                                           */
/*  Beschreibung :  Es wird ueberprueft, ob dom_start_exp beim Experten ge-  */
/*		    setzt ist und ob er bei der relevanten Domaene des       */
/*                  naechsten Zyklus im Startteam ist und ob diese relevante */
/*                  Domaene sich noch in der Startphase befindet.            */
/*                                                                           */
/*  Globale Var. :                                                           */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

bool DomStartExp( short exp_nr )
{
  short i;

  if( !ExpertGlobalInfo[exp_nr].dom_start_exp )
  {
    return false;
  }

  if( !TeamDomCount )
  {
    return false; /* keien relevanten Domaenen in diesem Zyklus */
  }
  
  /* Ueberpruefen, ob sich die Domaene noch in der Startphase befindet */
  if( !DomainGlobalInfo[DomainTeamInfo[0].dom_nr].startphase )
  {
    return false;
  }
  
  /* Ueberpruefen, ob Experte im Startteam der Domaene aufgefuehrt ist */
  for( i=0; i<DomainGlobalInfo[DomainTeamInfo[0].dom_nr].start_team_groesse; i++)
  {
    if( DomainGlobalInfo[DomainTeamInfo[0].dom_nr].start_team_exp[i].nr == exp_nr )
    {
      return true;
    }
  } /* Ende von for */
  
  return false;

} /* Ende von DomStartExp */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  exp_bei_exp_geeignet                                     */
/*                                                                           */
/*  Parameter    :  Expertennummer                                           */
/*		    Expertennummer					     */
/*                  Pointer auf double           		  	     */
/*                                                                           */
/*  Returnwert   :  true, falls der erste Expert bei dem zweiten als geeig-  */
/*			  net aufgefuehrt ist;                               */
/*		    false sonst 					     */
/*                  Im positiven Fall wird in dem dritten Parameter die Guete*/
/*                  des Experten fuer diese Experten zurueckgegeben.         */
/*                                                                           */
/*  Beschreibung :  Die Liste der gutgeeigneten Experten beim zweiten Exper- */
/*		    ten wird durchsucht,d.h. die Liste der guten Teamkollegen*/
/*                  Es duerfen keine Spezialisten angegeben werden.          */
/*                                                                           */
/*  Globale Var. :  ExpertGlobalInfo                                         */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

bool exp_bei_exp_geeignet ( short exp_nr1, short exp_nr2, double *guete )
{
  int i;

  for ( i=0; i<ExpertGlobalInfo[exp_nr2].team_exp_anz; i++ )
  {
    if ( !strcmp( ExpertGlobalInfo[exp_nr2].team_exp[i].name,
		  ExpertGlobalInfo[exp_nr1].exp_name ) )
    {  
      *guete = ExpertGlobalInfo[exp_nr2].team_exp[i].guete;
      return true;
    }
  } /* Ende von for */
  
  return false;
} /* Ende von exp_bei_exp_geeignet */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  get_exp_nr                                               */
/*                                                                           */
/*  Parameter    :  Name eines Experten                                      */
/*                                                                           */
/*  Returnwert   :  Falls der Name als Expertenname identifiziert werden     */
/*                  konnte, wird die zugehoerige Nummer zurueckgegeben,      */
/*                  d.h. der Index in ExpertGlobalInfo,                      */
/*                  ansonsten wird der Fehlercode NOEXPERT zurueckgegeben.   */
/*                  Spezialisten werden nicht erkannt!                       */
/*                                                                           */
/*  Beschreibung :  Es wird die Tabelle ExpertGlobalInfo durchsucht.         */
/*                                                                           */
/*  Globale Var. :  ExpertGlobalInfo                                         */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

short get_exp_nr ( char *exp_name )
{
  short i;

  for ( i=0; i<ExpertCount; i++ )
  {
    if ( !strcmp( ExpertGlobalInfo[i].exp_name, exp_name ) )
    {
      return i;
    }
  } /* Ende von for */
  
  return NOEXPERT;
} /* Ende von get_exp_nr */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  prae_orderedliste_aus                                    */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Ausgabe der prae_orderedliste in das Planungsdokumenta-  */
/*                  tionsfile.                                               */
/*                                                                           */
/*  Globale Var. :  PraeOrderStart                                           */
/*                  PraeOrderLength                                          */
/*                  PraeOrderList                                            */
/*                  ExpertGlobalInfo                                         */
/*                  plan_doc                                                 */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/
void prae_orderedliste_aus( void )
{
  short i;
  short j;

  fprintf( plan_doc, "    Laenge der Eignungsliste: %d\n",PraeOrderLength);
  for ( j=0, i=PraeOrderStart; i!=LISTENENDE; j++, i=PraeOrderList[i].ptr)
  {
    fprintf( plan_doc,"     %2d-ter Experte in Eignungsliste : Nr.: %2d, Name : %20s, Eignung : %2f\n",
    j, PraeOrderList[i].exp_nr, get_exp_name(PraeOrderList[i].exp_nr), ExpertGlobalInfo[PraeOrderList[i].exp_nr].eignung);
  }
} /* Ende von prae_orderedliste_aus */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  orderedliste_aus                                         */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Ausgabe der orderedliste in das Planungsdokumenta-       */
/*                  tionsfile.                                               */
/*                                                                           */
/*  Globale Var. :  plan_doc                                                 */
/*                  OrderedExpLength                                         */
/*                  OrderedExpStart                                          */
/*                  OrderedExpList                                           */
/*                  ExpertGlobalInfo                                         */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/
void orderedliste_aus( void )
{
  short i;
  short j;

  fprintf( plan_doc,"    Laenge der Bewertungsliste : %d\n", OrderedExpLength );
  for ( j=0, i=OrderedExpStart;i!=LISTENENDE; j++, i=OrderedExpList[i].ptr)
  {
    fprintf( plan_doc,"    %2d-ter Experte in Bewertungsliste : Nr.: %2d, Name : %20s \n",
	   j, OrderedExpList[i].exp_nr, get_exp_name(OrderedExpList[i].exp_nr) );
    fprintf( plan_doc,"                                        Bewertung : %6f\n",
           ExpertGlobalInfo[OrderedExpList[i].exp_nr].akt_bewertung );
  }
  fprintf( plan_doc, "\n\n");
} /* Ende von orderedliste_aus */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  exp_global_debug                                         */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Dieses Modul gibt die Belegungen aller globalen Variablen*/
/*                  des Moduls expert.c aus. Fuer Testzwecke.                */
/*                                                                           */
/*                                                                           */
/*  Globale Var. :  alle, die es gibt ( fast alle )                          */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

void exp_global_debug ( void )
{
  short i;

  printf("Anzahl der Exp. in der Siegerliste : %d\n",ExpWinnerListLength);
  for (i=0;i<ExpWinnerListLength;i++)
  {
    printf("Experten in der Siegerliste : %d\n",ExpWinnerList[i] );
  } /* Ende von for */

  printf("\nAnzahl der Exp. in der Loserliste : %d\n",ExpLoserListLength);
  for (i=0;i<ExpLoserListLength;i++)
  {
    printf("Experten in der LoserListe : %d\n",ExpLoserList[i] );
  } /* Ende von for */
  
  printf("\nBewertung des besten Losers : %f\n",BestLoser);

  printf("\nAnzahl der Experten in der Durchschnittsliste : %d\n",ExpAverageListLength);
  for (i=0;i<ExpAverageListLength;i++)
  {
    printf("Experten in der LoserListe : %d\n",ExpAverageList[i] );
  } /* Ende von for */
  
  printf("\nBeweisphase : %f\n",ProofPhase);

  printf("\nMaximale Anzahl von Experten, die betrachtet werden : %d\n",MaxLookAtExp);
  printf("Maximale Anzahl von Experten, die besser sein muessen : %d\n",MaxBetterExp);

  printf("\nAnzahl der Experten, die bisher beurteilt wurden : %d\n",LookAtExp);
  printf("Anzahl der Experten, die bisher besser als bestloser beurteilt wurden : %d\n",BetterExp);

  printf("\nPraeOrderList-Laenge : %d\n",PraeOrderLength);
  for (i=PraeOrderStart;i!=LISTENENDE;i=PraeOrderList[i].ptr)
  {
    printf("%d-ter Experte in PraeOrderList : %d\n",i, PraeOrderList[i].exp_nr);
  } /* Ende von for */
  
  printf("\nOrderedExpList-Laenge : %d\n",OrderedExpLength);
  for (i=OrderedExpStart;i!=LISTENENDE;i=OrderedExpList[i].ptr)
  {
    printf("%d-ter Experte in OrderedExpList : %d\n",i, OrderedExpList[i].exp_nr);
  } /* Ende von for */
  
  printf("\nAnzahl der Experten, die im Team bleiben : %d\n",CountOldExp);
  printf("Gesamtsumme der Bewertungen dieser Experten : %d\n",SumOldExp);
  printf("Anzahl der neuen Team-Experten : %d\n",CountNewExp);
  printf("Gesamtsumme der Bewertungen dieser neuen Team-Experten : %f\n",SumNewExp);

  printf("\nAnzahl der Zyklen, in denen Team unveraendert : %d\n",TeamNotChanged);

}/* Ende von exp_global_debug */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  SelectNextTeamExperts                                    */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Diese Komponete steuert die gesamte Planung fuer den     */
/*                  naechsten Zyklus was die Experten anbetrifft.            */
/*                  Sie laeuft auf dem Rechner ab, der der Leiter des        */
/*                  naechsten Zyklus ist.                                    */
/*                                                                           */
/*  Globale Var. :  SumOldExp                                                */
/*                  CountOldExp                                              */
/*                  CountNewExp                                              */
/*                  SumNewExp                                                */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

void SelectNextTeamExperts ( void )
{
  /* Initialisierungsteil */
  SumOldExp = 0;
  CountOldExp = 0;

  CountNewExp = 0;
  SumNewExp = 0;

  NewTeamLength = ExpWinnerListLength = ExpLoserListLength = ExpAverageListLength = 0;

  initialisiere_eignungsliste();

  initialisiere_bewertungsliste();

  HandleStaticExperts();

  if( !team_besetzt() )
  {
    HandleDomainPhase();
  }

  if( !team_besetzt() && CycleCount ) /* Im ersten Zyklus gibt es nichts zu tun hier */
  {
    ClassifyTeamExperts();
  }

  if( !team_besetzt() )
  {
    spezialisten_einbauen();
  }

  if( !team_besetzt() )
  {
    ClassifyNonTeamExperts();

    DefineBreakOffs();

    PrepareTeamSelection();
  }

  FormNextTeam();

  TeamExpCount = NewTeamLength;

  SetNextConfiguration();

  /* Versorgung von relevanten Teamvariablen */

  if ( !CountNewExp )
  {
    TeamNotChanged++;
  }
  else
  {
    TeamNotChanged = 0;
  } /* Ende von if/else */

}/*Ende von SelectNextTeamExperts */








static void  exp_winnerlist_aus( void )
{
  short i;
  printf("\nExpWinnerListLength : %d\n",ExpWinnerListLength);

  for(i=0;i<ExpWinnerListLength;i++)
  {
    printf("Experte in ExpWinnerList : Nr.: %d, Name : %s\n",ExpWinnerList[i],get_exp_name(ExpWinnerList[i]) );
  }
}
static void  exp_loserlist_aus( void )
{
  short i;
  printf("\nExpLoserListLength : %d\n",ExpLoserListLength);

  for(i=0;i<ExpLoserListLength;i++)
  {
    printf("Experte in ExpLoserList : Nr.: %d, Name : %s\n",ExpLoserList[i],get_exp_name(ExpLoserList[i]) );
  }
}
static void  exp_averagelist_aus( void )
{
  short i;
  printf("\nExpAverageListLength : %d\n",ExpAverageListLength);

  for(i=0;i<ExpAverageListLength;i++)
  {
    printf("Experte in ExpAverageList : Nr.: %d, Name : %s\n",ExpAverageList[i],get_exp_name(ExpAverageList[i]) );
  }
}

  /* Fuer debuggerzwecke */
  /*printf("Nach FormNextTeam\n");
  printf("CountOldExp : %d          CountNewExp : %d     \n",CountOldExp,CountNewExp);
  exp_winnerlist_aus();
  exp_loserlist_aus( );
  exp_averagelist_aus( );
  prae_orderedliste_aus();
  orderedliste_aus();*/
