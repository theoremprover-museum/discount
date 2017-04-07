/*****************************************************************************/
/*                                                                           */
/*  Projekt      : TEAM-COMPLETION                                           */
/*                                                                           */
/*  Modul        : exp_value_exp                                             */
/*                                                                           */
/*  Author       : Martin Kronenburg, 1993                                   */
/*                                                                           */
/*  Beschreibung : In diesem Modul werden alle Funktionen zur Verfuegung ge- */
/*                 stellt, die fuer die Bewertung eines Experten benoetigt   */
/*                 werden.                                                   */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/param.h>
#include <math.h>
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
#include "exp_value_exp.h"
#include "team.h"
#define __j(name) {#name , name},

/*****************************************************************************/
/*                                                                           */
/*                       Lokale Typdefinitionen                              */
/*                                                                           */
/*****************************************************************************/
typedef enum { DOM_GEEIGNET, DOM_UNGEEIGNET, DOM_KEINE_ANGABEN } dom_bez_typ;

/*****************************************************************************/
/*                                                                           */
/*                            Globale Variablen                              */
/*                                                                           */
/*****************************************************************************/
double        good_dom_exp_kno;
double        no_dom_exp_kno;
double        bad_dom_exp_kno;

int           relevantlastcycl;
int           minimumofactions;
int           inspectedactions;

int           constantfactor;
int           trendexpfactor;

double        relkzgfactor;

double        maxdifference;

int           proof_mid;

/* aktuelle Beweisphaseneinstufung durch den Leiter : */
double        ProofPhase = 0.0;

/*****************************************************************************/
/*                                                                           */
/*                 Forward-Deklarationen interner Funktionen                 */
/*                                                                           */
/*****************************************************************************/

static double bew_phas_anf ( void );

static double bew_phas_mit ( void );

static double bew_phas_end ( void );

static dom_bez_typ exp_dom_bez_best ( short exp_nr, double *guete );

static double bew_wissensanteil_best ( short exp_nr );

static double dom_wissensanteil_best ( short exp_nr );

static bool exp_eingesetzt ( short exp_nr, short zyklen, short einsaetze );

static double norm_abs_mittelwert ( short exp_nr, short einsaetze );

static double rel_mittelwert ( short exp_nr, short einsaetze );

static bool exp_konstant_test( short exp_nr, short einsaetze, double mittel );

static double kzg_anteil_best ( short exp_nr );

/*****************************************************************************/
/*                                                                           */
/*                      Hilfsfunktionen fuer ValueExperts                    */
/*                                                                           */
/*****************************************************************************/
  /*************************************************************************/
  /*                                                                       */
  /*                   Funktionen zur Beweisphasen-Eignung                 */
  /*                                                                       */
  /*************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  DefineProofPhase                                         */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Diese Funktion berechnet die aktuelle Beweisphase.       */
/*                  Zur Zeit geschieht dies nur in Abh"angigkeit von der     */
/*                  Zykluszahl un der in der Planungsdatei angebbaren Gr"o"se*/
/*                  proof_mid.                                               */
/*                                                                           */
/*  Globale Var. :  ProofPhase                                               */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

void DefineProofPhase ( void )
{
  ProofPhase = 2 * atan( CycleCount/proof_mid ) / M_PI;
} /* Ende von DefineProofPhase */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  bew_phas_anf                                             */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  double-Wert                                              */
/*                                                                           */
/*  Beschreibung :  Diese Funktion berechnet den Faktor fuer die Anfangs-    */
/*                  phase der Beweisphasen in Abhaengigkeit von der Ein-     */
/*                  stufung der atuellen Beweisphase durch den Leiter.       */
/*                                                                           */
/*  Globale Var. :  ProofPhase                                               */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

static double bew_phas_anf ( void )
{
  if ( ProofPhase > 0.4 )
  {
    return 0;
  }
  
  return ( 2 - 5 * ProofPhase );
} /* Ende von bew_phas_anf */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  bew_phas_mit                                             */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  double-Wert                                              */
/*                                                                           */
/*  Beschreibung :  Diese Funktion berechnet den Faktor fuer die Mittel-     */
/*                  phase der Beweisphasen in Abhaengigkeit von der Ein-     */
/*                  stufung der atuellen Beweisphase durch den Leiter.       */
/*                                                                           */
/*  Globale Var. :  ProofPhase                                               */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

static double bew_phas_mit ( void )
{
  if ( ( ProofPhase < 0.2 ) || ( ProofPhase > 0.8 ) )
  {
    return 0;
  }
  
  if ( ProofPhase > 0.5 )
  {
    return ( (16.0 - 20.0 * ProofPhase)/3.0 );
  }
  
  return ( (20.0 * ProofPhase - 4.0)/3.0 );
} /* Ende von bew_phas_mit */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  bew_phas_end                                             */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  double-Wert                                              */
/*                                                                           */
/*  Beschreibung :  Diese Funktion berechnet den Faktor fuer die Ende-       */
/*                  phase der Beweisphasen in Abhaengigkeit von der Ein-     */
/*                  stufung der atuellen Beweisphase durch den Leiter.       */
/*                                                                           */
/*  Globale Var. :  ProofPhase                                               */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/
static double bew_phas_end ( void )
{
  if ( ProofPhase < 0.6 )
  {
    return 0;
  }

  return ( 5.0 * ProofPhase - 3);
} /* Ende von bew_phas_end */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  bew_wissensanteil_best                                   */
/*                                                                           */
/*  Parameter    :  Nummer des Experten, dessen beweisphasenspezifischer     */
/*                  Wissensanteil bestimmt werden soll                       */
/*                                                                           */
/*  Returnwert   :  beweisphasenspezifischer Wissensanteil des angegebenen   */
/*                  Experten                                                 */
/*                                                                           */
/*  Beschreibung :  F"ur jede der drei Phasen wird ein Wert bestimmt in Ab-  */
/*                  h"angigkeit von der im Expertenrahmen angegebenen Eig-   */
/*                  nungswerten f"ur diese Phasen. Diese drei Werte werden   */
/*                  dann aufaddiert.                                         */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  ExpertGlobalInfo                                         */
/*                  plan_doc                                                 */
/*                                                                           */
/*****************************************************************************/

static double bew_wissensanteil_best ( short exp_nr )
{
  double bew_wissensanteil;

  bew_wissensanteil = ExpertGlobalInfo[exp_nr].phase_anfang * bew_phas_anf() +
		      ExpertGlobalInfo[exp_nr].phase_mitte  * bew_phas_mit() +
		      ExpertGlobalInfo[exp_nr].phase_ende   * bew_phas_end();

  if ( PlanDocum )
  {
    fprintf( plan_doc,  "    Beweisphasenspezifischer Anteil             : %4f\n\n",
	     bew_wissensanteil );
  }

  return bew_wissensanteil;
} /* Ende von bew_wissensanteil_best */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  exp_dom_bez_best                                         */
/*                                                                           */
/*  Parameter    :  Nummer des Experten, der betrachtet werden soll          */
/*                  Pointer auf double                                       */
/*                                                                           */
/*  Returnwert   :  DOM_GEEIGNET, wenn der Experte bei einer der             */
/*                        relevanten Domaenen als geeignet eingestuft ist;   */
/*		    DOM_UNGEEIGNET, wenn der Experte bei einer der           */
/*      	          relevanten Domaenen als ungeeignet eingestuft ist; */
/*                  DOM_KEINE_ANGABEN, wenn der Experte nirgends aufge-      */
/*                        fuehrt ist.                                        */
/*                  In dem zweiten Parameter wird in den beiden ersten Fael- */
/*                  len die beste bzw. schlechteste angegebene Guete zu-     */
/*                  rueckgegeben.                                            */
/*                  Trifft Fall 1 und Fall 2 zu, so hat der erste Fall       */
/*                  Priorit"at.                                              */
/*                                                                           */
/*  Beschreibung :  Es wird zunaechst getestet, ob der Experte bei einer der */
/*                  fuer den naechsten Zyklus relevanten Domaenen als ge-    */
/*                  eignet eingestuft ist; 				     */
/*        	    nur wenn dies nicht der Fall ist, wird getestet, ob er   */
/*                  bei einer dieser Domaenen als ungeeignet eingestuft ist. */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  TeamDomCount                                             */
/*                  DomainTeamInfo         				     */
/*                                                                           */
/*****************************************************************************/

static dom_bez_typ exp_dom_bez_best ( short exp_nr, double *guete )
{
  short        i;
  double       lokal_guete;
  dom_bez_typ  dom_bez = DOM_KEINE_ANGABEN;

  *guete = 0;

  /************************************************************/
  /* Es wird getestet, ob Experte als geeignet eingestuft ist */
  /************************************************************/
  for ( i=0; i<TeamDomCount; i++ )
  {
    if ( exp_bei_dom_geeignet( exp_nr, DomainTeamInfo[i].dom_nr, &lokal_guete ))
    {
      dom_bez = DOM_GEEIGNET;
      if ( lokal_guete > *guete )
      {
	*guete = lokal_guete;
      }
    }
  } /* Ende von for */
  
  if ( dom_bez == DOM_GEEIGNET ) 
  {
    return DOM_GEEIGNET;
  }

  /**************************************************************/
  /* Es wird getestet, ob Experte als ungeeignet eingestuft ist */
  /**************************************************************/
  for ( i=0; i<TeamDomCount; i++ )
  {
    if ( exp_bei_dom_ungeeignet( exp_nr, DomainTeamInfo[i].dom_nr, &lokal_guete ))
    {
      dom_bez = DOM_UNGEEIGNET;
      if ( lokal_guete < *guete )
      {
	*guete = lokal_guete;
      }
    }
  } /* Ende von for */
  
  return dom_bez;
} /* Ende von exp_dom_bez_best */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  dom_wissensanteil_best                                   */
/*                                                                           */
/*  Parameter    :  Nummer des Experten, dessen dom"anenspezifischer Wissens-*/
/*                  anteil bestimmt werden soll                              */
/*                                                                           */
/*  Returnwert   :  dom"anenspezifischer Wissensanteil des angegebenen       */
/*                  Experten                                                 */
/*                                                                           */
/*  Beschreibung :  Der Domaenenspezifischer Anteil wird zunaechst in 2      */
/*                  Schritten berechnet. Es wird getestet, ob der Experte bei*/
/*                  einer der als fuer den naechsten Zyklus als relevant ge- */
/*                  kennzeichneten Domaenen als geeigneter bzw. ungeeigneter */
/*                  Experte aufgefuehrt wird. Je nachdem ob dies der Fall    */
/*                  ist, wird good_dom_exp_kno, bad_dom_exp_kno oder         */
/*                  no_dom_exp_kno zur"uckgeliefert. Im ersten Fall wird     */
/*                  dieser Wert mit dem bei dem Experten angegebenen Wissens-*/
/*                  anteil und der von der Funktion exp_dom_bez_best gegebe- */
/*                  nen G"ute multipliziert. Im zweiten Fall nur mit der     */
/*                  Guete.                                                   */
/*                                                                           */
/*  Globale Var. :  ExpertGlobalInfo                                         */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

static double dom_wissensanteil_best ( short exp_nr )
{
  double dom_wissensanteil = 0;
  double guete;

  switch(  exp_dom_bez_best( exp_nr, &guete ) )
  {
    case DOM_GEEIGNET:      dom_wissensanteil = ExpertGlobalInfo[exp_nr].wissen *
				         	good_dom_exp_kno * guete;
		            break;
       
    case DOM_UNGEEIGNET:    dom_wissensanteil = bad_dom_exp_kno * guete;
		            break;
     
    case DOM_KEINE_ANGABEN: dom_wissensanteil = no_dom_exp_kno;
		            break;

    default:                break;
  } /* Ende von switch */

  if ( PlanDocum )
  {
    fprintf( plan_doc,  "    Domaenenspezifischer Anteil                 : %4f\n\n",
	     dom_wissensanteil );
  }

  return dom_wissensanteil;
} /* Ende von dom_wissensanteil_best */

/*****************************************************************************/
/*                                                                           */
/*                   Hilfsfunktionen fuer kzg_anteil_best                    */
/*                                                                           */
/*****************************************************************************/


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  exp_eingesetzt                                           */
/*                                                                           */
/*  Parameter    :  Expertennummer                                           */
/*                  Anzahl der letzten Zyklen, die betrachtet werden sollen  */
/*                  Anzahl der Mindesteinsaetze, die er haben soll           */
/*                                                                           */
/*  Returnwert   :  true, falls der angegebene Experte in den durch den 2.   */
/*                        Parameter angegebenen letzten Zyklen wenigstens so */
/*                        oft im Team war, wie es der 3. Parameter angibt    */
/*                  false sonst    					     */
/*                                                                           */
/*  Beschreibung :  Es werden von hinten her die Zyklennummern betrachtet,   */
/*                  in denen der Experte eingesetzt war, solange dieser Ein- */
/*                  satz in den durch den 2. Parameter bestimmten Rahmen     */
/*                  faellt, wird der dritte Parameter dekrementiert.         */
/*                  Faellt ein Einsatz nicht mehr in diesen Rahmen, so er-   */
/*                  fuellt der Experte die geforderte Bedingung nicht, denn  */
/*                  sonst waere die for-Schleife vorher bereits abgebrochen  */
/*                  wurden; denn die Schleife bricht ab, entweder wenn ge-   */
/*                  nuegend viele Einsaetze gefunden wurden ( Parameter      */
/*                  einsaetze auf 0 gezaehlt ) oder der Experte hat keine    */
/*                  Einsaetze mehr, die betrachtet werden koennen. Im ersten */
/*                  Fall erfuellt der Experte die Bedingung, im 2. nicht.    */
/*                                                                           */
/*  Globale Var. :  ExpertGlobalInfo                                         */
/*                                                                           */
/*  Externe Var. :  CycleCount                                               */
/*                                                                           */
/*****************************************************************************/

static bool exp_eingesetzt ( short exp_nr, short zyklen, short einsaetze )
{
  short i;

  for ( i = ExpertGlobalInfo[exp_nr].anz_einsaetze; i && einsaetze; i-- )
  {
    if ( ExpertGlobalInfo[exp_nr].einsaetze[i-1] > CycleCount-zyklen )
    {
      einsaetze--;
    }
    else
    {
      return false;
    }
  } /* Ende von for */
  
  if ( einsaetze )
  {
    return false;
  }
  
  return true;
} /* Ende von exp_eingesetzt */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  norm_abs_mittelwert                                      */
/*                                                                           */
/*  Parameter    :  Expertennummer                                           */
/*                  Zahl der letzten Einsaetze, die betrachtet werden sollen */
/*                                                                           */
/*  Returnwert   :  berechneter Mittelwert                                   */
/*                                                                           */
/*  Beschreibung :  Es wird der Mittelwert der normierten absoluten Beurtei- */
/*                  lungen des angegebenen Experten berechnet, wobei nur die */
/*                  im 2. Parameter angegebenen letzten Einsaetze berueck-   */
/*                  sichtigt werden ( hat der Experte nicht soviele Ein-     */
/*                  traege, werden die beruecksichtigt, die er hat )         */
/*                                                                           */
/*  Globale Var. :  ExpertGlobalInfo                                         */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

static double norm_abs_mittelwert ( short exp_nr, short einsaetze )
{
  short i;
  long  summe = 0;
  short summanden = 0;

  for ( i = ExpertGlobalInfo[exp_nr].anz_einsaetze; 
	i && einsaetze; i--, einsaetze-- )
  {
    summe += ExpertGlobalInfo[exp_nr].norm_abs_bew[ExpertGlobalInfo[exp_nr].einsaetze[i-1]];
    summanden++;
  }

  return ( summanden ? (double)summe/(double)summanden : 0.0 );
} /* Ende von norm_abs_mittelwert */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  rel_mittelwert                                           */
/*                                                                           */
/*  Parameter    :  Expertennummer                                           */
/*                  Zahl der letzten Einsaetze, die betrachtet werden sollen */
/*                                                                           */
/*  Returnwert   :  berechneter Mittelwert                                   */
/*                                                                           */
/*  Beschreibung :  Es wird der Mittelwert der relativen Beurteilungen des   */
/*                  angegebenen Experten berechnet, wobei nur die im 2. Pa-  */
/*                  rameter angegebenen letzten Einsaetze beruecksichtigt    */
/*		    werden ( hat der Experte nicht soviele Eintraege, werden */
/*                  die beruecksichtigt, die er hat )                        */
/*                                                                           */
/*  Globale Var. :  ExpertGlobalInfo                                         */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

static double rel_mittelwert ( short exp_nr, short einsaetze )
{
  short i;
  double  summe = 0.0;
  short summanden = 0;

  for ( i = ExpertGlobalInfo[exp_nr].anz_einsaetze; 
	i && einsaetze; i--, einsaetze-- )
  {
    summe += ExpertGlobalInfo[exp_nr].rel_bewertung[ExpertGlobalInfo[exp_nr].einsaetze[i-1]];
    summanden++;
  }

  return ( summanden ? summe/(double)summanden : 0.0 );
} /* Ende von rel_mittelwert */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  exp_konstant_test                                        */
/*                                                                           */
/*  Parameter    :  Expertennummer                                           */
/*                  Zahl der letzten Einsaetze, die betrachtet werden sollen */
/*                  Mittelwert                                               */
/*                                                                           */
/*  Returnwert   :  true, falls die normierten absoluten Bewertungen des Ex- */
/*                        perten alle in einer e-Umgebung von MAXDIFFERECNE  */
/*                        um den angegebenen Mittelwert liegen. Betrachtet   */
/*                        werden nur die Bewertungen der durch den 2. Para-  */
/*                        meter angegebenen letzten Einsaetze.               */
/*                        D.h. die Leistungen des Experten werden als Kon-   */
/*                        stant betrachtet ueber diesen Zeitraum.            */
/*		    false sonst      					     */
/*                                                                           */
/*  Beschreibung :  Die normierten absoluten Beurteilungen werden mit dem    */
/*                  Mittelwert verglichen. Sobald eine Differenz groesser als*/
/* 		    MAXDIFFERECNE ist, wird false zurueckgeliefert.          */
/*                  Hat der Experte nicht soviele Eintraege, wie betrachtet  */
/*		    werden sollen, so werden nur die betrachtet, die er hat. */
/*                                                                           */
/*  Globale Var. :  ExpertGlobalInfo                                         */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

static bool exp_konstant_test( short exp_nr, short einsaetze, double mittel )
{
  short i;

  for ( i = ExpertGlobalInfo[exp_nr].anz_einsaetze;
	i && einsaetze; i--, einsaetze-- )
  {
    if ( abs(mittel - 
	    ExpertGlobalInfo[exp_nr].norm_abs_bew[ExpertGlobalInfo[exp_nr].einsaetze[i-1]])
	 > maxdifference )
    {
      return false;
    }
  } /* Ende von for */
  
  return true;
} /* Ende von exp_konstant_test */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  kzg_anteil_best                                          */
/*                                                                           */
/*  Parameter    :  Nummer des Experten, dessen KZG-Anteil bestimmt wird     */
/*                                                                           */
/*  Returnwert   :  KZG-Anteil des angegebenen Experten                      */
/*                                                                           */
/*  Beschreibung :  Der Wert des KZG-Anteil ergibt sich aus 2 Komponenten :  */
/*                  a) alle relativen Bewertungen                            */
/*                  b) Tendenz des Experten ueber eine bestimmte Anzahl von  */
/*                     Zyklen hinweg.                                        */
/*                  a) erhaelt man durch Mittelwertbildung aller bisherigen  */
/*                  relativen Bewertungen.                                   */
/*                  b) wird wie folgt berechnet :                            */
/*		    War der Experte in den letzten relevantlastcycl min-     */
/*    		    destens minimumofactions eingesetzt, so wird der Mittel- */
/*                  wert ueber die letzten inspectedactions absoluten Be-    */
/*                  wertungen gebildet.                                      */
/*                  Liegen alle diese Bewertungen in einer e-Umgebung von    */
/*                  maximal MAXDIFFERECNE um diesen Mittelwert, so wird der  */
/*                  Experte als konstant betrachtet; ansonsten wird der Un-  */
/* 		    terschied zwischen der letzten Bewertung und der         */
/*                  inspectedactions-ten letzten Bewertung als Tendenz in-   */
/*                  terpretiert, d.h. es wird also zwischen diesen beiden    */
/*                  Werten linear interpoliert.                              */
/*                  Ein negativer Wert bedeutet hierbei fallende ein posi-   */
/*                  tiver steigende Tendenz.                                 */
/*                  Die Werte von a) und b) werden dann noch mit einem kon-  */
/*                  stanten Faktor multipliziert, um die jeweilige Gewichtung*/
/*    		    zu verstaerken.                                          */
/*                  In der Funktion wird zuerst b) dann a) berechnet.        */
/*                                                                           */
/*  Globale Var. :  ExpertGlobalInfo                                         */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

static double kzg_anteil_best ( short exp_nr )
{
  double mittelwert;
  double kzg_wert = 0;
  short  last_einsatz;   /* Zyklusnummer des letzten Einsatz dieses Experten.*/
  short  ins_einsatz;    /* Zyklusnummer des inspectedactions-ten Einsatz vor*/
			 /* dem letzten Einsatz.                             */

  if ( exp_eingesetzt( exp_nr, relevantlastcycl, minimumofactions ) )
  {
    mittelwert = norm_abs_mittelwert( exp_nr, inspectedactions );
    if ( exp_konstant_test( exp_nr, inspectedactions, mittelwert ) )
    {
      kzg_wert = mittelwert * constantfactor;

      if ( PlanDocum )
      {
	fprintf( plan_doc,  "     Experte wird als konstant eingestuft !\n");
	fprintf( plan_doc,  "     --> Bewertung dafuer : %2f\n", kzg_wert );
      }
    }
    else
    {
      last_einsatz = ExpertGlobalInfo[exp_nr].einsaetze[ExpertGlobalInfo[exp_nr].anz_einsaetze-1];

      ins_einsatz = ExpertGlobalInfo[exp_nr].einsaetze[ExpertGlobalInfo[exp_nr].anz_einsaetze-inspectedactions-1];

      kzg_wert = ( ExpertGlobalInfo[exp_nr].norm_abs_bew[last_einsatz] -
		   ExpertGlobalInfo[exp_nr].norm_abs_bew[ins_einsatz]) * trendexpfactor;
      if ( PlanDocum )
      {
	fprintf( plan_doc,  "     Experte wird nicht als konstant eingestuft !\n");
	fprintf( plan_doc,  "     --> Bewertung fuer den Trend : %2f\n", kzg_wert );
      }
    } /* Ende von else --> nicht konstant */
  }

  kzg_wert += rel_mittelwert( exp_nr, ExpertGlobalInfo[exp_nr].anz_einsaetze ) * relkzgfactor;

  if ( PlanDocum )
  {
    fprintf( plan_doc,  "    Anteil des KZG                              : %4f", kzg_wert );
    fprintf( plan_doc,  " * %2f = %4f\n", ExpKZGFaktor, (double)ExpKZGFaktor*kzg_wert );
  } /* Ende von if */

  return kzg_wert;
} /* Ende von kzg_anteil_best */



/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  ValueExpert                                              */
/*                                                                           */
/*  Parameter    :  Nummer des Experten, der bewertet werden soll            */
/*                                                                           */
/*  Returnwert   :  Bewertung des angegebenen Experten                       */
/*                                                                           */
/*  Beschreibung :  Diese Funktion bewertet den angegebenen Experten.        */
/*                  Dabei werden aus drei Kriterien eine gewichtete Summe ge-*/
/*                  bildet : a) Robustheit          			     */
/*                           b) Wisensanteil     			     */
/*                           c) Kurzzeitgedaechtnis ( KZG )                  */
/*                  Spaeter kann auch noch ein viertes Kriterium             */
/*                           d) Teamfaehigkeit				     */
/*                  verwendet werden. Zur Zeit wird dieses ausser Acht ge-   */
/*                  lassen.   						     */
/*                                                                           */
/*  Globale Var. :  ExpertGlobalInfo                                         */
/*                                                                           */
/*  Externe Var. :  ExpDomFaktor                                             */
/*                  ExpRobFaktor                                             */
/*                  ExpKZGFaktor                                             */
/*                                                                           */
/*****************************************************************************/

double ValueExperts ( short exp_nr )
{
  double beurteilung;

  if ( PlanDocum )
  {

    fprintf( plan_doc,  "\n  Experte %20s wird beurteilt :\n", get_exp_name( exp_nr ) );
    beurteilung = ExpDomFaktor * dom_wissensanteil_best( exp_nr ) +
                  ExpBewFaktor * bew_wissensanteil_best( exp_nr ) +
                  ExpRobFaktor * robustheit_best( exp_nr ) +
		  ExpKZGFaktor * kzg_anteil_best( exp_nr );
    fprintf( plan_doc,  "    -------------------------------------------------------------------\n");
    fprintf( plan_doc,  "        Gesamtbeurteilung ( gewichtet!! )       : %6f\n", beurteilung );
    return beurteilung;
  }

  return ( ExpDomFaktor * dom_wissensanteil_best( exp_nr ) +
	   ExpBewFaktor * bew_wissensanteil_best( exp_nr ) +
	   ExpRobFaktor * robustheit_best( exp_nr ) +
	   ExpKZGFaktor * kzg_anteil_best( exp_nr ) );
} /* Ende von ValueExperts */
