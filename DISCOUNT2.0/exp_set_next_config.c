/*****************************************************************************/
/*                                                                           */
/*  Projekt      : TEAM-COMPLETION                                           */
/*                                                                           */
/*  Modul        : exp_set_next_config                                       */
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
#include "expert.h"
#include "exp_set_next_config.h"
#include "team.h"
#define __j(name) {#name , name},

/*****************************************************************************/
/*                                                                           */
/*                            Lokale Variablen                               */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*                            Globale Variablen                              */
/*                                                                           */
/*****************************************************************************/
               /************************************************/
	       /* Nummer der Dom"ane aus SupposedDomain, bei   */
	       /* der das letzte Mal der Database seine Suche  */
	       /* gestartet hat.                               */
               /************************************************/
short StartDomNr = 0;

/*****************************************************************************/
/*                                                                           */
/*                 Forward-Deklarationen interner Funktionen                 */
/*                                                                           */
/*****************************************************************************/


/*****************************************************************************/
/*                                                                           */
/*                  Hilfsfunktionen fuer SetNextConfiguration                */
/*                                                                           */
/*****************************************************************************/


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  set_parameter                                            */
/*                                                                           */
/*  Parameter    :  Pointer auf ExpertConfig                                 */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Es werden die Parameter fuer den in der Struktur durch   */
/*                  exp_nr spezifizierten Experten fuer cp-Funktion und      */
/*                  cg-Funktion bestimmt.                                    */
/*                  Der gewuenschte Experte muss also vorher dort bereits    */
/*                  eingetragen wurden sein.                                 */
/*                  Zur Zeit werden einfach die Default-Werte der Parameter  */
/*                  uebertragen.                                             */
/*                  Spaeter sollten die Parameter berechnet werden - mit     */
/*                  Aussnahme der Polynom-Experten                           */
/*                                                                           */
/*  Globale Var. :  ExpertGlobalInfo                                         */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

void set_parameter ( ExpertConfig *expconfig )
{
  short i, j;
  short red_spec_idx; /* Index eines Reduktionsspezialisten                  */

  /*********************************************/
  /* Setzen der Parameter fuer die CP-Funktion */
  /*********************************************/
  /* Sonderbehandlung bei den Spezialisten     */
  /*********************************************/
  if ( exp_ist_spezialist( expconfig->exp_nr ) )
  {
    switch( expconfig->exp_nr )
    {
      case DATABASE  : /****************************************************/
		       /* Der erste Parameter besagt, nach wievielen Domae-*/
		       /* nen gesucht werden soll.                         */
		       /****************************************************/

		       /*********************************************/
		       /* Neuen Startpunkt f"ur die Suche bestimmen */
		       /*********************************************/
		       StartDomNr += LookedDomains;
		       if( StartDomNr == SupposedDomainsCount )
		       {
			 /********************************/
			 /* Suche f"angt wieder vorne an */
			 /********************************/
			 expconfig->cp_parameter[0] = SupposedDomainsCount;
			 StartDomNr = 0;
		       }
		       else
		       {
			 expconfig->cp_parameter[0] = SupposedDomainsCount - StartDomNr;
		       }

		       /************************************************************/
		       /* "Ubertragen der Dom"anen, nach denen gesucht werden soll */
		       /************************************************************/
 		       for ( j=1,i=StartDomNr; (i<SupposedDomainsCount) && (j<MAXPARAXPERT); i++, j++ )
		       {
			 expconfig->cp_parameter[j] = SupposedDomains[i];
		       }

		       /**************************************************/
		       /* Der erste Parameter von cg_parameter gibt den  */
		       /* Suchmodus an.                                  */
		       /**************************************************/
		       expconfig->cg_parameter[0] = Database.cpfkt_nr;

		       break;

      case REDUCE_1  :
      case REDUCE_2  :
      case REDUCE_3  : red_spec_idx = GetRedSpecIdx(expconfig->exp_nr);

		       switch( RedSpecInfo[red_spec_idx].red_typ )
		       {
			 case CP: expconfig->cp_parameter[0] = RedSpecInfo[red_spec_idx].cp_start;
				  expconfig->cp_parameter[1] = RedSpecInfo[red_spec_idx].reducecp;
				  expconfig->cp_parameter[2] = RedSpecInfo[red_spec_idx].subsumcp;
				  expconfig->cp_parameter[3] = RedSpecInfo[red_spec_idx].doublecp;
				  break;

			 case CG: expconfig->cg_parameter[0] = RedSpecInfo[red_spec_idx].cg_start;
				  expconfig->cg_parameter[1] = RedSpecInfo[red_spec_idx].reducecg;
				  expconfig->cg_parameter[2] = RedSpecInfo[red_spec_idx].subsumcg;
				  expconfig->cg_parameter[3] = RedSpecInfo[red_spec_idx].doublecg;
				  break;

			 default: Error( __FILE__ " : " "set_parameter", "Unbekannter Reduktionstyp." );
			 	  break;
		       } /* Ende von switch */
		       break;

      default        : Error( __FILE__ " : " "set_parameter", "Unbekannter Spezialist." );
		       break;
    } /* Ende von switch */
  } /* Ende von if */

  /**********************/
  /* Jetzt die Experten */
  /*********************************************/
  /* Nur bei Experten gibt es eine CG-Funktion */
  /*********************************************/
  else
  {
    switch ( ExpertGlobalInfo[expconfig->exp_nr].cpfkt_nr )
    {
      case SX_OCCNEST     :
      case SX_ADD         :
      case SX_FIFO        :
      case SX_TEAM_FIFO   :
      case SX_GLOBAL_LEARN:
      case SX_GOAL_BOUND_LEARN:
      case SX_SPEC_BOUND_LEARN:
      case SX_1TET_LEARN:
      case SX_2TET_LEARN:
      case SX_MAX         :
      case SX_GT          :
      case SX_DIFF        :
      case SX_SPEC_ADD    :
      case SX_SPEC_MAX    :
      case SX_SPEC_GT     :
      case SX_KBO_ADD     :
      case SX_KBO_MAX     :
      case SX_KBO_GT      :
      case SX_POLY_ADD    :
      case SX_POLY_MAX    :
      case SX_POLY_GT     :
      case SX_GOALMATCH   :
      case SX_GOALSIM     :
      case SX_GOALTEST    :
      case SX_UNIF_GOAL   :
      case SX_DIFF_GOAL   :
      case SX_DIVERGENCE  : for ( i=0; i<ExpertGlobalInfo[expconfig->exp_nr].cpparam_anz; i++ )
			    {
			      expconfig->cp_parameter[i] =
				  ExpertGlobalInfo[expconfig->exp_nr].cpparamliste[i].normal;
			    }
			    break;

      case SX_ADD_RWEIGHT :
      case SX_MAX_RWEIGHT :
      case SX_GT_RWEIGHT  :
      case SX_ADD_EWEIGHT :
      case SX_MAX_EWEIGHT :
      case SX_GT_EWEIGHT  :
      case SX_ADD_FWEIGHT :
      case SX_MAX_FWEIGHT :
      case SX_GT_FWEIGHT  : /******************************************************/
			    /* Es werden zunaechst die Standard-Parameter gesetzt */
                            /******************************************************/
			    for ( i=0; i<3; i++ )
			    {
			      expconfig->cp_parameter[i] =
				ExpertGlobalInfo[expconfig->exp_nr].cpparamliste[i].normal;
			    }
			    
			    /************************************************************/
			    /* Uebertragen der Liste mit speziellen Gewichten fuer Fkten*/
			    /************************************************************/
			    i=3;
			    while ( i<ExpertGlobalInfo[expconfig->exp_nr].cpparam_anz )
			    {
			      expconfig->cp_parameter[i] = 
				ExpertGlobalInfo[expconfig->exp_nr].cpparamliste[i].normal;
			      expconfig->cp_parameter[i+1] =
				ExpertGlobalInfo[expconfig->exp_nr].cpparamliste[i+1].normal;
			      i += 2;
			    }
			    break;
      default             : Error( __FILE__ " : " "set_parameter", "Unbekannte CP-Funktion." );
			 	  break;
    }

    /*********************************************/
    /* Setzen der Parameter fuer die CG-Funktion */
    /*********************************************/
    switch ( ExpertGlobalInfo[expconfig->exp_nr].cgfkt_nr )
    {
      case SX_OCCNEST     :
      case SX_ADD         :
      case SX_MAX         :
      case SX_GT          :
      case SX_DIFF        :
      case SX_SPEC_ADD    :
      case SX_SPEC_MAX    :
      case SX_SPEC_GT     :
      case SX_KBO_ADD     :
      case SX_KBO_MAX     :
      case SX_KBO_GT      :
      case SX_GOALMATCH   :
      case SX_GOALSIM     :
      case SX_GOALTEST    :
      case SX_UNIF_GOAL   :
      case SX_DIFF_GOAL   :
      case SX_DIVERGENCE  : for ( i=0; i<ExpertGlobalInfo[expconfig->exp_nr].cgparam_anz; i++ )
			    {
			      expconfig->cg_parameter[i] = 
				ExpertGlobalInfo[expconfig->exp_nr].cgparamliste[i].normal;
			    }
			    break;

      case SX_ADD_RWEIGHT :
      case SX_MAX_RWEIGHT :
      case SX_GT_RWEIGHT  :
      case SX_ADD_EWEIGHT :
      case SX_MAX_EWEIGHT :
      case SX_GT_EWEIGHT  :
      case SX_ADD_FWEIGHT :
      case SX_MAX_FWEIGHT :
      case SX_GT_FWEIGHT  : /******************************************************/
			    /* Es werden zunaechst die Standard-Parameter gesetzt */
			    /******************************************************/
			    for ( i=0; i<3; i++ )
			    {
			      expconfig->cg_parameter[i] =
				ExpertGlobalInfo[expconfig->exp_nr].cgparamliste[i].normal;
			    }
			    
			    /************************************************************/
			    /* Uebertragen der Liste mit speziellen Gewichten fuer Fkten*/
			    /************************************************************/
			    i=3;
			    while ( i<ExpertGlobalInfo[expconfig->exp_nr].cgparam_anz )
			    {
			      expconfig->cg_parameter[i] = 
				ExpertGlobalInfo[expconfig->exp_nr].cgparamliste[i].normal;
			      expconfig->cg_parameter[i+1] =
				ExpertGlobalInfo[expconfig->exp_nr].cgparamliste[i+1].normal;
			      i += 2;
			    }
			    break;

      default             : Error( __FILE__ " : " "set_parameter", "Unbekannte CG-Funktion." );
			    break;

    } /* Ende von switch */
  } /* Ende von else -> Experten */
} /* Ende von set_parameter */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  SetNextConfiguration                                     */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Diese Komponente ordnet jedem Experten,der fuer den      */
/*                  naechsten Zyklus ausgewaehlt wurde, seine Konfiguration  */
/*                  zu und legt sie in der globalen Variable ExpertTeamInfo  */
/*                  ab.                                                      */
/*                  Dabei werden lediglich die Parameterlisten fuer die      */
/*                  cp-Funktionen und die cg-Funktionen festgelegt.          */
/*                  Alles andere kann an Hand der Expertennummer aus         */
/*                  ExpertGlobalInfo gelesen werden.                         */
/*                                                                           */
/*  Globale Var. :  ExpertGlobalInfo                                         */
/*                  ExpertTeamInfo                                           */
/*                  ExpWinnerList                                            */
/*                  TeamExpCount                                             */
/*                  CycleCount                                               */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

void SetNextConfiguration ( void )
{
   short i;            /* Laufvariable fuer die Experten im Team              */
  
   short red_spec_idx; /* Index eines Reduktionsspezialisten                  */
   short rechner_nr;   /* Nummer des Rechners der als naechstes besetzt wird  */
   /* Rechner des Siegers wird nicht besetzt!!!!!         */
   short exp_idx_auf_win_host = 0; /* Index (in  NewTeam ) des Experten, der  */
   /* auf dem Leiter-Rechner laeuft.          */
   
   neuer_leiter_exp = true;
   
   /***************************************************************************/
   /* Einsetzen eines leiterfaehigen Experten auf dem aktuellen Leiter-Rechner*/
   /* Es wird versucht, denselben Experten des letzten Zyklus auf dem Leiter- */
   /* Rechner einzusetzen, weil in dem Fall, die kritischen Paare und Ziele   */
   /* nicht neu bewertet werden muesen.  Wenn dies moeglich ist, hat die      */
   /* Variable neuer_leiter_exp den Wert false, sonst true.                   */
   /* Dieser Test macht vor dem ersten Zyklus natuerlich keinen Sinn!         */
   /***************************************************************************/
   if( CycleCount )
   {
      for( i=0; i<TeamExpCount; i++ )
      {
	 /* Testen, ob der Sieger auch im neuen Teamm ist */
	 if( NewTeam[i] == ExpertTeamInfo[WinnerHost].exp_nr )
	 {
	    /* Die Parameter werden gesetzt, weil sie sich ja geaendert haben koennten */
	    /* rein theoretisch bei spaeteren Versionen !!!!                           */
	    set_parameter( &(ExpertTeamInfo[WinnerHost]) );
	    exp_idx_auf_win_host = i;
	    neuer_leiter_exp = false;
	    break;
	 } /* Ende von if NewTeam[i] == ExpertTeamInfo[WinnerHost].exp_nr */
      } /* Ende von for */
   } /* Ende von if CycleCount */
   
   if( neuer_leiter_exp )
   { /* der letzte Sieger ist nicht wieder im Team */
      for( i=0; i<TeamExpCount; i++ )
      {
	 if( leiterfaehig( NewTeam[i] ) )
	 {
	    ExpertTeamInfo[WinnerHost].exp_nr = NewTeam[i];
	    set_parameter( &(ExpertTeamInfo[WinnerHost]) );
	    exp_idx_auf_win_host = i;
	    break;
	 }
      } /* Ende von for */
   } /* Ende von if neuer_leiter_exp */
   
   /**********************************/
   /* Belegen der restlichen Rechner */
   /**********************************/
   for( i=0, rechner_nr=0; (i<TeamExpCount) && (rechner_nr<HostCount); i++, rechner_nr++ )
   {
      if ( i == exp_idx_auf_win_host )
      {
	 i++;   /* Experte auf Leiter-Rechner wird uebersprungen */
      }
      
      if ( rechner_nr == WinnerHost )
      {
	 rechner_nr++; /* Rechner des Siegers/neuen Leiters wird uebersprungen  */
      }
      
      /* Testen, ob noch Rechner zu belegen sind */
      if ( (rechner_nr < HostCount ) && ( i < TeamExpCount ) )
      {
	 ExpertTeamInfo[rechner_nr].exp_nr = NewTeam[i];
	 
	 set_parameter( &(ExpertTeamInfo[rechner_nr]) );             
      }
   } /* Ende von for */
   
   if ( PlanDocum )
   {
      fprintf( plan_doc, "\n************************************************************\n");
      fprintf( plan_doc, "********  Die naechste Team-Zusammensetzung lautet : *******\n");
      fprintf( plan_doc, "************************************************************\n\n");
      flush();
      for(i=0; i<TeamExpCount; i++)
      {
	 short j;
	 
	 fprintf( plan_doc, "Auf Rechner %d arbeitet Experte %s mit den Parametern :\n",
		 i, get_exp_name(ExpertTeamInfo[i].exp_nr) );
	 
	 if( exp_ist_spezialist( ExpertTeamInfo[i].exp_nr ) )
	 {
	    /*************************************/
	    /* Sonderbehandlung bei Spezialisten */
	    /*************************************/
	    switch ( ExpertTeamInfo[i].exp_nr )
	    {
	    case DATABASE  : 
	       fprintf(plan_doc, "Es wird nach folgenden %d Domaenen gesucht :\n",
		       ExpertTeamInfo[i].cp_parameter[0] );
	       for ( j=1; j<=ExpertTeamInfo[i].cp_parameter[0]; j++ )
		  fprintf(plan_doc, " %d. Domaene : %s\n", j,
			  DomainGlobalInfo[ExpertTeamInfo[i].cp_parameter[j]].dom_name );
	       fprintf(plan_doc, "Suchmodus ist : %s\n",
		       ExpertTeamInfo[i].cg_parameter[0] == SFIND_ALL ?
		       "FIND_ALL" : "FIND_FIRST" );
	       break;
	       
	    case REDUCE_1  :
	    case REDUCE_2  :
	    case REDUCE_3  : 
	       red_spec_idx = GetRedSpecIdx( ExpertTeamInfo[i].exp_nr );
	       
	       if( RedSpecInfo[red_spec_idx].red_typ == CP )
	       {
		  if( RedSpecInfo[red_spec_idx].reduce_cp_fkt == SX_REDUCE_CP )
		  {
		     fprintf(plan_doc, "Startpunkt krititischer Paare: %d\n", 
			     ExpertTeamInfo[i].cp_parameter[0] );
		  }
		  else
		  {
		     fprintf(plan_doc, "Starte bei krit. Paaren mit prozent. Anteil   : %d\n",
			     ExpertTeamInfo[i].cp_parameter[0] );
		  }
		  fprintf(plan_doc, "Reduktion kritischer Paare   : %s\n", 
			  ExpertTeamInfo[i].cp_parameter[1] ? "Ja" : "Nein" );
		  fprintf(plan_doc, "Ueberdeckung kritischer Paare: %s\n",
			  ExpertTeamInfo[i].cp_parameter[2] ? "Ja" : "Nein" );
		  fprintf(plan_doc, "Doppelte kritischer Paare    : %s\n",
			  ExpertTeamInfo[i].cp_parameter[3] ? "Ja" : "Nein" );
	       }
	       
	       if( RedSpecInfo[red_spec_idx].red_typ == CG )
	       {
		  if( RedSpecInfo[red_spec_idx].reduce_cg_fkt == SX_REDUCE_CG )
		  {
		     fprintf(plan_doc, "Startpunkt krititischer Ziele: %d\n", 
			     ExpertTeamInfo[i].cg_parameter[0] );
		  }
		  else
		  {
		     fprintf(plan_doc, "Starte bei krit. Zielen mit prozent. Anteil   : %d\n",
			     ExpertTeamInfo[i].cg_parameter[0] );
		  }
		  fprintf(plan_doc, "Reduktion kritischer Ziele   : %s\n", 
			  ExpertTeamInfo[i].cg_parameter[1] ? "Ja" : "Nein" );
		  fprintf(plan_doc, "Ueberdeckung kritischer Ziele: %s\n",
			  ExpertTeamInfo[i].cg_parameter[2] ? "Ja" : "Nein" );
		  fprintf(plan_doc, "Doppelte kritischer Ziele    : %s\n",
			  ExpertTeamInfo[i].cg_parameter[3] ? "Ja" : "Nein" );
	       }
	       break;
	       
	    default        : 
	       Error( __FILE__ ": "  "SetNextConfiguration", "Unbekannter Spezialist!!!" );
	       break;
	    }
	 } /* Ende von if */
	 else
	 {
	    /***********/
	    /* Experte */
	    /***********/
	    
	    for ( j=0; j<ExpertGlobalInfo[ExpertTeamInfo[i].exp_nr].cpparam_anz; j++)
	    {
	       fprintf( plan_doc, "%-20s : %5d \n", 
		       ExpertGlobalInfo[ExpertTeamInfo[i].exp_nr].cpparamliste[j].name, 
		       ExpertTeamInfo[i].cp_parameter[j] );
	       
	    } /* Ende von for */
	 } /* Ende von else */
	 fprintf( plan_doc, "\n");
      } /* Ende von for */
   } /* Ende von if( PlanDocum ) */
} /* Ende von SetNextConfiguration */

