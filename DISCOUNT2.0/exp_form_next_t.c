/*****************************************************************************/
/*                                                                           */
/*  Projekt      : TEAM-COMPLETION                                           */
/*                                                                           */
/*  Modul        : exp_form_next_t                                           */
/*                                                                           */
/*  Author       : Martin Kronenburg, 1993                                   */
/*                                                                           */
/*  Beschreibung : In diesem Modul Wird das Team fuer den naechsten Zyklus   */
/*                 zusammengesetzt.                                          */
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
#include "exp_form_next_t.h"
#include "exp_prepare_t_select.h"
#include "team.h"
#define __j(name) {#name , name},

/*****************************************************************************/
/*                                                                           */
/*                            Globale Variablen                              */
/*                                                                           */
/*****************************************************************************/
               /************************************************/
               /* Variablen f"ur die Einsatzplanung des        */
               /* Database-Experten.                           */
               /************************************************/
int           host_limit_db;
int           cycle_diff_db;
int           dom_limit_db;

               /************************************************/
               /* Variablen f"ur die Einsatzplanung der        */
               /* Reduktionsspezialisten.                      */
               /************************************************/
int           host_limit_cp;
int           cycle_diff_cp;
int           crit_p_limit_cp;

int           host_limit_cg;
int           cycle_diff_cg;
int           crit_g_limit_cg;

/*****************************************************************************/
/*                                                                           */
/*                 Forward-Deklarationen interner Funktionen                 */
/*                                                                           */
/*****************************************************************************/

static bool exp_in_neuem_team ( short exp_nr );

static void fairen_exp_einbauen ( void );

static bool database_bed_erfuellt ( void );

static void databaseexperten_einbauen ( void );

static bool krit_paar_spez_bed_erfuellt ( short red_spec_nr );

static bool krit_ziel_spez_bed_erfuellt ( short red_spec_nr );

static short besten_exp_einbauen ( void );

/*****************************************************************************/
/*                                                                           */
/*                  Hilfsfunktionen fuer FormNextTeam                        */
/*                                                                           */
/*****************************************************************************/


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  exp_in_neuem_team                                        */
/*                                                                           */
/*  Parameter    :  Nummer des Experten, der getestet werden soll            */
/*                                                                           */
/*  Returnwert   :  true, falls der Experte bereits in dem neuen Team ist;   */
/*                  false sonst.                                             */
/*                                                                           */
/*  Beschreibung :  Die Liste NewTeam wird durchlaufen.                      */
/*                                                                           */
/*  Globale Var. :  NewTeamLength                                            */
/*                  NewTeam                                                  */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

static bool exp_in_neuem_team ( short exp_nr )
{
  short i;

  for ( i=0; i<NewTeamLength; i++ )
  {
    if ( NewTeam[i] == exp_nr )
    {
      return true;
    }
  }
  
  return false;
} /* Ende von exp_in_neuem_team */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  fairer_exp_in_neuem_team                                 */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  true, falls im bisher zusammaengesetzten neuen Team be-  */
/*                        reits ein fairer Expoerte vorkommt                 */
/*                  false sonst                                              */
/*                                                                           */
/*  Beschreibung :  Die Liste NewTeam wird durchlaufen.                      */
/*                                                                           */
/*  Globale Var. :  NewTeamLength                                            */
/*                  NewTeam                                                  */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

static bool fairer_exp_in_neuem_team ( void )
{
  short i;

  for ( i=0; i<NewTeamLength; i++ )
  {
    if ( ExpertGlobalInfo[NewTeam[i]].fair )
    {
      return true;
    }
  }
  
  return false;
} /* Ende von fairer_exp_in_neuem_team */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  leiter_faehiger_exp_in_neuem_team                        */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  true, falls im bisher zusammaengesetzten neuen Team be-  */
/*                        reits ein leiterfaehiger Experte vorkommt          */
/*                  false sonst                                              */
/*                                                                           */
/*  Beschreibung :  Die Liste NewTeam wird durchlaufen.                      */
/*                                                                           */
/*  Globale Var. :  NewTeamLength                                            */
/*                  NewTeam                                                  */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

bool leiter_faehiger_exp_in_neuem_team ( void )
{
  short i;

  for ( i=0; i<NewTeamLength; i++ )
  {
    /*****************************/
    /* Abfangen der Spezialisten */
    /*****************************/
    if( exp_ist_spezialist( NewTeam[i] ) )
    {
      continue;
    }

    if ( RefGlobalInfo[NewTeam[i]].beurt_paramliste[REF_NOMASTER].normal )
    {
      return true;
    }
  }
  
  return false;
} /* Ende von leiter_faehiger_exp_in_neuem_team */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  leiter_einbauen                                          */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Diese Funktion ueberprueft zunaechst, ob bereits ein     */
/*                  leiterfaehiger Experte im Team ist, wenn nicht, wird     */
/*                  einer eingebaut.                                         */
/*                  Dazu wird zuerst die Siegerliste untersucht und,  falls  */
/*                  da kein leiterfaehiger Experte war, die OrderedExpList.  */
/*                                                                           */
/*                  Dass in der Konfigurationsdatei ueberhaupt ein leiter-   */
/*                  faehiger Experte angegeben wurde, wird bereits beim Ein- */
/*                  lesen derselben abgeprueft, so dass die Fehlermeldung    */
/*                  am Ende nie eintreten duerfte.                           */
/*                  Es wird stets der am besten beurteilte leiterfaehige     */
/*                  Experte eingesetzt.                                      */
/*                                                                           */
/*  Globale Var. :  OrderedExpList                                           */
/*                  OrderedExpStart                                          */
/*                  NewTeam                                                  */
/*                  NewTeamLength                                            */
/*                  ExpertGlobalInfo                                         */
/*                                                                           */
/*  Externe Var. :  CycleCount                                               */
/*                                                                           */
/*****************************************************************************/

static void leiter_einbauen ( void )
{
  short lauf_ptr;
  short i;
  short j;
   
  if( leiter_faehiger_exp_in_neuem_team() )
  {
    return;
  }

  /*************************************/
  /* Zuerst Betrachten der Siegerliste */
  /*************************************/
  for( i=0; i<ExpWinnerListLength; i++ )
  {
    if( leiterfaehig( ExpWinnerList[i] ) )
    {
      NewTeam[NewTeamLength++] = ExpWinnerList[i];

      /* Verschieben der anderen Experten aus der Siegerliste nach vorne */
      ExpWinnerListLength--;
      for( j=i; j<ExpWinnerListLength; j++ )
      {
	 ExpWinnerList[j] =  ExpWinnerList[j+1];
      } /* Ende von for */
      return;
    }
  } /* Ende von for */

  /************************************************************************/
  /* Betrachten der anderen Experten, in der Reihenfolge ihrer Einstufung */
  /************************************************************************/
  lauf_ptr = OrderedExpStart;
  do
  {
    if ( leiterfaehig( OrderedExpList[lauf_ptr].exp_nr ) )
    { 
      NewTeam[NewTeamLength++] = OrderedExpList[lauf_ptr].exp_nr;
      exp_aus_bewertungsliste_loeschen( OrderedExpList[lauf_ptr].exp_nr );

      ExpertGlobalInfo[OrderedExpList[lauf_ptr].exp_nr].wartekonto = 0;

      return;
    }
    lauf_ptr = OrderedExpList[lauf_ptr].ptr;
  } while ( lauf_ptr != LISTENENDE );

  Error( __FILE__ ": " "leiter_einbauen", 
	 "Es konnte kein leiterfaehiger Experte ins Team eingebaut werden!" );
} /* Ende von leiter_einbauen */



/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  fairen_exp_einbauen                                      */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Es wird zuerst getestet, ob in dem bisher zusammenge-    */
/*                  setzten Team ein fairer Experte vorkommt; ist dies nicht */
/*                  der Fall, wird die OrderedExpList durchsucht, bis ein    */
/*		    fairer Experte gefunden wurde. Da im vorigen Team ein    */
/*		    fairer Experte war, muss in dieser Liste einer vorkommen;*/
/*		    es wird natuerlich der am besten beurteilte ausgewaehlt. */
/*                  Dieser neue Experte wird dann in die Liste OrderedExpList*/
/*                  hinzugefuegt ans Ende, es erfolgt keine irgendwie gearte-*/
/*                  te Einordnung ( ist auch nicht mehr erforderlich!!)      */
/*                  Es werden zusaetzlich die Variablen SumOldExp, SumNewExp,*/
/*                  CountOldExp und CountNewExp versorgt.                    */
/*                                                                           */
/*  Globale Var. :  OrderedExpList                                           */
/*                  OrderedExpStart                                          */
/*                  NewTeamLength                                            */
/*                  NewTeam                                                  */
/*                  ExpertGlobalInfo                                         */
/*                                                                           */
/*  Externe Var. :  CycleCount                                               */
/*                                                                           */
/*****************************************************************************/

static void fairen_exp_einbauen ( void )
{
  short lauf_ptr;
   
  if ( fairer_exp_in_neuem_team() )
  {
    return;
  }
  
  /******************************************/
  /* bisher ist kein fairer Experte im Team */
  /******************************************/
  lauf_ptr = OrderedExpStart;
  do
  {
    if ( ExpertGlobalInfo[OrderedExpList[lauf_ptr].exp_nr].fair )
    { 
      NewTeam[NewTeamLength++] = OrderedExpList[lauf_ptr].exp_nr;
      exp_aus_bewertungsliste_loeschen( OrderedExpList[lauf_ptr].exp_nr );

      return;
    }

    lauf_ptr = OrderedExpList[lauf_ptr].ptr;
  } while ( lauf_ptr != LISTENENDE );

  Error( __FILE__ ": " "fairen_exp_einbauen", 
	 "Es konnte kein fairer Experte ins Team eingebaut werden!" );
} /* Ende von fairen_exp_einbauen */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  speziellere_domaenen_bekannt                             */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  true, falls dem System speziellere Domaenen bekannt      */
/*                        sind, bezogen auf die bereits bekannten Domaenen;  */
/*                  false sonst.                                             */
/*                                                                           */
/*  Beschreibung :  Diese Funktion ist zur Zeit nur als Makro definert, das  */
/*                  stets false liefert.     				     */
/*                                                                           */
/*  Globale Var. :                                                           */
/*                                                                           */
/*  Externe Var. :                                                           */
/*                                                                           */
/*****************************************************************************/


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  database_bed_erfuellt                                    */ 
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  true, falls eine der unten genannten Kriterien fuer den  */
/*                        Einsatz eines Database-Experten erfuellt ist       */
/*                  false sonst.                                             */
/*                                                                           */
/*  Beschreibung :  In DefineKnownDomains wird der Zyklus festgesetzt, in    */
/*                  dem der database-Experten das n"achste mal fr"uhestens   */
/*                  eingesetzt wird.                                         */
/*                  In dieser Funktion wird getestet, ob uberhaupt           */
/*                  Dom"anen angegeben wurden, nach denen gesucht werden soll*/
/*                  Wenn nicht, so wird false zur"uckgeliefert.              */
/*                                                                           */
/*                  Anschlie"send wird nur noch getestet, ob der Zyklus      */
/*                  f"ur den n"achsten Einsatz des Database-Experten bereits */
/*                  erreicht oder sogar "uberschritten ist; wenn ja, so wird */
/*                  true zur"uckgeliefert.                                   */
/*                  Ansonsten wird fal;se zur"uckgeliefert.                  */
/*                                                                           */
/*  Globale Var. :                                                           */
/*                                                                           */
/*  Externe Var. :                                                           */
/*                                                                           */
/*****************************************************************************/

static bool database_bed_erfuellt ( void )
{
  if ( !SupposedDomainsCount )    /* Keine Domaenen vorhanden, nach denen ge-*/
  {                               /* sucht werden soll.                      */
    if ( PlanDocum )
    {
      fprintf( plan_doc,  "      Keine Domaenen vorhanden, nach denen gesucht werden soll!\n");
    }

    return false;
  }

  if( CycleCount >= Database.wartekonto )
  {
    if( PlanDocum )
    {
      fprintf( plan_doc,  "      Database-Experte wird eingesetzt.\n");
    }
    return true;
  }
  else
  {
    if( PlanDocum )
    {
      fprintf( plan_doc,  "      Database-Experte wird nicht eingesetzt, da\n");
      fprintf( plan_doc,  "      der naechste Einsatz fruehestens im %d. ten Zyklus erfolgt.\n", Database.wartekonto );
    }
  }

  return false;
} /* Ende von database_bed_erfuellt */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  databaseexperten_einbauen                                */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Es wird getestet, ob die Bedingungen fuer den Einsatz    */
/*                  eines Database-Experten gegeben sind; falls ja so wird   */
/*                  der Database-Experte eingesetzt.                         */
/*                  Es wird vorausgesetzt, dass noch Platz fuer einen Exper- */
/*                  ten im Team ist.                                         */
/*                                                                           */
/*  Globale Var. :  NewTeam                                                  */
/*                  NewTeamLength                                            */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

static void databaseexperten_einbauen ( void )
{
  if ( PlanDocum )
  {
    fprintf( plan_doc,  "  Entscheidungsphase fuer den Einsatz eines Databaseexperten :\n");
  }

  if ( Database.typ == SSTATIC ) 
  {
    if ( PlanDocum )
    {
      fprintf( plan_doc,  "Databaseexperte ist als STATIC deklariert.\n");
    }
    return;
  }

  if ( ( Database.typ == SINITIAL ) && !CycleCount )
  {
    if ( PlanDocum )
    {
      fprintf( plan_doc,  "Databaseexperte ist als INITIAL deklariert.\n");
    }
    return;
  }

  if ( !Database.spezialist )
  {
    if ( PlanDocum )
    {
      fprintf( plan_doc, "  Databaseexperte ist in Konfigurationsdatei nicht aufgefuehrt.\n");
    }
    return;
  }

  if ( database_bed_erfuellt() )
  {
    if ( PlanDocum )
    {
      fprintf( plan_doc,  "    Database-Experte wird eingesetzt\n");
    }
    NewTeam[NewTeamLength++] = DATABASE;
  }
  else
  {
    if ( PlanDocum )
    {
      fprintf( plan_doc,  "    Database-Experte wird nicht eingesetzt.\n");
    }
  }
} /* Ende von databaseexperten_einbauen */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  krit_paar_spez_bed_erfuellt                              */
/*                                                                           */
/*  Parameter    :  Nummer des Reduktionsspezialisten in der globalen Vari-  */
/*                  ablen RedSpecInfo                                        */
/*                                                                           */
/*  Returnwert   :  true, falls eine der Bedingungen fuer den Einsatz eines  */
/*                        Spezialisten fuer kritische Paare erfuellt ist;    */
/*                  false sonst                                              */
/*                                                                           */
/*  Beschreibung :  Die abgeprueften Bedingungen sind zur Zeit etwas spaer-  */
/*                  lich und sollten in Zukunft ausgefeilt werden!!!         */
/*                  Welche Bedinungen getestet werden , ist in der Beschrei- */
/*                  bung der Fuinktion reduktionsspezialisten_einbauen er-   */
/*                  kl"art.                                                  */
/*                                                                           */
/*  Globale Var. :                                                           */
/*                                                                           */
/*  Externe Var. :                                                           */
/*                                                                           */
/*****************************************************************************/

static bool krit_paar_spez_bed_erfuellt ( short red_spec_nr )
{
  short naechster_einsatz_cp;

  /*********************************************************/
  /* Festlegen des n"achsten Einsatzes dieses Spezialisten */
  /*********************************************************/
  naechster_einsatz_cp = cycle_diff_cp;

  if( HostCount < host_limit_cp )
  {
    naechster_einsatz_cp *= 2;
  }

  if( SizeOf(SetOfCriticalPairs) < crit_p_limit_cp * CycleCount )
  {
    naechster_einsatz_cp *= 2;
  }

  /*********************************************************************/
  /* Unterscheidung, ob Spezialist schon mal eingesetzt war oder nicht */
  /*********************************************************************/
  if ( !RedSpecInfo[red_spec_nr].anz_einsaetze )
  { /* Reduktionsspezialist wurde noch nie eingesetzt */
    if ( naechster_einsatz_cp <= CycleCount )
    {
      if( PlanDocum )
      {
	fprintf( plan_doc, " Reduktionsspezialist fuer krit. Paare wurde bisher noch nicht eingesetzt \n");
	fprintf( plan_doc, " --> Einsatz im naechsten Zyklus, da Einsatz ab Zyklus %d moeglich.\n",
		 naechster_einsatz_cp);
      }
      return true;
    }
  }
  else
  { /* Reduktionsspezialist war schon einmal eingesetzt wurden */
    if ( RedSpecInfo[red_spec_nr].einsaetze[RedSpecInfo[red_spec_nr].anz_einsaetze-1] +
	 naechster_einsatz_cp <= CycleCount )
    {
      if( PlanDocum )
      {
	fprintf( plan_doc, " Reduktionsspezialist fuer krit. Paare war bisher schon eingesetzt worden\n");
	fprintf( plan_doc, " --> Einsatz im naechsten Zyklus, da Einsatz ab Zyklus %d moeglich.\n",
		 naechster_einsatz_cp);
      }
      return true;
    }
  }

  if( PlanDocum )
  {
    fprintf( plan_doc, " Reduktionsspezialist fuer krit. Paare wird nicht eingesetzt\n");
    fprintf( plan_doc, " --> Einsatz ab Zyklus %d moeglich.\n", naechster_einsatz_cp);
  }
  return false;
} /* Ende von krit_paar_spez_bed_erfuellt */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  krit_ziel_spez_bed_erfuellt                              */
/*                                                                           */
/*  Parameter    :  Nummer des Reduktionsspezialisten in der globalen Vari-  */
/*                  ablen RedSpecInfo                                        */
/*                                                                           */
/*  Returnwert   :  true, falls eine der Bedingungen fuer den Einsatz eines  */
/*                        Spezialisten fuer kritische Ziele erfuellt ist;    */
/*                  false sonst                                              */
/*                                                                           */
/*  Beschreibung :  Die abgeprueften Bedingungen sind zur Zeit etwas spaer-  */
/*                  lich und sollten in Zukunft ausgefeilt werden!!!         */
/*                  Welche Bedinungen getestet werden , ist in der Beschrei- */
/*                  bung der Fuinktion reduktionsspezialisten_einbauen er-   */
/*                  kl"art.                                                  */
/*                                                                           */
/*  Globale Var. :                                                           */
/*                                                                           */
/*  Externe Var. :                                                           */
/*                                                                           */
/*****************************************************************************/

static bool krit_ziel_spez_bed_erfuellt ( short red_spec_nr )
{
  short naechster_einsatz_cg;

  /*********************************************************/
  /* Festlegen des n"achsten Einsatzes dieses Spezialisten */
  /*********************************************************/
  naechster_einsatz_cg = cycle_diff_cg;

  if( HostCount < host_limit_cg )
  {
    naechster_einsatz_cg *= 2;
  }

  if( SizeOf(SetOfCriticalGoals) < crit_g_limit_cg * CycleCount )
  {
    naechster_einsatz_cg *= 2;
  }

  /*********************************************************************/
  /* Unterscheidung, ob Spezialist schon mal eingesetzt war oder nicht */
  /*********************************************************************/
  if ( !RedSpecInfo[red_spec_nr].anz_einsaetze )
  { /* Reduktionsspezialist wurde noch nie eingesetzt */
    if ( naechster_einsatz_cg <= CycleCount )
    {
      if( PlanDocum )
      {
	fprintf( plan_doc, " Reduktionsspezialist fuer krit. Ziele wurde bisher noch nicht eingesetzt \n");
	fprintf( plan_doc, " --> Einsatz im naechsten Zyklus, da Einsatz ab Zyklus %d moeglich.\n",
		 naechster_einsatz_cg);
      }
      return true;
    }
  }
  else
  { /* Reduktionsspezialist war schon einmal eingesetz wurden */
    if ( RedSpecInfo[red_spec_nr].einsaetze[RedSpecInfo[red_spec_nr].anz_einsaetze-1] +
	 naechster_einsatz_cg <= CycleCount )
    {
      if( PlanDocum )
      {
	fprintf( plan_doc, " Reduktionsspezialist fuer krit. Ziele war bisher schon eingesetzt worden\n");
	fprintf( plan_doc, " --> Einsatz im naechsten Zyklus, da Einsatz ab Zyklus %d moeglich.\n",
		 naechster_einsatz_cg);
      }
      return true;
    }
  } /* Ende von else */

  if( PlanDocum )
  {
    fprintf( plan_doc, " Reduktionsspezialist fuer krit. Paare wird nicht eingesetzt\n");
    fprintf( plan_doc, " --> Einsatz ab Zyklus %d moeglich.\n", naechster_einsatz_cg);
  }
  return false;
} /* Ende von krit_ziel_spez_bed_erfuellt */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  reduktionsspezialisten_einbauen                          */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Es wird getestet, ob die Bedingungen fue den Einsatz     */
/*                  eines Reduktionsspezialisten Paare gegeben sind;         */
/*                  Maximal k"onnen MAXREDSPEC Reduktionsspezialisten        */
/*                  in der Konfigurationsdatei angegeben werden. Pro Zyklus  */
/*                  kann maximal einer zum Einsatz kommen.                   */
/*                  Es wird in der Reihenfolge, in der sie in der Konfigu-   */
/*                  rationsdatei angegeben wurden "uberpr"uft, ob sie zum    */
/*                  Einsatz kommen oder nicht.                               */
/*                  Die Kriterien sind die folgenden:                        */
/*                  Wurde nur eine Funktion zur Reduktion der kritischen     */
/*                  Paare beim Reduktionsspezialisten angegeben, so wird     */
/*                  getestet, ob zwischen dem letzten Einsatz bereits min-   */
/*                  destens cycle_diff_cp Zyklen liegen. Dieser Mindest-     */
/*                  abstand zwischen zwei Zyklen verdoppelt sich, wenn weni- */
/*                  ger als host_limit_cp Rechner zur Verf"ugung stehen.     */
/*                  Er verdoppelt sich ebenfalls, wenn weniger als           */
/*                  crit_p_limitcp * CycleCount kritische Paare zur Ver-     */
/*                  f"ugung stehen.                                          */
/*                  Dies "uberpr"uft die Funktion krit_paar_spez_bed_erfuellt*/
/*                                                                           */
/*                  Analog "uberpr"uft die Funktion                          */
/*                  krit_ziel_spez_bed_erfuellt diese Bedingungen f"ur       */
/*                  kritische Ziele, wenn beim Reduktionsspezialisten nur    */
/*                  eine Funktion zur Reduktion der kritischen  Ziele ange-  */
/*                  geben wurde. Ist keine Paramodulation angegeben, so      */
/*                  liefert diese Funktion stets false zur"uck.              */
/*                                                                           */
/*                  Wurde sowohl eine Funktion zur Reduktion der kritischen  */
/*                  Paare und der kritischen Ziele angegeben, so gen"ugt es, */
/*                  wenn eine der beiden Funktionen true zur"uckliefert.     */
/*                                                                           */
/*  Globale Var. :  NewTeam                                                  */
/*                  NewTeamLength                                            */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

static void reduktionsspezialisten_einbauen ( void )
{
  short i;

  if ( PlanDocum )
  {
    fprintf( plan_doc,  "\n  Entscheidungsphase fuer den Einsatz eines Spezialisten fuer krit. Paare :\n");
  }

  if ( !RedSpecCount )
  {
    if ( PlanDocum )
    {
      fprintf( plan_doc,  "\n  Es wurden keine Reduktionsspezialisten angegeben\n");
    }
    return;
  }

  /******************************************************************/
  /* "Uberpr"ufen, ob Reduktionsspezialist als STATIC oder INITIAL  */
  /* gekennzeichnet wurde.                                          */
  /* Dann wird nicht mehr weiter untersucht! ( bei INITIAL nur vor  */
  /* dem ersten Zyklus)                                             */
  /******************************************************************/
  for( i=0; i<RedSpecCount; i++ )
  {
    if( StaticRedSpec(i) || ( InitialRedSpec(i) && !CycleCount ) )
    {
      if( PlanDocum )
      {
	if( StaticRedSpec(i) )
	{
	  fprintf( plan_doc,  "  Reduktionsspezialist %s als STATIC definiert.\n", RedSpecInfo[i].name);
	}
	else
	{
	  fprintf( plan_doc,  "  Reduktionsspezialist %s als INITIAL definiert.\n", RedSpecInfo[i].name);
	}
      } /* Ende von if( PlanDocum ) */
      return;
    }
  } /* Ende von for */

  /*************************************************************************/
  /* Es wurden Reduktionsspezialisten in der Konfigurationsdatei angegeben */
  /* Diese werden der Reihe nach jetzt alle betrachtet.                    */
  /*************************************************************************/
  for( i=0; i<RedSpecCount; i++ )
  {
    switch( RedSpecInfo[i].red_typ )
    {
      case CP        :  if( krit_paar_spez_bed_erfuellt( i ) )
			{
			  if( PlanDocum )
			  {
			    fprintf( plan_doc, "  Reduktionsspezialist %s wird eingesetzt.\n",
				    RedSpecInfo[i].name );
			  }
			  NewTeam[NewTeamLength++] = RedSpecNo(i);
			  return;
			}
			break;
      case CG        :  if( krit_ziel_spez_bed_erfuellt( i ) )
			{
			  if( PlanDocum )
			  {
			    fprintf( plan_doc, "  Reduktionsspezialist %s wird eingesetzt.\n",
				    RedSpecInfo[i].name );
			  }
			  NewTeam[NewTeamLength++] = RedSpecNo(i);
			  return;
			}
			break;
      default        :  break; 
    } /* Ende von switch */
  } /* Ende von for */

} /* Ende von reduktionsspezialisten_einbauen */



/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  spezialisten_einbauen                                    */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Zur Zeit koennen 2 Arten von Spezialisten eingebaut wer- */
/*                  den : a) Database - Experte                              */
/*                        b) kritischer Paar Spezialist                      */
/*                  Es wird, falls noch Platz im Team ist, versucht, den     */
/*                  Database-Experten einzubauen, und falls dann immer noch  */
/*                  Paltz ist, einen Spezialisten zur Elimination kriti-     */
/*                  scher Paare.                                             */
/*                                                                           */
/*                  Es wird zuerst ueberprueft, ob in dem Team bereits ein   */
/*                  fairer Experte ist. Wenn nicht und es ist nur noch ein   */
/*                  freier Platz zu vergeben, so darf dieser freie Platz     */
/*                  nicht durch eine Spezialisten belegt werden!             */
/*                  Beachte : Wenn bisher kein fairer Experte im Team ist,   */
/*                            kann das Team noch nicht voll besetzt sein.    */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

void spezialisten_einbauen ( void )
{
  if ( PlanDocum )
  {
    fprintf( plan_doc,  "\n\nBetrachten der Spezialisten :\n");
  }
  flush();

  /* @@ faire Experten werden zur Zeit nicht betrachtet */
  if ( team_besetzt() || ( !leiter_faehiger_exp_in_neuem_team() && (freie_plaetze() < 2) ) )
  {
    if ( PlanDocum )
    {
      fprintf( plan_doc,  "\n  Kein Platz mehr fuer einen Spezilisten!\n");
    }
    flush();
    return;
  }
  
  databaseexperten_einbauen();

  /* @@ faire Experten werden zur Zeit nicht betrachtet */
  if ( team_besetzt() || ( !leiter_faehiger_exp_in_neuem_team() && (freie_plaetze() < 2) ) )
  {
    if ( PlanDocum )
    {
      fprintf( plan_doc,  "\n  Kein Platz mehr fuer einen Reduktionsspezialisten!\n");
    }
    flush();
    return;
  }
  
  reduktionsspezialisten_einbauen();

} /* Ende von spezialisten_einbauen */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  besten_exp_einbauen                                      */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  Nummer des Experten, der ins Team aufgenommen wurde,     */
/*                  falls es noch Experten gibt, ansonsten der Wert          */
/*                  LISTENENDE                                               */
/*                                                                           */
/*  Beschreibung :  Diese Prozedur belegt einen weitern Platz im neuen Team. */
/*                  Dafuer werden zuerst die Experten aus der Siegerliste    */
/*                  ins Team uebernommen und anschliessend wird die Liste    */
/*                  OrderedExpList durchgegangen; Es wird immer der erste aus*/
/*                  dieser Liste genommen, dies ist der jeweils am besten    */
/*		    beurteilte, der noch zu vergeben ist.                    */
/*                  BEACHTE: Die Siegerliste ist momentan nicht weiter sor-  */
/*                           tiert; aus ihr werden die Experten von hinten   */
/*                           einfach entnommen.                              */
/*                  BEACHTE: Es wird vorausgesetzt, dass noch Plaetze im Team*/
/*                           zu vergeben sind.                               */
/*                                                                           */
/*                                                                           */
/*  Globale Var. :  ExpWinnerListLength                                      */
/*                  ExpWinnerList                                            */
/*                                                                           */
/*  Externe Var. :  NewTeam                                                  */
/*                  NewTeamLength                                            */
/*                                                                           */
/*****************************************************************************/

static short besten_exp_einbauen ( void )
{
  short exp_nr;

  /*************************************/
  /* Zuerst Betrachten der Siegerliste */
  /*************************************/
  if( ExpWinnerListLength > 0 )
  {
    if( !exp_im_neuen_team( ExpWinnerList[ExpWinnerListLength-1] ) )
    {
      NewTeam[NewTeamLength++] = ExpWinnerList[--ExpWinnerListLength];
      return ExpWinnerList[ExpWinnerListLength];
    }
  }
  /*******************************************************/
  /* Naechster Experte wird aus OrderedExpList entnommen */
  /*******************************************************/
  else  
  {
    while( ( exp_nr = exp_aus_bewertungsliste_lesen() ) != LISTENENDE )
    {
      if( !exp_im_neuen_team( exp_nr ) )
      {
	NewTeam[NewTeamLength++] = exp_nr;
	return exp_nr;
      }
    } /* Ende von while */
  } /* Ende von else */

  return LISTENENDE;
} /* Ende von besten_exp_einbauen */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  FormNextTeam                                             */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Diese Komponente setzt aufgrund der Ergebnisse der ande- */
/*		    ren Komponenten das Team zusammen, wobei zur Zeit vor al-*/
/*  		    lem die Expertenplaetze belegt werden.                   */
/*                  Als erstes wird der Platz eines fairen Experte belegt, so*/
/*		    einer muss in jedem Zyklus vorkommen.                    */
/*		    Als naechstes wird der Einsatz von Spezialisten unter-   */
/* 		    sucht.            					     */
/*		    Zum Schluss werden die freien Plaetze im Team durch die  */
/*		    am besten beurteilten Experten aufgefuellt.              */
/*                  Nach dieser Prozedur stehen in der Variablen             */
/*                  NewTeam die Nummern der Experten, die im naechsten       */
/*                  Zyklus zum Einsatz kommen.                               */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

void FormNextTeam ( void )
{
  short i;

  /***********************************************************************/
  /* Garantieren, da"s mindestens ein leiterf"ahiger Experte im Team ist */
  /***********************************************************************/
  leiter_einbauen();

  /* @@ Das Einbauen eines fairen Experten wird momentan ausser Acht gelassen*/
  /*fairen_exp_einbauen();*/
  
  /*************************************/
  /* Auffuellen des Teams mit Experten */
  /*************************************/
  while ( !team_besetzt() && (besten_exp_einbauen() != LISTENENDE) )
  {
    /* Leerer while-Schleifenk"orper */
  } /* Ende von while */

  /*****************************************/
  /* Auffuellen des Teams mit Spezialisten */
  /*****************************************/
  /* Zuerst mit Reduktionsspezialisten     */
  /*****************************************/
  for( i=0; !team_besetzt() && ( i<RedSpecCount ); i++ )
  {
    if( !exp_im_neuen_team(RedSpecNo(i)) )
    {
      NewTeam[NewTeamLength++] = RedSpecNo(i);
    }
  } /* Ende von for */

  /*****************************************/
  /* Jetzt den Database-Experten           */
  /*****************************************/
  if( !team_besetzt() && Database.spezialist )
  {
    NewTeam[NewTeamLength++] = DATABASE;
  }

  if( !team_besetzt() )
  {
    /* Diese Fehlermeldung sollte nie erreicht werden k"onnen */
    Error( __FILE__ ": "  "besten_exp_einbauen", "Weniger Experten als Rechner!!!" );
  }


  /***************************************************************************/
  /* Aktualisieren des KZG ueber die Experten, die im naechsten Team sind    */
  /***************************************************************************/
  for( i=0; i<NewTeamLength; i++ )
  {
    /*****************************************/
    /* Sonderbehandlung bei den Spezialisten */
    /*****************************************/
    if( exp_ist_spezialist( NewTeam[i] ) )
    {
      switch ( NewTeam[i] )
      {
	case DATABASE  : Database.einsaetze[Database.anz_einsaetze++] = CycleCount + 1;
			 break;
	case REDUCE_1  : RedSpecInfo[0].einsaetze[RedSpecInfo[0].anz_einsaetze++] = 
			   CycleCount + 1;
			 break;
	case REDUCE_2  : RedSpecInfo[1].einsaetze[RedSpecInfo[1].anz_einsaetze++] = 
			   CycleCount + 1;
			 break;
	case REDUCE_3  : RedSpecInfo[2].einsaetze[RedSpecInfo[2].anz_einsaetze++] = 
			   CycleCount + 1;
			 break;
	default        : Error( __FILE__ ": "  "FormNextTeam", "Unbekannter Spezialist!!!" );
			 break;
      } /* Ende von switch */
    } /* Ende von if */
    else
    {
      /***************************/
      /* Eintragen des Einsatzes */
      /***************************/
      ExpertGlobalInfo[NewTeam[i]].einsaetze[ExpertGlobalInfo[NewTeam[i]].anz_einsaetze++] = CycleCount + 1;

      /*****************************************/
      /* Gesamtbeurteilung des Teams vornehmen */
      /*****************************************/
      /* dabei Aufteilen nach "alten" Experten, d.h. solchen, die bereits im letzten */
      /* Zyklus im Team waren, und solchen, die neu ins Team gekommen sind. Bei      */
      /* diesen wird auch noch das Wartegeloescht.                                   */
      if( exp_im_alten_team( NewTeam[i] ) )
      {
	SumOldExp += ExpertGlobalInfo[NewTeam[i]].norm_abs_bew[CycleCount-1];
	CountOldExp++;
      }
      else
      {
	SumNewExp += ExpertGlobalInfo[NewTeam[i]].akt_bewertung;
	CountNewExp++;
	ExpertGlobalInfo[NewTeam[i]].wartekonto = 0;
      } /* Ende von if/else */

    } /* Ende von else */
  } /* Ende von for */

} /* Ende von FormNextTeam */
