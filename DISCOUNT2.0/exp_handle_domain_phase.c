/*****************************************************************************/
/*                                                                           */
/*  Projekt      : TEAM-COMPLETION                                           */
/*                                                                           */
/*  Modul        : exp_handle_domain_phase                                   */
/*                                                                           */
/*  Author       : Martin Kronenburg, 1993                                   */
/*                                                                           */
/*  Beschreibung : In diesem Modul wird die Startphase einer Domaene ver-    */
/*                 waltet und die Experten ins Team aufgenommen.             */
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
#include "exp_def_next_cycletime.h"
#include "exp_handle_domain_phase.h"
#include "exp_form_next_t.h"
#include "expert.h"
#include "team.h"
#define __j(name) {#name , name},

/*****************************************************************************/
/*                                                                           */
/*                            Globale Variablen                              */
/*                                                                           */
/*****************************************************************************/
/*****************************************************************************/
/*                                                                           */
/*                 Forward-Deklarationen interner Funktionen                 */
/*                                                                           */
/*****************************************************************************/

static void starte_neue_domaenenphase( DomMatchSpec *domspec );

static void continue_domaene_phase( DomMatchSpec *domspec );


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  starte_neue_domaenenphase                                */
/*                                                                           */
/*  Parameter    :  Zeiger auf das Match, dessen Startphase beginnt          */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Diese Funktion fuehrt die notwendigen Aktionen durch, die*/
/*                  beim Start einer neuen Domaenenphase erforderlich sind.  */
/*                  Sie ueberprueft:                                         */
/*                     - ob ueberhaupt ein Team angegeben wurde              */
/*                     - Behandeln eines leiterf"ahigen Experten             */
/*                       Wenn kein leiterf"ahiger Experte bisher im Team ist */
/*                       (z.B. ein stat. Experte oder aus einem anderen      */
/*                       Startteam), so wird der erste leiterf"ahige Experte */
/*                       im Startteam gesucht und ins neue Team aufgenommen. */
/*                       Gibt es im Startteam keinen leiterf"ahigen Experten,*/
/*                       so wird ein Platz freigehalten.                     */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  TeamDomCount                                             */
/*                  DomainTeamInfo                                           */
/*                                                                           */
/*****************************************************************************/

static void starte_neue_domaenenphase( DomMatchSpec *domspec )
{
	      /************************************************/
	      /* Nummer eines Experten des Startteams in der  */
	      /* externen Variablen ExpertGlobalInfo.         */
	      /************************************************/
  short         member_nr; 
	      /************************************************/
	      /* Pointer auf die aktuell betrachtete Dom"ane  */
	      /************************************************/
  DomainFrame  *akt_dom;
	      /************************************************/
	      /* true, wenn im neuen Team bereits ein leiter- */
	      /*       f"ahiger Experte vorkommt.             */
	      /************************************************/
  bool          leiterfaehiger_exp_im_team = false;
	      /************************************************/
	      /* Index des Experten in start_team_exp, der    */
	      /* leiterfaehig ist, falls nicht bereits ein    */
	      /* leiterf"ahiger Experte im Team ist.          */
	      /************************************************/
  short         leiter_nr_im_startteam = -1;
  short         i;

  akt_dom = &(DomainGlobalInfo[domspec->dom_nr]);

  /*********************************************************************/
  /* Uberpruefen, ob ueberhaupt ein Experte im Startteam angegeben ist */
  /*********************************************************************/
  if( akt_dom->start_team_groesse > 0 )
  {
    /*******************************************/
    /* Behandeln eines Leiterfaehigen Experten */
    /*******************************************/

    if( !leiter_faehiger_exp_in_neuem_team() )
    {
      /*******************************/
      /* Ueberpruefen des Startteams */
      /*******************************/
      for( i=0; i<akt_dom->start_team_groesse; i++ )
      {
	member_nr = get_exp_nr( akt_dom->start_team_exp[i].name );
	if( leiterfaehig( member_nr ) )
	{
	  /* Hier ist es entscheidend, dass noch fuer einen Experten Platz ist. */
	  /* Hier muss nicht abgefragt werden, ob der Experte schon im Team ist,*/
	  /* weil er sonst als leiterf"ahig erkannt worden w"are.               */
	  NewTeam[NewTeamLength++] = member_nr;
	  if( PlanDocum )
	  {
	    fprintf( plan_doc, "Experte %s im Startteam der Domaene %s.\n",
		     ExpertGlobalInfo[member_nr].exp_name, akt_dom->dom_name );
	  }
	  ExpertGlobalInfo[member_nr].dom_start_exp = true;
	  leiter_nr_im_startteam = i;
	  leiterfaehiger_exp_im_team = true;

	  break;
	} /* Ende von if leiterfaehig */
      } /* Ende von for */
    } /* Ende von if !leiter_faehiger_exp_in_neuem_team */
    else
    {
      leiterfaehiger_exp_im_team = true;
    } /* Ende von else */

    /***************************************************************************/
    /* Auffuellen der freien Plaetze im Team durch die Experten des Startteams */
    /***************************************************************************/
    for( i=0; ( i<akt_dom->start_team_groesse ) && 
	      ( (leiterfaehiger_exp_im_team && !team_besetzt() ) ||
		(!leiterfaehiger_exp_im_team && (freie_plaetze() > 1) ) ); i++ )
    {
      if( i == leiter_nr_im_startteam )
      {
	continue; /* Dieser Experte ist bereits eingetragen */
      }
      
      if( (member_nr = get_exp_nr( akt_dom->start_team_exp[i].name ) ) == NOEXPERT )
      {
	if( PlanDocum )
	{
	  fprintf( plan_doc, "Experte %s im Startteam der Domaene %s unbekannt!\n",
		   akt_dom->start_team_exp[i].name, akt_dom->dom_name );
	}
	  continue;
      }

      if( !exp_im_neuen_team( member_nr ) )
      {
	NewTeam[NewTeamLength++] = member_nr;
      }
      ExpertGlobalInfo[member_nr].dom_start_exp = true;
      if( PlanDocum )
      {
	fprintf( plan_doc, "Experte %s im Startteam der Domaene %s.\n",
		 ExpertGlobalInfo[member_nr].exp_name, akt_dom->dom_name );
      }
    } /* Ende von for */
  } /* Ende von if */

  /****************************/
  /* Kein StartTeam angegeben */
  /****************************/
  else
  { 
    if( PlanDocum )
    {
      fprintf( plan_doc, "Bei der Domaene %s wurde kein Startteam angegeben!\n",
	       akt_dom->dom_name );
    }
  } /* Ende von else */

  /***************************************************************************/
  /* Einstellen der Zeitbasis fuer die Zyklusdauer-Berechnung, falls in der  */
  /* Domaenenspezifikation eine Zeit angegeben wurde.                        */
  /***************************************************************************/
  if( akt_dom->zyklen_info[0].normal == NO_CYCLE_TIME )
  {
    if( PlanDocum )
    {
      fprintf( plan_doc, "Bei der Domaene %s wurde keine Zyklusdauer angegeben!\n",
	       akt_dom->dom_name );
      fprintf( plan_doc, "Daher erfolgt keine Anpassung bei der Basis fuer die Zeiberechnung!\n");
    } /* Ende von if ( PlanDocum ) */
  } /* Ende des aeusseren ifs */
  else
  /* Zyklendauer ist angegeben -> timebase wird auf den ersten angegeben */
  /* "Normalwert" gesetzt.                                               */
  /* Ber"ucksichtigt wird, da"s diese Angabe stets in Sekunden erfolgt */
  {
    if( timeunit == SECONDS )
    {
      timebase = akt_dom->zyklen_info[0].normal; 
    }
    else
    {
      timebase = 1000 * akt_dom->zyklen_info[0].normal; 
    }
  }/* Ende von else */

} /* Ende von starte_neue_domaenenphase */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  continue_domaene_phase                                   */
/*                                                                           */
/*  Parameter    :  Zeiger auf das Match, dessen Startphase beginnt          */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Diese Funktion fuehrt die notwendigen Aktionen durch, die*/
/*                  beim Fortsetzen einer  Domaenenphase erforderlich sind.  */
/*                  Sie ueberprueft:                                         */
/*                     - ob ueberhaupt ein Team angegeben wurde              */
/*                  BEACHTE : Es wird vorausgestzt, dass fuer wenigstens     */
/*                            einen Experten noch Platz im Team ist.         */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  TeamDomCount                                             */
/*                  DomainTeamInfo                                           */
/*                                                                           */
/*****************************************************************************/

static void continue_domaene_phase( DomMatchSpec *domspec )
{
	      /************************************************/
	      /* Nummer eines Experten des Startteams in der  */
	      /* externen Variablen ExpertGlobalInfo.         */
	      /************************************************/
  short         member_nr; 
	      /************************************************/
	      /* Pointer auf die aktuell betrachtete Dom"ane  */
	      /************************************************/
  DomainFrame  *akt_dom;
	      /************************************************/
	      /* true, wenn im neuen Team bereits ein leiter- */
	      /*       f"ahiger Experte vorkommt.             */
	      /************************************************/
  bool          leiterfaehiger_exp_im_team = false;
	      /************************************************/
	      /* Index des Experten in start_team_exp, der    */
	      /* leiterfaehig ist, falls nicht bereits ein    */
	      /* leiterf"ahiger Experte im Team ist.          */
	      /************************************************/
  short         leiter_nr_im_startteam = -1;
  short         i;

  akt_dom = &(DomainGlobalInfo[domspec->dom_nr]);

  /*********************************************************************/
  /* Uberpruefen, ob ueberhaupt ein Experte im Startteam angegeben ist */
  /*********************************************************************/
  if( akt_dom->start_team_groesse > 0 )
  {
    /*******************************************/
    /* Behandeln eines leiterfaehigen Experten */
    /*******************************************/

    if( !leiter_faehiger_exp_in_neuem_team() )
    {
      /*******************************/
      /* Ueberpruefen des Startteams */
      /*******************************/
      for( i=0; i<akt_dom->start_team_groesse; i++ )
      {
	member_nr = get_exp_nr( akt_dom->start_team_exp[i].name );
	if( leiterfaehig( member_nr ) )
	{
	  /* Hier ist es entscheidend, dass noch fuer einen Experten Platz ist */
	  /* Hier muss nicht abgefragt werden, ob der Experte schon im Team ist,*/
	  /* weil er sonst als leiterf"ahig erkannt worden w"are.               */
	  NewTeam[NewTeamLength++] = member_nr;
	  if( PlanDocum )
	  {
	    fprintf( plan_doc, "Experte %s im Startteam der Domaene %s.\n",
		     ExpertGlobalInfo[member_nr].exp_name, akt_dom->dom_name );
	  }
	  ExpertGlobalInfo[member_nr].dom_start_exp = true;
	  leiter_nr_im_startteam = i;
	  leiterfaehiger_exp_im_team = true;

	  break;
	} /* Ende von if leiterfaehig */
      } /* Ende von for */
    } /* Ende von if !leiter_faehiger_exp_in_neuem_team */
    else
    {
      leiterfaehiger_exp_im_team = true;
    } /* Ende von else */

    /***************************************************************************/
    /* Auffuellen der freien Plaetze im Team durch die Experten des Startteams */
    /***************************************************************************/
    for( i=0; ( i<akt_dom->start_team_groesse ) && 
	      ( (leiterfaehiger_exp_im_team && !team_besetzt() ) ||
		(!leiterfaehiger_exp_im_team && (freie_plaetze() > 1) ) ); i++ )
    {
      if( i == leiter_nr_im_startteam )
      {
	continue; /* Dieser Experte ist bereits eingetragen */
      }
      
      if( (member_nr = get_exp_nr( akt_dom->start_team_exp[i].name ) ) == NOEXPERT )
      {
	if( PlanDocum )
	{
	  fprintf( plan_doc, "Experte %s im Startteam der Domaene %s unbekannt!\n",
		   akt_dom->start_team_exp[i].name, akt_dom->dom_name );
	}
	  continue;
      }

      if( !exp_im_neuen_team( member_nr ) )
      {
	NewTeam[NewTeamLength++] = member_nr;
      }
      ExpertGlobalInfo[member_nr].dom_start_exp = true;
      if( PlanDocum )
      {
	fprintf( plan_doc, "Experte %s im Startteam der Domaene %s.\n",
		 ExpertGlobalInfo[member_nr].exp_name, akt_dom->dom_name );
      }
    } /* Ende von for */
  } /* Ende von if akt_dom->start_team_groesse > 0 */
  else
  {  
    /****************************/
    /* Kein StartTeam angegeben */
    /****************************/
    if( PlanDocum )
    {
      fprintf( plan_doc, "Bei der Domaene %s wurde kein Startteam angegeben!\n",
	       akt_dom->dom_name );
    }
  } /* Ende von else */
} /* Ende von continue_domaene_phase */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  nach_domaenenphase                                       */
/*                                                                           */
/*  Parameter    :  Zeiger auf das Match, dessen Startphase soeben zu Ende   */
/*                  gegangen ist.                                            */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Diese Funktion fuehrt die notwendigen Aktionen durch, die*/
/*                  im Anschluss an die Startphase einer Domaene erforder-   */
/*                  lich ist. Zur Zeit werden, falls angegeben, die Nach-    */
/*                  folger ins Team gebracht!                                */
/*                  BEACHTE : Es wird vorausgestzt, dass fuer wenigstens     */
/*                            einen Experten noch Platz im Team ist.         */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  TeamDomCount                                             */
/*                  DomainTeamInfo                                           */
/*                                                                           */
/*****************************************************************************/

static void nach_domaenenphase( DomMatchSpec *domspec )
{
  
} /* Ende von nach_domaenenphase */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  HandleDomainPhase                                        */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Diese Funktion ueberprueft, ob die erste relevante Domae-*/
/*                  ne ihre Startphase beginnt, sich noch in ihr befindet    */
/*                  und fortgesetzt werden muss oder ob sie diese bereits    */
/*                  beendet hat und nur noch relevant ist fuer den naechsten */
/*                  Zyklus. Im letzten Fall wird ueberprueft, ob das der er- */
/*                  ste Zyklus nach der Startphase ist. Falls Nachfolger an- */
/*                  gegeben sin, werden diese dann naemlich eingesetzt.      */
/*                  Es werden die notwendigen Funktionen aufgerufen.         */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  TeamDomCount                                             */
/*                  DomainTeamInfo                                           */
/*                                                                           */
/*****************************************************************************/

void HandleDomainPhase( void )
{
  short i;

  /******************************************************/
  /* Ueberpruefen, ob relevante Domaenen vorhanden sind */
  /******************************************************/
  for(i=0; i<TeamDomCount; i++)
  {
    if( new_rel_dom( &(DomainTeamInfo[i]) ) )
    {
      if( PlanDocum )
      {
	fprintf( plan_doc, "Neue Domaenenstartphase der Domaene %s\n",
			   DomainGlobalInfo[DomainTeamInfo[i].dom_nr].dom_name );
      }

      /************************************/
      /* Beginn einer neuen Domaenenphase */
      /************************************/
      starte_neue_domaenenphase(  &(DomainTeamInfo[i]) );
    } /* Ende von if new_rel_dom */

    /******************************************************/
    /* Ueberpruefen, ob die Domaene in der Startphase ist */
    /******************************************************/
    else if( DomainGlobalInfo[DomainTeamInfo[i].dom_nr].
	     DomMatches[DomainTeamInfo[i].match_nr].anz_einsaetze <=
	     DomainGlobalInfo[DomainTeamInfo[i].dom_nr].startzyklen_anz )
    {
      if( PlanDocum )
      {
	fprintf( plan_doc, "Domaenenstartphase der Domaene %s wird fortgesetzt.\n",
			   DomainGlobalInfo[DomainTeamInfo[i].dom_nr].dom_name );
      }
      /*******************************************************/
      /* Fortsetzen einer Domaene, die in der Startphase ist */
      /*******************************************************/
      continue_domaene_phase( &(DomainTeamInfo[i]) );
    } /* Ende von else if */
    /*****************************************************************/
    /* Ueberpruefen, ob das der erste Zyklus nach der Startphase ist */
    /*****************************************************************/
    else if( DomainGlobalInfo[DomainTeamInfo[i].dom_nr].
	     DomMatches[DomainTeamInfo[i].match_nr].anz_einsaetze == 
	     DomainGlobalInfo[DomainTeamInfo[i].dom_nr].startzyklen_anz + 1 )
    {
      nach_domaenenphase( &(DomainTeamInfo[i]) );
    }
    else
    {
      /*************************************************************************/
      /* Domaene befindet sich nicht mehr in der Startphase, nur noch relevant */
      /*************************************************************************/
      if( PlanDocum )
      {
	fprintf( plan_doc, "Domaene %s befindet sich nicht mehr in der Startphase.\n",
			   DomainGlobalInfo[DomainTeamInfo[i].dom_nr].dom_name );
      }
    } /* Ende von else */

  } /* Ende von for */
} /* Ende von HandleDomainPhase */
