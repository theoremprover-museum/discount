/*****************************************************************************/
/*                                                                           */
/*  Projekt      : TEAM-COMPLETION                                           */
/*                                                                           */
/*  Modul        : exp_handle_static_exp                                     */
/*                                                                           */
/*  Author       : Martin Kronenburg, 1993                                   */
/*                                                                           */
/*  Beschreibung : In diesem Modul werden die als initial oder statisch ge-  */
/*                 kennzeichneten Experten/Spezialisten bearbeitet.          */
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
#include "exp_handle_static_exp.h"
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
/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  HandleStaticExperts                                      */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  In dieser Funktion werden die Experten und Spezialisten  */
/*                  mit dem Attribut STATIC bzw. INITIAL betrachtet, d.h.    */
/*                  direkt ins Team aufgenommen. Sie werden nicht weiter be- */
/*                  trachtet.                                                */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

void HandleStaticExperts( void )
{
  short i;

  /***********************************************************************/
  /* Expertenliste wird durchlaufen : Suchen von STATIC/INITIAL-Experten */
  /***********************************************************************/

  if( PlanDocum )
  {
    fprintf( plan_doc, "\n\nDie folgenden Experten sind als STATIC oder INITIAL definiert:\n");
  }

  for( i=0; i<ExpertCount; i++ )
  {
    if ( StaticExp(i) || ( InitialExp(i) && !CycleCount) )
    {  /* Experte muss ins Team */
      if( PlanDocum )
      {
	fprintf( plan_doc, "    Experte %s\n", ExpertGlobalInfo[i].exp_name );
      }

      NewTeam[NewTeamLength++] = i;
    }
  } /* Ende von for */
  if( PlanDocum )
  {
    fprintf( plan_doc, "\n\n");
  }
  
  /*******************************/
  /* Betrachten der Spezialisten */
  /*******************************/
      /********************/
      /* Database-Experte */
      /********************/
  if ( ((!CycleCount &&  (Database.typ == SINITIAL) ) || (Database.typ == SSTATIC)) && SupposedDomainsCount )
  {
    if( PlanDocum )
    {
      fprintf( plan_doc, "    DATABASE-Spezialist als STATIC/INITIAL definiert\n");
    }

    NewTeam[NewTeamLength++] = DATABASE;
  }

      /**************************/
      /* Reduktionsspezialisten */
      /**************************/
  for( i=0; i<RedSpecCount; i++ )
  {
    if( (!CycleCount && InitialRedSpec(i)) || (StaticRedSpec(i)) )
    {
      if( PlanDocum )
      {
	fprintf( plan_doc, "    Reduktionsspezialist %s als STATIC/INITIAL definiert\n",
		 RedSpecInfo[i].name );
      }
      
      NewTeam[NewTeamLength++] = RedSpecNo(i);
    }
  } /* Ende von for */
} /* Ende von HandleStaticExperts */
