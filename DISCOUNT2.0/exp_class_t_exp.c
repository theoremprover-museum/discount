/*****************************************************************************/
/*                                                                           */
/*  Projekt      : TEAM-COMPLETION                                           */
/*                                                                           */
/*  Modul        : exp_class_t_exp                                           */
/*                                                                           */
/*  Author       : Martin Kronenburg, 1993                                   */
/*                                                                           */
/*  Beschreibung : In diesem Modul werden alle Funktionen zur Verfuegung ge- */
/*                 stellt, die fuer die Klassifizierung der Experten, die im */
/*                 letzten Zyklus eingesetz waren, benoetigt werden.         */
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
#include "exp_class_t_exp.h"
#include "exp_form_next_t.h"
#include "team.h"
#define __j(name) {#name , name},

/*****************************************************************************/
/*                                                                           */
/*                            Globale Variablen                              */
/*                                                                           */
/*****************************************************************************/

/* Die folgenden Variablen dienen zur Bestimmung der Siegergrenze            */
int           w_size_r;
int           w_size_e;
int           w_size_g;
int           w_size_cp;
int           w_size_cg;
int           w_new_r;
int           w_red_r;
int           w_del_r;
int           w_new_e;
int           w_red_e;
int           w_del_e;
int           w_new_g;
int           w_red_g;
int           w_new_cp;
int           w_del_cp;
int           w_new_cg;
int           w_redcount;

double        increase_percent;

/* Die folgenden Variablen dienen zur Bestimmung der Verlierergrenze         */
int           l_size_r;
int           l_size_e;
int           l_size_g;
int           l_size_cp;
int           l_size_cg;
int           l_new_r;
int           l_red_r;
int           l_del_r;
int           l_new_e;
int           l_red_e;
int           l_del_e;
int           l_new_g;
int           l_red_g;
int           l_new_cp;
int           l_del_cp;
int           l_new_cg;
int           l_redcount;

double        decrease_percent;

int           verlierermaxcycl;
double        verlierergrenze3;

int           teamnotchanged;

/*****************************************************************************/
/*                                                                           */
/*                 Forward-Deklarationen interner Funktionen                 */
/*                                                                           */
/*****************************************************************************/

static long siegergrenze_best( short exp_nr );

static long verlierergrenze_best ( short exp_nr );

static bool siegerkriterien_erfuellt ( short exp_nr );

static bool verlierekriterien_erfuellt ( short exp_nr );

/*****************************************************************************/
/*                                                                           */
/*                 Hilfsfunktionen fuer ClassifyTeamExperts                  */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  siegergrenze_best                                        */
/*                                                                           */
/*  Parameter    :  Nummer des Experten, dessen Siegergrenze bestimmt wird   */
/*                                                                           */
/*  Returnwert   :  aktuelle Siegergrenze des angegebenen Experten           */
/*                                                                           */
/*  Beschreibung :  Die Beurteilung eines Experten haengt im wesentlichen    */
/*                  von dem Gutachter und dessen Parameterwerten ab. Um      */
/*                  dies bei der Ueberpruefung, ob ein Experte zu den Siegern*/
/*                  zaehlt oder nicht, zu beruecksichtigen, wird fuer jeden  */
/*                  Experten individuell diese Grenze bestimmt, indem die    */
/*                  Bewertung des Gutachters mit vom Leiter fuer gut einge-  */
/*                  stuften Default-Werten simuliert wird.                   */
/*                  Fuer jede Menge von Termpaaren gibt es also eine "opti-  */
/*                  male" Anzahl in Form einer Konstanten.                   */
/*                  Multipliziert wird die Summe dann noch mit der Zyklus-   */
/*                  zeit, die ja auch entscheidend fuer die Anzahl der ver-  */
/*                  schiedenen erzeugten Termpaaren ist.                     */
/*                                                                           */
/*  Globale Var. :  ExpertGlobalInfo                                         */
/*                                                                           */
/*  Externe Var. :  CycleCount                                               */
/*                  CycleTime                                                */
/*                                                                           */
/*****************************************************************************/

static long siegergrenze_best ( short exp_nr )
{
  switch ( RefGlobalInfo[exp_nr].beurteile_ga )
  {
    case SR_STATISTIC : 
	  return ( ExpertGlobalInfo[exp_nr].korrektur_fac[CycleCount] *
		   ( w_size_r   * RefGlobalInfo[exp_nr].beurt_paramliste[REF_SIZE_R].normal
		   + w_size_e   * RefGlobalInfo[exp_nr].beurt_paramliste[REF_SIZE_E].normal
		   + w_size_cp  * RefGlobalInfo[exp_nr].beurt_paramliste[REF_SIZE_CP].normal
		   + w_redcount * RefGlobalInfo[exp_nr].beurt_paramliste[REF_REDCOUNT].normal ) );
    case SR_EXTENDED  :
	  return ( ExpertGlobalInfo[exp_nr].korrektur_fac[CycleCount] *
		   ( w_size_r   * RefGlobalInfo[exp_nr].beurt_paramliste[REF_SIZE_R].normal
		   + w_size_e   * RefGlobalInfo[exp_nr].beurt_paramliste[REF_SIZE_E].normal
		   + w_size_cp  * RefGlobalInfo[exp_nr].beurt_paramliste[REF_SIZE_CP].normal
		   + w_new_r    * RefGlobalInfo[exp_nr].beurt_paramliste[REF_NEW_R].normal
		   + w_red_r    * RefGlobalInfo[exp_nr].beurt_paramliste[REF_RED_R].normal
		   + w_del_r    * RefGlobalInfo[exp_nr].beurt_paramliste[REF_DEL_R].normal
		   + w_new_e    * RefGlobalInfo[exp_nr].beurt_paramliste[REF_NEW_E].normal
		   + w_red_e    * RefGlobalInfo[exp_nr].beurt_paramliste[REF_RED_E].normal
		   + w_del_e    * RefGlobalInfo[exp_nr].beurt_paramliste[REF_DEL_E].normal
		   + w_new_cp   * RefGlobalInfo[exp_nr].beurt_paramliste[REF_NEW_CP].normal
		   + w_del_cp   * RefGlobalInfo[exp_nr].beurt_paramliste[REF_DEL_CP].normal
		   + w_redcount * RefGlobalInfo[exp_nr].beurt_paramliste[REF_REDCOUNT].normal ) );
	  
    case SR_FEELGOOD  : return 0;
    case SR_DIVERGENCE: return 0;
    case SR_NONE      : return 0;
  }

  return 0;
} /* Ende von siegergrenze_best */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  verlierergrenze_best                                     */
/*                                                                           */
/*  Parameter    :  Nummer des Experten, dessen Verlierergrenze bestimmt wird*/
/*                                                                           */
/*  Returnwert   :  aktuelle Verlierergrenze des angegebenen Experten        */
/*                                                                           */
/*  Beschreibung :  Dies ist das Analogon zu siegergrenze_best fuer die      */
/*                  Berechnung der Verlierergrenze.                          */
/*                                                                           */
/*  Globale Var. :  ExpertGlobalInfo                                         */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

static long verlierergrenze_best ( short exp_nr )
{
  switch ( RefGlobalInfo[exp_nr].beurteile_ga )
  {
    case SR_STATISTIC : 
            return ( ExpertGlobalInfo[exp_nr].korrektur_fac[CycleCount] *
		     ( l_size_r   * RefGlobalInfo[exp_nr].beurt_paramliste[REF_SIZE_R].normal
		     + l_size_e   * RefGlobalInfo[exp_nr].beurt_paramliste[REF_SIZE_E].normal
		     + l_size_cp  * RefGlobalInfo[exp_nr].beurt_paramliste[REF_SIZE_CP].normal
		     + l_redcount * RefGlobalInfo[exp_nr].beurt_paramliste[REF_REDCOUNT].normal ) );
    case SR_EXTENDED  :
	    return ( ExpertGlobalInfo[exp_nr].korrektur_fac[CycleCount] *
		     ( l_size_r   * RefGlobalInfo[exp_nr].beurt_paramliste[REF_SIZE_R].normal
		     + l_size_e   * RefGlobalInfo[exp_nr].beurt_paramliste[REF_SIZE_E].normal
		     + l_size_cp  * RefGlobalInfo[exp_nr].beurt_paramliste[REF_SIZE_CP].normal
		     + l_new_r    * RefGlobalInfo[exp_nr].beurt_paramliste[REF_NEW_R].normal
		     + l_red_r    * RefGlobalInfo[exp_nr].beurt_paramliste[REF_RED_R].normal
		     + l_del_r    * RefGlobalInfo[exp_nr].beurt_paramliste[REF_DEL_R].normal
		     + l_new_e    * RefGlobalInfo[exp_nr].beurt_paramliste[REF_NEW_E].normal
		     + l_red_e    * RefGlobalInfo[exp_nr].beurt_paramliste[REF_RED_E].normal
		     + l_del_e    * RefGlobalInfo[exp_nr].beurt_paramliste[REF_DEL_E].normal
		     + l_new_cp   * RefGlobalInfo[exp_nr].beurt_paramliste[REF_NEW_CP].normal
		     + l_del_cp   * RefGlobalInfo[exp_nr].beurt_paramliste[REF_DEL_CP].normal
		     + l_redcount * RefGlobalInfo[exp_nr].beurt_paramliste[REF_REDCOUNT].normal ) );
	  
    case SR_FEELGOOD  : return 0;
    case SR_DIVERGENCE: return 0;
    case SR_NONE      : return 0;
  }

  return 0;
} /* Ende von verlierergrenze_best */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  siegerkriterien_erfuellt                                 */
/*                                                                           */
/*  Parameter    :  Nummer des Experten, der getestet werden soll            */
/*                                                                           */
/*  Returnwert   :  true, wenn der Experte als Sieger eingestuft wird        */
/*                  false sonst                                              */
/*                                                                           */
/*  Beschreibung :  Es wird stets geguckt, ob der angegebene Experte bei sei-*/
/*                  nem letzten Einsatz die Siegerkriterien erfuellt hat.    */
/*                  Dabei wird vorausgesetzt, dass der Experte im letzten    */
/*                  Team zum Einsatz kam.                                    */
/*                  Ist dies nicht der Fall, ist das Ergebnis nicht vorher-  */
/*                  sagbar!!!!!                                              */
/*                  Es gibt 2 Kriterien, die getestet werden :               */
/*                  a) Die absolute Beurteilung liegt oberhalb einer be-     */
/*                     stimmten Grenze.                                      */
/*                  b) wenn er qualifizierte aufsteigende Tendenz zeigt, d.h.*/
/*                     er muss sich innerhalb der letzten beiden Zyklen ver  */
/*                     bessert haben und zwar derart, dass, wenn er sich im  */
/*                     naechsten Zyklus um mindestens increase_percent       */
/*                     Prozent dieser Verbesserung erneut verbessert, er die */
/*                     fuer ihn ermittelte siegergrenze ueberschreiten wuer- */
/*                     de.                                                   */
/*                     Damit koennen guten Experten vorzeitig erkannt        */
/*                     werden.                                               */
/*                                                                           */
/*  Globale Var. :  ExpertGlobalInfo                                         */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

static bool siegerkriterien_erfuellt ( short exp_nr )
{
  long    diff;     /* Differenz zwischen den beiden letzten Zyklen */
  double  norm_abs; /* auf den aktuellen Arbeitszyklus normierte abs. Bewertung */
		    /* des vorletzten Zyklus.                                   */
  
  if ( PlanDocum )
  {
    fprintf( plan_doc, "        Siegergrenze dieses Experten : %d\n",siegergrenze_best( exp_nr ) );
  }

  if ( ExpertGlobalInfo[exp_nr].abs_bewertung[CycleCount] > 
       ( ExpertGlobalInfo[exp_nr].siegergrenze = siegergrenze_best( exp_nr ) ) )
  {
    return true;
  }
  
  /***************************************************************************/
  /* Es wird ueberprueft, ob der Experte mindestens zweimal eingesetzt wurde */
  /***************************************************************************/
  if ( ExpertGlobalInfo[exp_nr].anz_einsaetze > 1 )
  {
    /************************************************************/
    /* Es wird getestet, ob er im Zyklus davor eingesetzt wurde */
    /************************************************************/
    if ( ExpertGlobalInfo[exp_nr].einsaetze[ExpertGlobalInfo[exp_nr].anz_einsaetze-2] ==
	 ExpertGlobalInfo[exp_nr].einsaetze[ExpertGlobalInfo[exp_nr].anz_einsaetze-1] - 1)
    {
      /*********************************************************************/
      /* Normieren der absoluten Beurteilung des vorletzten Zyklus auf den */
      /* aktuellen Zyklus.                                                 */
      /*********************************************************************/
      norm_abs = ExpertGlobalInfo[exp_nr].korrektur_fac[CycleCount]/
		 ExpertGlobalInfo[exp_nr].korrektur_fac[CycleCount-1] * 
		 ExpertGlobalInfo[exp_nr].abs_bewertung[CycleCount-1];
				  
      diff = ExpertGlobalInfo[exp_nr].abs_bewertung[CycleCount] - (long)norm_abs;

      if ( PlanDocum )
      {
        fprintf( plan_doc, "        Normierte Differenz der letzten beiden Einsaetze : %ld\n", diff );
	if( diff > 0)
	{
	  fprintf( plan_doc, "        Voraussichtliche naechste Beurteilung  : %ld\n", 
			     (long)(diff * increase_percent + 
			     ExpertGlobalInfo[exp_nr].abs_bewertung[CycleCount] ));
	  if ( (diff * increase_percent + ExpertGlobalInfo[exp_nr].abs_bewertung[CycleCount] ) >
	       ExpertGlobalInfo[exp_nr].siegergrenze )
	  {
	    fprintf( plan_doc, "        Dieser Wert liegt oberhalb der Siegergrenze %d\n", 
		     ExpertGlobalInfo[exp_nr].siegergrenze );
	    fprintf( plan_doc, "        --> Experte erfuellt die Siegerkriterien.\n");
	  }
	  else
	  {
	    fprintf( plan_doc, "        Dieser Wert liegt unterhalb der Siegergrenze %d\n", 
		     ExpertGlobalInfo[exp_nr].siegergrenze );
	    fprintf( plan_doc, "        --> Experte erfuellt die Siegerkriterien nicht.\n");
	  }
	} /* Ende von if ( diff > 0 ) */
	else
	{
	  fprintf( plan_doc, "        Experte hat sich verschlechtert in den beiden letzten\n");
	  fprintf( plan_doc, "        Zyklen verschlechtert. Es wird daher keine voraussichtliche\n");
	  fprintf( plan_doc, "        naechste Beurteilung berechnet, um die Siegerkriterien\n");
	  fprintf( plan_doc, "        zu ueberpruefen.\n");
	} /* Ende von else */
      } /* Ende von if PlanDocum */

      if ( diff > 0 )
      {
	if ( (diff * increase_percent + ExpertGlobalInfo[exp_nr].abs_bewertung[CycleCount] ) >
	     ExpertGlobalInfo[exp_nr].siegergrenze )
	{
	  return true;
	}
      } /* Ende von if ( diff > 0 ) */
    }
  } /* Ende von if zweimal im Einsatz */

  return false;
}/* Ende von siegerkriterien_erfuellt */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  verlierekriterien_erfuellt                               */
/*                                                                           */
/*  Parameter    :  Nummer des Experten, der getestet werden soll            */
/*                                                                           */
/*  Returnwert   :  true, falls der angegebene Experte die Verliererkriterien*/
/*                        erfuellt         				     */
/*                  false sonst                                              */
/*                                                                           */
/*  Beschreibung :  Es wird stets geguckt, ob der angegebene Experte bei sei-*/
/*                  nem letzten Einsatz die Verliererkriterien erfuellt hat. */
/*                  Dabei wird vorausgesetzt, dass der Experte im letzten    */
/*                  Team zum Einsatz kam.                                    */
/*                  Ist dies nicht der Fall, ist das Ergebnis nicht vorher-  */
/*                  sagbar!!!!!                                              */
/*                  Es gibt 3 Kriterien, die getestet werden :               */
/*                  a) Die absolute Beurteilung liegt unterhalb einer be-    */
/*                     stimmten Grenze.                                      */
/*                  b) wenn er qualifizierte absteigende Tendenz zeigt, d.h. */
/*                     er muss sich in den beiden letzten Zyklen verschlech- */
/*                     tert haben und zwar derart, dass, wenn er sich im     */
/*                     naechsten Zyklus erneut um mindestens decrease_percent*/
/*                     Prozent dieser Verschlechterung verschlechtert, er    */
/*                     unter die fuer ihn ermittelte Verlierergrenze fallen  */
/*                     wuerde.                                               */
/*                  c) wenn der Experte eine bestimmte Anzahl von Zyklen im  */
/*                     Team ist und seine relative Beurteilung innerhalb     */
/*                     dieser letzten Zyklen (es werden die durch die "be-   */
/*                     stimmte Anzahl" angegebenen letzten Zyklen betrachtet)*/
/*                     stets unter einer bestimmetn Schranke gelegen hat.    */
/*                                                                           */
/*  Globale Var. :  ExpertGlobalInfo                                         */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

static bool verlierekriterien_erfuellt ( short exp_nr )
{
  short i;

  long    diff;
  double  norm_abs; /* auf den aktuellen Arbeitszyklus normierte abs. Bewertung */
		    /* des vorletzten Zyklus.                                   */

  if ( PlanDocum )
  {
    fprintf( plan_doc,  "    Testen der Verliererkriterien : \n");
    fprintf( plan_doc,  "        Verlierergrenze dieses Experten : %d\n",verlierergrenze_best( exp_nr ) );
  }
  
  /************************************/
  /* erstes Kriterium wird getestet : */
  /************************************/
  if ( ExpertGlobalInfo[exp_nr].abs_bewertung[CycleCount] < 
       ( ExpertGlobalInfo[exp_nr].verlierergrenze = verlierergrenze_best( exp_nr ) ) )
  {
    return true;
  }
  
  /*************************************/
  /* zweites Kriterium wird getestet : */
  /***************************************************************************/
  /* Es wird ueberprueft, ob der Experte mindestens zweimal eingesetzt wurde */
  /***************************************************************************/
  if ( ExpertGlobalInfo[exp_nr].anz_einsaetze > 1 )
  {
    /***********************************************************/
    /* Es wird getestet, ob er im Zyklus davor engesetzt wurde */
    /***********************************************************/
    if ( ExpertGlobalInfo[exp_nr].einsaetze[ExpertGlobalInfo[exp_nr].anz_einsaetze-2] ==
	 ExpertGlobalInfo[exp_nr].einsaetze[ExpertGlobalInfo[exp_nr].anz_einsaetze-1] - 1)
    {
      /*********************************************************************/
      /* Normieren der absoluten Beurteilung des vorletzten Zyklus auf den */
      /* aktuellen Zyklus.                                                 */
      /*********************************************************************/
      norm_abs = ExpertGlobalInfo[exp_nr].korrektur_fac[CycleCount]/
                 ExpertGlobalInfo[exp_nr].korrektur_fac[CycleCount-1] *
                 ExpertGlobalInfo[exp_nr].abs_bewertung[CycleCount-1];

      diff = ExpertGlobalInfo[exp_nr].abs_bewertung[CycleCount] - (long)norm_abs;

      if ( PlanDocum )
      {
        fprintf( plan_doc,  "        Normierte Differenz der letzten beiden Einsaetze : %ld\n", diff );

	if ( diff < 0 )
        {
	  fprintf( plan_doc,  "        Voraussichtliche naechste Beurteilung  : %ld\n",
			      (long)(diff * decrease_percent + 
			      ExpertGlobalInfo[exp_nr].abs_bewertung[CycleCount]));
          if ( diff * decrease_percent + ExpertGlobalInfo[exp_nr].abs_bewertung[CycleCount] <
             ExpertGlobalInfo[exp_nr].verlierergrenze )
          {
            fprintf( plan_doc, "        Dieser Wert liegt unterhalb der Verlierergrenze %d\n",
                     ExpertGlobalInfo[exp_nr].verlierergrenze );
            fprintf( plan_doc, "        --> Experte erfuellt die Verliererkriterien.\n");
          }
          else
          {
            fprintf( plan_doc, "        Dieser Wert liegt oberhalb der Verlierergrenze %d\n",
                     ExpertGlobalInfo[exp_nr].verlierergrenze );
            fprintf( plan_doc, "        --> Experte erfuellt die Verliererkriterien nicht.\n");
          }
	}
	else
	{
	  fprintf( plan_doc,  "        Der Experte hat sich verbessert in den letzten beiden\n");
	  fprintf( plan_doc,  "        Zyklen. Es wird daher keine voraussichtliche naechste\n");
	  fprintf( plan_doc,  "        Beurteilung berechnet, um die Verliererkriterien zu testen.\n");
	} /* Ende von else */
      } /* Ende von if PlanDocum */

      if ( diff < 0 )
      {
	if ( diff * decrease_percent + ExpertGlobalInfo[exp_nr].abs_bewertung[CycleCount] <
	     ExpertGlobalInfo[exp_nr].verlierergrenze )
	{
	  return true;
	}
      } /* Ende von diff < 0 */
    } /* Ende von if --> Test, ob im vorletzten Zyklus eingesetzt */
  } /* Ende von if --> Test, ob mindestens zweimal bisher eingesetzt */
    
  /***********************************/
  /* drittes Kriterium wird getestet */
  /***********************************/
  if ( ExpertGlobalInfo[exp_nr].anz_einsaetze > verlierermaxcycl ) 
  {
    for ( i = ExpertGlobalInfo[exp_nr].anz_einsaetze-1;
	  i > ExpertGlobalInfo[exp_nr].anz_einsaetze-verlierermaxcycl-1;
	  i-- )
    {
      if ( ExpertGlobalInfo[exp_nr].rel_bewertung[ExpertGlobalInfo[exp_nr].einsaetze[i]] >
	   verlierergrenze3 )
      {
	  return false;
      }
    } /* Ende von for */
    
    if ( PlanDocum )
    {
      fprintf( plan_doc,  "        drittes Verliererkriterium erfuellt!\n");
    }

    return true;
  }
  return false;
}/* Ende von verlierekriterien_erfuellt */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  ClassifyTeamExperts                                      */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Die Experten, die im letzten Zyklus eingesetzt waren,    */
/*                  werden anhand ihrere Beurteilungen in drei Klassen einge-*/
/*                  teilt :        					     */
/*                  a) Sieger : Experten, die so gut beurteilt wurden , dass */
/*                              sie im naechsten Zyklus auf jeden Fall im    */
/*                              Team bleiben.                                */
/*                  b) Verlierer : Experten, deren Beurteilung so schlecht   */
/*                              war, dass sie potentielle Rauswurfkandidaten */
/*                              sind.                                        */
/*                  c) Experten, die zu keiner der obigen Klassen dazugehoe- */
/*                              ren; "Durchschnittstypen".                   */
/*  		    Ist die Menge der Verlierer leer & das Team bereits eine */
/*                  bestimmte Anzahl von Zyklen unveraendert im Einsatz & die*/
/*                  Menge der Durchschnittstypen nicht leer, so werden die   */
/*                  Experten aus der Menge der Durchschnittstypen in die     */
/*                  Verliererklasse uebertragen, damit ueberhaupt einmal     */
/*                  eine Veraenderung vorgenommen werden kann.               */
/*                                                                           */
/*                  In keine der Listen werden Spezialisten aufgenommen!!!   */
/*                                                                           */
/*                  Es erfolgt eine besondere Behandlung von Dom"anenstart-  */
/*                  experten, die nach der Dom"anenstartphase nicht mehr in  */
/*                  die Siegerliste aufgenommen werden:                      */
/*                  Ist dies der Fall, so wird, falls angegeben, sein        */
/*                  erstgenannter Nachfolger in das Team eingebaut, falls    */
/*                  noch Platz vorhanden ist.                                */
/*                                                                           */
/*  Globale Var. :  ExpertTeamInfo                                           */
/*                  TeamNotChanged             				     */
/*                  ExpWinnerList                                            */
/*                  ExpWinnerListLength                                      */
/*                  ExpLoserList                                             */
/*                  ExpLoserListLength                                       */
/*                  ExpAverageList					     */
/*		    ExpAverageListLength				     */
/*                  SumOldExp                                                */
/*                  CountOldExp                                              */
/*                  ExpertGlobalInfo                                         */
/*                  TeamExpCount                                             */
/*                                                                           */
/*  Externe Var. :  WinnerHost                                               */
/*                  CycleCount                                               */
/*                                                                           */
/*****************************************************************************/

void ClassifyTeamExperts ( void )
{
  short i;

  short akt_nr;   /* Nummer des Experten, der momentan betrachtet wird */
  short nachf_nr; /* Nummer eines Nachfolge-Experten */

  if ( PlanDocum )
  {
    fprintf( plan_doc,  "\n\nKlassifizierung der Experten, die im letzten Team waren :\n");
  }

  /**********************************************************/
  /* Behandlung der im letzten Zyklus eingesetzten Experten */
  /**********************************************************/
  /* Hier enthaelt TeamExpCount noch die Anzahl der im letzten Zyklus einge- */
  /* setzten Experten.                                                       */
  for ( i=0; i<TeamExpCount; i++ )
  { 
    akt_nr = ExpertTeamInfo[i].exp_nr;

    if ( PlanDocum )
    {
      fprintf( plan_doc,  "  Rechner %d : ", i );
      if ( exp_ist_spezialist( akt_nr ) )
      {
	fprintf( plan_doc,  "  Spezialist : %40s\n\n", get_exp_name( akt_nr ) );
      }
      else if ( StaticExp( akt_nr ) )
      {
	fprintf( plan_doc,  "  Statischer Experte : %20s\n\n", get_exp_name( akt_nr ) );
      }
      else if ( DomStartExp( akt_nr ) )
      {
	fprintf( plan_doc,  "  Domaenenstart-Experte : %20s\n\n", get_exp_name( akt_nr ) );
      }
      else
      {
	fprintf( plan_doc,  "  Experte    : %40s wird eingestuft :\n", get_exp_name( akt_nr ) );
	fprintf( plan_doc,  "    Bewertung seiner geleisteten Arbeit : %d.\n", 
		 ExpertGlobalInfo[akt_nr].abs_bewertung[CycleCount] );
      }
    } /* Ende von if PlanDocum */

    /*************************************************************************/
    /* Sonderbehandlung bei Spezialisten, Experten einer DomaenenStartPhase, */
    /* und statischen Experten.                                              */
    /*************************************************************************/
    if ( exp_ist_spezialist( akt_nr ) || StaticExp( akt_nr ) || DomStartExp( akt_nr ) )
    {
      continue;
    }

    if ( siegerkriterien_erfuellt( akt_nr ) )
    {
      if ( PlanDocum )
      {
	fprintf( plan_doc,  "        --> Siegerkriterien erfuellt!\n");
      }

      ExpWinnerList[ExpWinnerListLength++] = ExpertTeamInfo[i].exp_nr;
      ExpertGlobalInfo[ExpertTeamInfo[i].exp_nr].in_siegerliste = true;

    }
    else if ( verlierekriterien_erfuellt( akt_nr ) )
    {
      if ( PlanDocum )
      {
	fprintf( plan_doc,  "        --> Verliererkriterien erfuellt!\n");
      }

      ExpLoserList[ExpLoserListLength++] = akt_nr;
      ExpertGlobalInfo[akt_nr].in_siegerliste = false;
    }
    else
    {
      if ( PlanDocum )
      {
	fprintf( plan_doc,  "        --> Durchschnitts-Typ!\n");
      }

      ExpAverageList[ExpAverageListLength++] = ExpertTeamInfo[i].exp_nr;
      ExpertGlobalInfo[akt_nr].in_siegerliste = false;
    }

    /****************************************************/
    /* Nachfolger-Behandlung von Domaenenstart-Experten */
    /****************************************************/
    if( !ExpertGlobalInfo[akt_nr].in_siegerliste && 
	 ExpertGlobalInfo[akt_nr].dom_start_exp )
    {
      if ( PlanDocum )
      {
	fprintf( plan_doc, 
		 "Experte %s (Domaenenstart-Experte) faellt aus der Siegerliste\n", 
		 ExpertGlobalInfo[akt_nr].exp_name );
      }

      ExpertGlobalInfo[akt_nr].dom_start_exp = false;

      /*********************************************/
      /* Ueberpruefen, ob es einen Nachfolger gibt */
      /*********************************************/
      if( ExpertGlobalInfo[akt_nr].nachf_exp_anz > 0 )
      {
	/*********************************************/
	/* Uberpruefen, ob er dem System bekannt ist */
	/*********************************************/
	if( (nachf_nr = get_exp_nr( ExpertGlobalInfo[akt_nr].nachf_exp[0].name) ) 
	    != NOEXPERT )
	{
	  /**************************************************************************/
	  /* Uberpruefen, ob er bereits im neuen Team und ob noch Platz im Team ist */
	  /**************************************************************************/
	  if( !exp_im_neuen_team( nachf_nr ) )
	  {
	    if( ( leiter_faehiger_exp_in_neuem_team()  && !team_besetzt() ) ||
		( !leiter_faehiger_exp_in_neuem_team() && ( (freie_plaetze() > 1) ||
							    leiterfaehig( nachf_nr ) ) ) )
	    {
	      NewTeam[NewTeamLength++] = nachf_nr;	    

	      if( PlanDocum )
	      {
		fprintf( plan_doc, 
			 "Als Nachfolger wird Experte %s eingesetzt\n",
			 ExpertGlobalInfo[nachf_nr].exp_name );
	      } /* Ende von if PlanDocum */
	    }
	    else if( PlanDocum )
	    {
	      fprintf( plan_doc, "Kein Platz mehr fuer einen Nachfolger!\n");
	    } 
	  } /* Ende von if( !exp_im_neuen_team( nachf_nr ) ) */
	  else if( PlanDocum )
	  {
	    fprintf( plan_doc,"Angegebener Nachfolge-Experte %s ist bereits im Team!\n",
			      ExpertGlobalInfo[akt_nr].nachf_exp[0].name );
	  }
	}
	else if( PlanDocum )
	{
	  fprintf( plan_doc,"Angegebener Nachfolge-Experte %s ist unbekannt!\n",
		   ExpertGlobalInfo[akt_nr].nachf_exp[0].name );
	}

      } /* Ende von if nachf_exp_anz > 0 */
      else
      {
	if ( PlanDocum )
	{
	  fprintf( plan_doc,"Kein Nachfolger angegeben!\n");
	}
      }
    } /* Ende von if !in_siegerliste && dom_start_exp */
      

    if ( PlanDocum )
    {
      fprintf( plan_doc,  "\n" );
    }
  } /* Ende von for */

  if ( !ExpLoserListLength && ( TeamNotChanged > teamnotchanged ) &&
       ExpAverageListLength )
  {
    for ( i=0; i<ExpAverageListLength; i++ )
    {
      ExpLoserList[i] = ExpAverageList[i];
    }
    
    ExpLoserListLength = ExpAverageListLength;
    ExpAverageListLength = 0;
  }
} /* Ende von ClassifyTeamExperts */
