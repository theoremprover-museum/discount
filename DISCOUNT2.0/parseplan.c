/*****************************************************************************/
/*                                                                           */
/*  Projekt      : TEAM-COMPLETION                                           */
/*                                                                           */
/*  Modul        :                                                           */
/*                                                                           */
/*  Author       : Martin Kronenburg, 1993                                   */
/*                                                                           */
/*  Beschreibung : In diesem Modul ist der Parser fuer die Konfigurations-   */
/*                 datei mit den planungsrelevanten Daten enthalten.         */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <sys/param.h>
#include "defines.h"
#include "error.h"
#include "polynom.h"
#include "vartree.h"
#include "term.h"
#include "termpair.h"
#include "complet.h"
#include "domain.h"
#include "scanner.h"
#include "database.h"
#include "expert.h"
#include "exp_def_next_cycletime.h"
#include "exp_class_t_exp.h"
#include "exp_value_exp.h"
#include "exp_def_break.h"
#include "exp_class_nont_exp.h"
#include "exp_form_next_t.h"
#include "parseexp.h"
#include "parseplan.h"


/*****************************************************************************/
/*                                                                           */
/*                            Globale Variablen                              */
/*                                                                           */
/*****************************************************************************/

static char Filename[MAXPATHLEN];

static short f_descr; /* Interner Filedescriptor des Scanners */


/*****************************************************************************/
/*                                                                           */
/*               Variablen, die auch extern verwendet werden                 */
/*                                                                           */
/*****************************************************************************/

char PlanPath[MAXPATHLEN];


/*****************************************************************************/
/*                                                                           */
/*                 Forward-Deklarationen interner Funktionen                 */
/*                                                                           */
/*****************************************************************************/

static void    SyntaxError ( char *error );
static void    CheckError ( bool cond, char *msg );
static void    ReadZeile ( Symbol symbol, char *wort,
		           void *zahl, double standard, short zahl_typ );

/*
//-----------------------------------------------------------------------------
//  Funktion:       SyntaxError
//
//  Parameter:      error       Fehlermeldung
//
//  Beschreibung:   Ausgabe einer Fehlermeldung
//-----------------------------------------------------------------------------
*/

static void    SyntaxError ( char *error )
{
    PrintScanText ( f_descr );
    printf ( "****  Fehler in der eingelesenen Datei : %s.\n", Filename );
    printf ( "****  %s\n", error );
}

/*
//-----------------------------------------------------------------------------
//  Funktion:       CheckError
//
//  Parameter:      cond    Bedingung fuer Fehlerfall
//                  msg     Fehlermeldung
//
//  Beschreibung:   Ausgabe einer Fehlermeldung, falls cond erfuellt.
//-----------------------------------------------------------------------------
*/
static void CheckError ( bool cond, char *msg )
{
    if (cond)
    {
        SyntaxError ( msg );
        CloseInput ( f_descr );
	getchar ();
        exit ( 1 );
    }
}



/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  ReadZeile                                                */
/*                                                                           */
/*  Parameter    :  - Symbolname, der erwartet wird                          */
/*                  - Wort fuer die Fehlermeldung --> welches Schluesselwort */
/*                    erwartet wurde                                         */
/*                  - Pointer auf Variable, in die der Wert geschrieben wird */
/*                  - Default-Wert, falls nichts angegeben                   */
/*                  - Angabe, ob Integer oder Real gelesen werden soll       */
/*                    ( 0 --> Integer, !=0 --> Real )                        */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Es wird eine Zeile aus der Planungsdatei gelesen und     */
/*                  je nachdem, ob ein Wert angegeben wurde, entweder        */
/*                  dieser in den angegebenen Speicherbereich geschrieben    */
/*                  oder der angegebene Default-Wert ( Standard ).           */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  PlanDocum                                                */
/*                                                                           */
/*****************************************************************************/

static void ReadZeile ( Symbol symbol, char *wort,
			void *zahl, double standard, short zahl_typ )
{
  char    ident[IDENTLENGTH];
  Symbol  sym;
  char    meldung[2*IDENTLENGTH];

  SkipSpace( f_descr, ident, sym);
  sprintf( meldung, "%s erwartet.",wort );
  CheckError ( sym!=symbol, meldung );

  if ( ( sym = GetSymbol( f_descr,  ident ) ) == SCRSYM )
  {  /* Zeile ist leer */
    if ( !zahl_typ )
    {
      *(int*)zahl = (int)standard;
    }
    else
    {
      *(double*)zahl = standard;
    }
    return;
  }
  
  if ( !zahl_typ )
  {  /* Integer-Zahl soll gelesen werden */
    CheckError ( ( sym != SIDENT) || !IsNumber (ident), "Ganzahl erwartet." );
    *(int*)zahl = atoi( ident );
    CheckError( GetSymbol( f_descr,  ident ) != SCRSYM, "Neue Zeile erwartet." );
  }
  else
  {   /* Realzahl wird eingelesen */
    CheckError ( ( sym != SIDENT) || !IsNumber ( ident ), "Realzahl erwartet." );
    *(double*)zahl = atoi( ident );

    sym = GetSymbol( f_descr,  ident );
    if ( sym == SPUNKT )     /* Zahl hat auch Nachkommastellen */
    {
      sym = GetSymbol( f_descr,  ident );

      CheckError ( EOI( f_descr ), "Unerwartetes Dateiende." );
      CheckError ( ( sym != SIDENT) || !IsNumber ( ident ), "Realzahl erwartet." );
      *(double*)zahl += atoi( ident )/exp_10(strlen( ident ));
    }
    CheckError( GetSymbol( f_descr,  ident ) != SCRSYM, "Neue Zeile erwartet." );
  }
} /* Ende von ReadZeile */
    

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  SetPlanParams                                            */
/*                                                                           */
/*  Parameter    :  keine                                                    */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Diese Funktion wird aufgerufen, wenn in der Konfigura-   */
/*                  tionsdatei kein Supervisor-File angegeben wurde.         */
/*                  Es werden hier die f"ur die Planung relevanten Variablen */
/*                  mit Default-Werten belegt.                               */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  Alle fuer die Planung relevanten Variablen.              */
/*                                                                           */
/*****************************************************************************/

void SetPlanParams ( void )
{
  nodomexp = NODOMEXP;
  maxreldomain = MAXRELDOMAIN;
  d_expdomfaktor = D_EXPDOMFAKTOR;
  d_exprobfaktor = D_EXPROBFAKTOR;
  d_expbewfaktor = D_EXPBEWFAKTOR;
  d_expkzgfaktor = D_EXPKZGFAKTOR;
  nod_expdomfaktor = NOD_EXPDOMFAKTOR;
  nod_exprobfaktor = NOD_EXPROBFAKTOR;
  nod_expbewfaktor = NOD_EXPBEWFAKTOR;
  nod_expkzgfaktor = NOD_EXPKZGFAKTOR;

  w_size_r = W_SIZE_R;
  w_size_e = W_SIZE_E;
  w_size_g = W_SIZE_G;
  w_size_cp = W_SIZE_CP;
  w_size_cg = W_SIZE_CG;
  w_new_r = W_NEW_R;
  w_red_r = W_RED_R;
  w_del_r = W_DEL_R;
  w_new_e = W_NEW_E;
  w_red_e = W_RED_E;
  w_del_e = W_DEL_E;
  w_new_g = W_NEW_G;
  w_red_g = W_RED_G;
  w_new_cp = W_NEW_CP;
  w_del_cp = W_DEL_CP;
  w_new_cg = W_NEW_CG;
  w_redcount = W_REDCOUNT;
  increase_percent = INCREASE_PERCENT ;

  l_size_r = L_SIZE_R;
  l_size_e = L_SIZE_E;
  l_size_g = L_SIZE_G;
  l_size_cp = L_SIZE_CP;
  l_size_cg = L_SIZE_CG;
  l_new_r = L_NEW_R;
  l_red_r = L_RED_R;
  l_del_r = L_DEL_R;
  l_new_e = L_NEW_E;
  l_red_e = L_RED_E;
  l_del_e = L_DEL_E;
  l_new_g = L_NEW_G;
  l_red_g = L_RED_G;
  l_new_cp = L_NEW_CP;
  l_del_cp = L_DEL_CP;
  l_new_cg = L_NEW_CG;
  l_redcount = L_REDCOUNT;

  decrease_percent = DECREASE_PERCENT ;
  verlierermaxcycl = VERLIERERMAXCYCL;
  verlierergrenze3 = VERLIERERGRENZE3;
  teamnotchanged = TEAMNOTCHANGED;

  host_limit_db = HOST_LIMIT_DB;
  cycle_diff_db = CYCLE_DIFF_DB;
  dom_limit_db = DOM_LIMIT_DB;

  host_limit_cp = HOST_LIMIT_CP;
  cycle_diff_cp = CYCLE_DIFF_CP;
  crit_p_limit_cp = CRIT_P_LIMIT_CP;

  host_limit_cg = HOST_LIMIT_CG;
  cycle_diff_cg = CYCLE_DIFF_CG;
  crit_g_limit_cg = CRIT_G_LIMIT_CG;

  domain_suited = DOMAIN_SUITED;
  winner_suited = WINNER_SUITED;
  j_not_in_team = J_NOT_IN_TEAM;
  not_judged = NOT_JUDGED;

  min_inspected_x = MIN_INSPECTED_X;
  exp_part = EXP_PART;
  extra_exp1 = EXTRA_EXP1;
  loser_part = LOSER_PART;
  extra_exp2 = EXTRA_EXP2;
  loser_percent = LOSER_PERCENT;
  extra_exp3 = EXTRA_EXP3;
  min_better_exp = MIN_BETTER_EXP;

  good_dom_exp_kno = GOOD_DOM_EXP_KNO;
  bad_dom_exp_kno = BAD_DOM_EXP_KNO;
  no_dom_exp_kno = NO_DOM_EXP_KNO;

  relevantlastcycl = RELEVANTLASTCYCL;
  minimumofactions = MINIMUMOFACTIONS;
  inspectedactions = INSPECTEDACTIONS;
  maxdifference = MAXDIFFERENCE;
  constantfactor = CONSTANTFACTOR;
  trendexpfactor = TRENDEXPFACTOR;
  relkzgfactor = RELKZGFACTOR;
  proof_mid = PROOF_MID;

  if( !timebase_angegeben )
  {
    timebase = TIMEBASE;
  }
  tp_part = TP_PART;
  time_extra = TIME_EXTRA;
  good_team_t_fac = GOOD_TEAM_T_FAC;
  bad_team_t_fac = BAD_TEAM_T_FAC;
  upper_limit1 = UPPER_LIMIT1;
  down_limit1 = DOWN_LIMIT1;
  upper_limit2 = UPPER_LIMIT2;
  down_limit2 = DOWN_LIMIT2;
  goalfaktor = GOALFAKTOR;
} /* Ende von SetPlanParams */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  ParsePlan                                                */
/*                                                                           */
/*  Parameter    :  Name der Datei, die gelesen werden soll.                 */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Es wird die angegebene Datei gelesen, in der die Daten   */
/*                  fuer die Planung stehen muessen.                         */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  Alle fuer die Planung relevanten Variablen aus           */
/*                  expert.h und domain.h .                                  */
/*                                                                           */
/*****************************************************************************/

void ParsePlan ( char *filename )
{
  char ident[IDENTLENGTH];

  int  timebase_dummy;  /* Falls in der Konfigurationsdatei explizit TIMEBASE*/
			/* angegeben wird, wird in diese Variable der in der */
			/* Planungsdatei angegebene Wert f"ur timebase ge-   */
			/* schrieben, aber nicht in die globalke Variable    */

  strcpy( Filename, filename );

  if( !DemoMode )
  {
    printf("PlanungsDatei %s lesen\n", Filename );
  }

  f_descr = OpenInput ( filename );
  SetScanMode ( f_descr, PLAN_MODE );

  /* Einlesen der Daten */
  ReadZeile( SNODOMEXP, "NODOMEXP", (void*)&(nodomexp),(double)NODOMEXP,0);
  ReadZeile( SMAXRELDOMAIN, "MAXRELDOMAIN", (void*)&(maxreldomain),(double)MAXRELDOMAIN,0);

  ReadZeile( SD_EXPDOMFAKTOR, "D_EXPDOMFAKTOR", (void*)&(d_expdomfaktor),(double)D_EXPDOMFAKTOR,1);
  ReadZeile( SD_EXPROBFAKTOR, "D_EXPROBFAKTOR", (void*)&(d_exprobfaktor),(double)D_EXPROBFAKTOR,1);
  ReadZeile( SD_EXPBEWFAKTOR, "D_EXPBEWFAKTOR", (void*)&(d_expbewfaktor),(double)D_EXPBEWFAKTOR,1);
  ReadZeile( SD_EXPKZGFAKTOR, "D_EXPKZGFAKTOR", (void*)&(d_expkzgfaktor),(double)D_EXPKZGFAKTOR,1);
  ReadZeile( SNOD_EXPDOMFAKTOR, "NOD_EXPDOMFAKTOR", (void*)&(nod_expdomfaktor),(double)NOD_EXPDOMFAKTOR,1);
  ReadZeile( SNOD_EXPROBFAKTOR, "NOD_EXPROBFAKTOR", (void*)&(nod_exprobfaktor),(double)NOD_EXPROBFAKTOR,1);
  ReadZeile( SNOD_EXPBEWFAKTOR, "NOD_EXPBEWFAKTOR", (void*)&(nod_expbewfaktor),(double)NOD_EXPBEWFAKTOR,1);
  ReadZeile( SNOD_EXPKZGFAKTOR, "NOD_EXPKZGFAKTOR", (void*)&(nod_expkzgfaktor),(double)NOD_EXPKZGFAKTOR,1);


  ReadZeile( SW_SIZE_R, "W_SIZE_R", (void*)&(w_size_r),(double)W_SIZE_R,0);
  ReadZeile( SW_SIZE_E, "W_SIZE_E", (void*)&(w_size_e),(double)W_SIZE_E,0);
  ReadZeile( SW_SIZE_G, "W_SIZE_G", (void*)&(w_size_g),(double)W_SIZE_G,0);
  ReadZeile( SW_SIZE_CP, "W_SIZE_CP", (void*)&(w_size_cp),(double)W_SIZE_CP,0);
  ReadZeile( SW_SIZE_CG, "W_SIZE_CG", (void*)&(w_size_cg),(double)W_SIZE_CG,0);
  ReadZeile( SW_NEW_R, "W_NEW_R", (void*)&(w_new_r),(double)W_NEW_R,0);
  ReadZeile( SW_RED_R, "W_RED_R", (void*)&(w_red_r),(double)W_RED_R,0);
  ReadZeile( SW_DEL_R, "W_DEL_R", (void*)&(w_del_r),(double)W_DEL_R,0);
  ReadZeile( SW_NEW_E, "W_NEW_E", (void*)&(w_new_e),(double)W_NEW_E,0);
  ReadZeile( SW_RED_E, "W_RED_E", (void*)&(w_red_e),(double)W_RED_E,0);
  ReadZeile( SW_DEL_E, "W_DEL_E", (void*)&(w_del_e),(double)W_DEL_E,0);
  ReadZeile( SW_NEW_G, "W_NEW_G", (void*)&(w_new_g),(double)W_NEW_G,0);
  ReadZeile( SW_RED_G, "W_RED_G", (void*)&(w_red_g),(double)W_RED_G,0);
  ReadZeile( SW_NEW_CP, "W_NEW_CP", (void*)&(w_new_cp),(double)W_NEW_CP,0);
  ReadZeile( SW_DEL_CP, "W_DEL_CP", (void*)&(w_del_cp),(double)W_DEL_CP,0);
  ReadZeile( SW_NEW_CG, "W_NEW_CG", (void*)&(w_new_cg),(double)W_NEW_CG,0);
  ReadZeile( SW_REDCOUNT, "W_REDCOUNT", (void*)&(w_redcount),(double)W_REDCOUNT,0);

  ReadZeile( SINCREASE_PERCENT, "INCREASE_PERCENT", (void*)&(increase_percent),(double)INCREASE_PERCENT,1);

  ReadZeile( SL_SIZE_R, "L_SIZE_R", (void*)&(l_size_r),(double)L_SIZE_R,0);
  ReadZeile( SL_SIZE_E, "L_SIZE_E", (void*)&(l_size_e),(double)L_SIZE_E,0);
  ReadZeile( SL_SIZE_G, "L_SIZE_G", (void*)&(l_size_g),(double)L_SIZE_G,0);
  ReadZeile( SL_SIZE_CP, "L_SIZE_CP", (void*)&(l_size_cp),(double)L_SIZE_CP,0);
  ReadZeile( SL_SIZE_CG, "L_SIZE_CG", (void*)&(l_size_cg),(double)L_SIZE_CG,0);
  ReadZeile( SL_NEW_R, "L_NEW_R", (void*)&(l_new_r),(double)L_NEW_R,0);
  ReadZeile( SL_RED_R, "L_RED_R", (void*)&(l_red_r),(double)L_RED_R,0);
  ReadZeile( SL_DEL_R, "L_DEL_R", (void*)&(l_del_r),(double)L_DEL_R,0);
  ReadZeile( SL_NEW_E, "L_NEW_E", (void*)&(l_new_e),(double)L_NEW_E,0);
  ReadZeile( SL_RED_E, "L_RED_E", (void*)&(l_red_e),(double)L_RED_E,0);
  ReadZeile( SL_DEL_E, "L_DEL_E", (void*)&(l_del_e),(double)L_DEL_E,0);
  ReadZeile( SL_NEW_G, "L_NEW_G", (void*)&(l_new_g),(double)L_NEW_G,0);
  ReadZeile( SL_RED_G, "L_RED_G", (void*)&(l_red_g),(double)L_RED_G,0);
  ReadZeile( SL_NEW_CP, "L_NEW_CP", (void*)&(l_new_cp),(double)L_NEW_CP,0);
  ReadZeile( SL_DEL_CP, "L_DEL_CP", (void*)&(l_del_cp),(double)L_DEL_CP,0);
  ReadZeile( SL_NEW_CG, "L_NEW_CG", (void*)&(l_new_cg),(double)L_NEW_CG,0);
  ReadZeile( SL_REDCOUNT, "L_REDCOUNT", (void*)&(l_redcount),(double)L_REDCOUNT,0);

  ReadZeile( SDECREASE_PERCENT, "DECREASE_PERCENT", (void*)&(decrease_percent),(double)DECREASE_PERCENT,1);
  ReadZeile( SVERLIERERMAXCYCL, "VERLIERERMAXCYCL", (void*)&(verlierermaxcycl),(double)VERLIERERMAXCYCL,0);
  ReadZeile( SVERLIERERGRENZE3, "VERLIERERGRENZE3", (void*)&(verlierergrenze3),(double)VERLIERERGRENZE3,1);
  ReadZeile( STEAMNOTCHANGED, "TEAMNOTCHANGED", (void*)&(teamnotchanged),(double)TEAMNOTCHANGED,0);

  ReadZeile( SHOST_LIMIT_DB, "HOST_LIMIT_DB", (void*)&(host_limit_db),(double)HOST_LIMIT_DB,0);
  ReadZeile( SCYCLE_DIFF_DB, "CYCLE_DIFF_DB", (void*)&(cycle_diff_db),(double)CYCLE_DIFF_DB,0);
  ReadZeile( SDOM_LIMIT_DB, "DOM_LIMIT_DB", (void*)&(dom_limit_db),(double)DOM_LIMIT_DB,0);

  ReadZeile( SHOST_LIMIT_CP, "HOST_LIMIT_CP", (void*)&(host_limit_cp),(double)HOST_LIMIT_CP,0);
  ReadZeile( SCYCLE_DIFF_CP, "CYCLE_DIFF_CP", (void*)&(cycle_diff_cp),(double)CYCLE_DIFF_CP,0);
  ReadZeile( SCRIT_P_LIMIT_CP, "CRIT_P_LIMIT_CP", (void*)&(crit_p_limit_cp),(double)CRIT_P_LIMIT_CP,0);

  ReadZeile( SHOST_LIMIT_CG, "HOST_LIMIT_CG", (void*)&(host_limit_cg),(double)HOST_LIMIT_CG,0);
  ReadZeile( SCYCLE_DIFF_CG, "CYCLE_DIFF_CG", (void*)&(cycle_diff_cg),(double)CYCLE_DIFF_CG,0);
  ReadZeile( SCRIT_G_LIMIT_CG, "CRIT_G_LIMIT_CG", (void*)&(crit_g_limit_cg),(double)CRIT_G_LIMIT_CG,0);

  ReadZeile( SDOMAIN_SUITED, "DOMAIN_SUITED", (void*)&(domain_suited),(double)DOMAIN_SUITED,0);
  ReadZeile( SWINNER_SUITED, "WINNER_SUITED", (void*)&(winner_suited),(double)WINNER_SUITED,0);
  ReadZeile( SJ_NOT_IN_TEAM, "J_NOT_IN_TEAM", (void*)&(j_not_in_team),(double)J_NOT_IN_TEAM,0);
  ReadZeile( SNOT_JUDGED, "NOT_JUDGED", (void*)&(not_judged),(double)NOT_JUDGED,0);

  ReadZeile( SMIN_INSPECTED_X, "MIN_INSPECTED_X", (void*)&(min_inspected_x),(double)MIN_INSPECTED_X,0);
  ReadZeile( SEXP_PART, "EXP_PART", (void*)&(exp_part),(double)EXP_PART,0);
  ReadZeile( SEXTRA_EXP1, "EXTRA_EXP1", (void*)&(extra_exp1),(double)EXTRA_EXP1,0);
  ReadZeile( SLOSER_PART, "LOSER_PART", (void*)&(loser_part),(double)LOSER_PART,0);
  ReadZeile( SEXTRA_EXP2, "EXTRA_EXP2", (void*)&(extra_exp2),(double)EXTRA_EXP2,0);
  ReadZeile( SLOSER_PERCENT, "LOSER_PERCENT", (void*)&(loser_percent),(double)LOSER_PERCENT,1);
  ReadZeile( SEXTRA_EXP3, "EXTRA_EXP3", (void*)&(extra_exp3),(double)EXTRA_EXP3,0);
  ReadZeile( SMIN_BETTER_EXP, "MIN_BETTER_EXP", (void*)&(min_better_exp),(double)MIN_BETTER_EXP,0);

  ReadZeile( SGOOD_DOM_EXP_KNO, "GOOD_DOM_EXP_KNO", (void*)&(good_dom_exp_kno),(double)GOOD_DOM_EXP_KNO,1);
  ReadZeile( SBAD_DOM_EXP_KNO, "BAD_DOM_EXP_KNO", (void*)&(bad_dom_exp_kno),(double)BAD_DOM_EXP_KNO,1);
  ReadZeile( SNO_DOM_EXP_KNO, "NO_DOM_EXP_KNO", (void*)&(no_dom_exp_kno),(double)NO_DOM_EXP_KNO,1);

  ReadZeile( SRELEVANTLASTCYCL, "RELEVANTLASTCYCL", (void*)&(relevantlastcycl),(double)RELEVANTLASTCYCL,0);
  ReadZeile( SMINIMUMOFACTIONS, "MINIMUMOFACTIONS", (void*)&(minimumofactions),(double)MINIMUMOFACTIONS,0);
  ReadZeile( SINSPECTEDACTIONS, "INSPECTEDACTIONS", (void*)&(inspectedactions),(double)INSPECTEDACTIONS,0);
  ReadZeile( SMAXDIFFERENCE, "MAXDIFFERENCE", (void*)&(maxdifference),(double)MAXDIFFERENCE,1);
  ReadZeile( SCONSTANTFACTOR, "CONSTANTFACTOR", (void*)&(constantfactor),(double)CONSTANTFACTOR,0);
  ReadZeile( STRENDEXPFACTOR, "TRENDEXPFACTOR", (void*)&(trendexpfactor),(double)TRENDEXPFACTOR,0);
  ReadZeile( SRELKZGFACTOR, "RELKZGFACTOR", (void*)&(relkzgfactor),(double)RELKZGFACTOR,0);
  ReadZeile( SPROOF_MID, "PROOF_MID", (void*)&(proof_mid),(double)PROOF_MID,0);

  if( timebase_angegeben ) /* TIMEBASE explizit in der Konfigurationsdatei angegeben */
  {
    ReadZeile( STIMEBASE, "TIMEBASE", (void*)&(timebase_dummy),(double)TIMEBASE,0);
  }
  else
  {
    ReadZeile( STIMEBASE, "TIMEBASE", (void*)&(timebase),(double)TIMEBASE,0);
  }
  ReadZeile( STP_PART, "TP_PART", (void*)&(tp_part),(double)TP_PART,0);
  ReadZeile( STIME_EXTRA, "TIME_EXTRA", (void*)&(time_extra),(double)TIME_EXTRA,1);
  ReadZeile( SGOOD_TEAM_T_FAC, "GOOD_TEAM_T_FAC", (void*)&(good_team_t_fac),(double)GOOD_TEAM_T_FAC,1);
  ReadZeile( SBAD_TEAM_T_FAC, "BAD_TEAM_T_FAC", (void*)&(bad_team_t_fac),(double)BAD_TEAM_T_FAC,1);
  ReadZeile( SUPPER_LIMIT1, "UPPER_LIMIT1", (void*)&(upper_limit1),(double)UPPER_LIMIT1,0);
  ReadZeile( SDOWN_LIMIT1, "DOWN_LIMIT1", (void*)&(down_limit1),(double)DOWN_LIMIT1,0);
  ReadZeile( SUPPER_LIMIT2, "UPPER_LIMIT2", (void*)&(upper_limit2),(double)UPPER_LIMIT2,0);
  ReadZeile( SDOWN_LIMIT2, "DOWN_LIMIT2", (void*)&(down_limit2),(double)DOWN_LIMIT2,0);
  ReadZeile( SGOALFAKTOR, "GOALFAKTOR", (void*)&(goalfaktor),(double)GOALFAKTOR,1);
 
  while (!EOI)
  {
    CheckError ( (GetSymbol ( f_descr, ident) != SCRSYM), "Dateiende erwartet." );
  }
 
  flush();
  CloseInput ( f_descr );
} /* Ende von ParsePlan */
