/*
//=============================================================================
//      Projekt:        TEAM-COMPLETION
//
//      Modul:          referee
//
//      Author:         Werner Pitz, 1991
//
//      Beschreibung:   In diesem Modul werden die Referees fuer die
//                      Parallelvervollstaendigung implementiert
//-----------------------------------------------------------------------------
//      $Log: referee.c,v $
//      Revision 0.3  1991/09/19  13:05:53  pitz
//      Extended Referee implementiert.
//
//      Revision 0.2  1991/08/19  09:50:14  pitz
//      Fehlermeldungen mit Dateiangabe.
//
//      Revision 0.1  1991/08/14  11:58:19  pitz
//      Bewertung von NoReferee auf -10000000 gesetzt.
//
//      Revision 0.0  1991/08/09  08:18:19  pitz
//      Initiale Version
//
//=============================================================================
*/

#include    <stdio.h>
#include    <sys/param.h>

#ifdef    ATARI
    #include    <stdlib.h>
    #include    <ext.h>
#endif

#include    "error.h"
#include    "systime.h"
#include    "vartree.h"
#include    "polynom.h"
#include    "term.h"
#include    "termpair.h"
#include    "subst.h"
#include    "order.h"
#include    "cpweight.h"
#include    "reduce.h"
#include    "pcl.h"
#include    "scanner.h"
#include    "complet.h"
#include    "database.h"
#include    "team.h"
#include    "referee.h"
#include    "parseref.h"
#include    "exp_def_next_cycletime.h"


/*****************************************************************************/
/*                                                                           */
/*                            Lokale Variablen                               */
/*                                                                           */
/*****************************************************************************/

static long     ref_red_count, ref_red_right, ref_red_left, ref_red_equ, ref_red_goal,
                ref_cp_count, ref_subsum, ref_equ_low, ref_rule_low, ref_goal_low;

static short    *parameter;

static short    MaxRefRule, MaxRefEqu, MaxRefGoal;

/*****************************************************************************/
/*                                                                           */
/*             Variablen, die auch extern verwendet werden                   */
/*                                                                           */
/*****************************************************************************/

report          referee_report;

RefFrame        RefGlobalInfo[MAXEXPERT];
RefFrame        DatabaseRef;

/*
//-----------------------------------------------------------------------------
//      lokale Funktiondefinitionen
//-----------------------------------------------------------------------------
*/


static long     NoReferee ( void );


static void     InsertRefRule ( termpair *rule );
static void     InsertRefEqu  ( termpair *rule );

static void     LTestRule ( termpair *ptr );
static void     LTestEqu  ( termpair *ptr );

static void     LastSelect   ( void );

static void     TestRule ( termpair *ptr );
static void     TestEqu  ( termpair *ptr );

static void     StatisticSelect   ( void );

static void     GoalTestSelect    ( void );

static void     TestDivRule ( termpair *ptr );
static void     TestDivEqu  ( termpair *ptr );

static long     StatisticReferee  ( void );
static long     ExtendedReferee   ( void );
static long     FeelGoodReferee   ( void );
static long     DivergenceReferee ( void );



/*
//-----------------------------------------------------------------------------
//  Funktion:       Referee
//
//  Parameter:      type    Auswahl des Referees
//                  param   Zugehoerige Parameter
//
//  Beschreibung:   Standard Referee
//-----------------------------------------------------------------------------
*/

long     Referee ( short rtype, short *rparam, short stype, short *sparam )
{
    parameter = rparam;

    referee_report.config   = OwnConfig.exp_nr;

    if( timeunit == SECONDS )
    {
      referee_report.runtime *= 1000;
    }
    referee_report.cputime  = cputime;

    switch (rtype)
    {
      case SR_STATISTIC : referee_report.result = StatisticReferee ();
                          break;
      case SR_EXTENDED  : referee_report.result = ExtendedReferee ();
                          break;
      case SR_FEELGOOD  : referee_report.result = FeelGoodReferee ();
                          break;
      case SR_DIVERGENCE: referee_report.result = DivergenceReferee ();
                          break;
      case SR_NONE      : referee_report.result = NoReferee ();
                          break;
      default           : Error ( __FILE__ ": "  "Referee", "Unbekannter Referee." );
                          break;
    }

    parameter = sparam;
    switch (stype)
    {
      case SS_LAST      : LastSelect ();
                          break;

      case SR_STATISTIC : StatisticSelect ();
                          break;

      case SX_GOALTEST  : GoalTestSelect ();
                          break;

      case SR_NONE      : referee_report.rule_count = 0;
                          referee_report.equ_count  = 0;
                          referee_report.goal_count = 0;
			  printf ( "\n\n" );
			  printf ( "-------------------------------------------------------------------\n" );
			  printf ( "    Kein Auswahlgutachter\n" );
			  printf ( "-------------------------------------------------------------------\n" );
			  printf ( "\n\n" );
			  flush  ();
                          break;

      default           : Error ( __FILE__ ": "  "Referee", "Unbekannter Auswahl-Referee." );
                          break;
    }

    return referee_report.result;
}


/*
//=============================================================================
//       Default-Referee (keine Auswertung)
//=============================================================================
*/

/*
//-----------------------------------------------------------------------------
//  Funktion:       NoReferee
//
//  Parameter:      -keine-
//
//  Beschreibung:   Standart Referee
//-----------------------------------------------------------------------------
*/

static long  NoReferee ( void )
{
   printf ( "\n\n" );
   printf ( "-------------------------------------------------------------------\n" );
   printf ( "    Kein Beurteilungsgutachter\n" );
   printf ( "-------------------------------------------------------------------\n" );
   printf ( "\n\n" );
   flush  ();
   
   referee_report.result     = -100000000;
   
   referee_report.master     = 0;
   
   /* Die Komponete referee_report.runtime wird bei einem REPRO-Lauf nicht */
   /* versorgt, deswegen erfolgt auch keine Berechnung der CPU-Auslatung   */
   if(!ReproMode)
   {
      printf ( "    CPU-Auslastung:   %3d %%\n\n", 
	      (cputime*100)/referee_report.runtime );
      printf("\n\n");
   }
   return referee_report.result;
}

/*****************************************************************************/
/*                                                                           */
/*                        Einfuege-Funktionen                                */
/*                                                                           */
/*****************************************************************************/

static void   InsertRefRule ( termpair *rule )
{
    short   i;

    if (referee_report.rule_count < MaxRefRule)
        referee_report.rule[referee_report.rule_count++] = rule;

    for (i = referee_report.rule_count-2; i >=0; i--)
    {
        if (referee_report.rule[i]->ref_weight > rule->ref_weight)
        {
            referee_report.rule[i+1] = rule;
            return;
        }
        else
        {
            referee_report.rule[i+1] = referee_report.rule[i];
        }
    }
    referee_report.rule[0] = rule;
}


static void   InsertRefEqu ( termpair *equ )
{
    short   i;

    if (referee_report.equ_count < MaxRefEqu)
        referee_report.equ[referee_report.equ_count++] = equ;

    for (i = referee_report.equ_count-2; i >=0; i--)
    {
        if (referee_report.equ[i]->ref_weight > equ->ref_weight)
        {
            referee_report.equ[i+1] = equ;
            return;
        }
        else
        {
            referee_report.equ[i+1] = referee_report.equ[i];
        }
    }
    referee_report.equ[0] = equ;
}


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  InsertRefGoal                                            */
/*                                                                           */
/*  Parameter    :  Pointer auf ein Ziel                                     */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Das uebergebene Ziel wird in die Liste der Ziele, die    */
/*                  der Gutachter an den Leiter uebergibt eingetragen.       */
/*                  Wenn das Feld ( Liste ) noch nicht ganz belegt ist, wird */
/*                  das Ziel direkt eingetragen, ansonsten wird es gemaess   */
/*                  seinem Gewicht an die Stelle eingefuegt. Dabei wird auf  */
/*                  jeden Fall das bisherige Ziel mit kleinstem Gewicht aus  */
/*                  der Liste geloescht.                                     */
/*                                                                           */
/*  Globale Var. :                                                           */
/*                                                                           */
/*  Externe Var. :                                                           */
/*                                                                           */
/*****************************************************************************/

static void   InsertRefGoal ( termpair *goal )
{
    short   i;

    if (referee_report.goal_count < MaxRefGoal)
        referee_report.goal[referee_report.goal_count++] = goal;

    for (i = referee_report.goal_count-2; i >=0; i--)
    {
        if (referee_report.goal[i]->ref_weight > goal->ref_weight)
        {
            referee_report.goal[i+1] = goal;
            return;
        }
        else
        {
            referee_report.goal[i+1] = referee_report.goal[i];
        }
    }
    referee_report.goal[0] = goal;
}


/*
//=============================================================================
//      Last Referee
//=============================================================================
*/

static void   LTestRule ( termpair *rule )
{
    if (!MaxRefRule)
        return;

    if (State (rule) & INITIAL)
        return;

    rule->ref_weight = rule->count;

    if (   (referee_report.rule_count < MaxRefRule)
        || (referee_report.rule[MaxRefRule-1]->ref_weight < rule->ref_weight))
        InsertRefRule ( rule );
}


static void   LTestEqu ( termpair *equ )
{
    if (!MaxRefEqu)
        return;

    if (State (equ) & INITIAL)
        return;

    equ->ref_weight =  equ->count;

    if (   (referee_report.equ_count < MaxRefEqu)
        || (referee_report.equ[MaxRefEqu-1]->ref_weight < equ->ref_weight))
        InsertRefEqu ( equ );
}

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  LTestGoal                                                */
/*                                                                           */
/*  Parameter    :  Pointer auf ein Ziel                                     */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Es wird getestet, ob das uebergebene Ziel an den Leiter  */
/*                  uebergeben wird. Dies ist der Fall, wenn es in diesem    */
/*                  Zyklus erzeugt wurde ( erste if-Abfrage ) und entweder   */
/*                  die maximale Anzahl von Zielen, die uebergeben werden    */
/*                  darf, noch nicht erreicht wurde oder das Ziel ein klei-  */
/*                  neres Gewicht hat, d.h. hier eine groessere Nummer hat,  */
/*                  also spaeter erzeugt wurde. Dies ist ja die Strategie    */
/*                  des Last - Gutachters.                                   */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

static void LTestGoal ( termpair *goal )
{
    if ( !MaxRefGoal )
    {
      return;
    }

    if ( State ( goal ) & INITIAL )
    {
      return;
    }

    goal->ref_weight = goal->count;

    if ( ( referee_report.goal_count < MaxRefGoal ) ||
         ( referee_report.goal[MaxRefGoal-1]->ref_weight < goal->ref_weight ) )
    {
      InsertRefGoal ( goal );
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       LastSelect
//
//  Parameter:      -keine-
//
//  Beschreibung:   Auswahl-Referee, der die letzten Regeln weiterleitet.
//-----------------------------------------------------------------------------
*/

static void  LastSelect ( void )
{
    short   i;

    MaxRefRule = parameter[REF_MAX_RULE];
    MaxRefEqu  = parameter[REF_MAX_EQU];
    MaxRefGoal = parameter[REF_MAX_GOAL];

    referee_report.rule_count = 0;
    referee_report.equ_count  = 0;
    referee_report.goal_count  = 0;

    if (MaxRefRule)
    {
      ForAllRulesDo ( LTestRule );
    }

    if (MaxRefEqu)
    {
      ForAllEquDo   ( LTestEqu  );
    }

    if (MaxRefGoal)
    {
      ForAllEquDo   ( LTestEqu  );
    }

    printf ( "Die letzten %d Regeln.\n", referee_report.rule_count );
    for (i = 0; i < referee_report.rule_count; i++)
    {
        printf ( "(%4ld)  ", referee_report.rule[i]->ref_weight );
        printtpair ( referee_report.rule[i] );
    }
    printf ( "\n" );

    printf ( "Die letzten %d Gleichungen.\n", referee_report.equ_count );
    for (i = 0; i < referee_report.equ_count; i++)
    {
        printf ( "(%4ld)  ", referee_report.equ[i]->ref_weight );
        printtpair ( referee_report.equ[i] );
    }
    printf ( "\n" );

    if( Paramodulation )
    {
      printf ( "Die letzten %d Ziele.\n", referee_report.goal_count );
      for (i = 0; i < referee_report.goal_count; i++)
      {
	  printf ( "(%4ld)  ", referee_report.goal[i]->ref_weight );
	  printtpair ( referee_report.goal[i] );
      }
      printf ( "\n" );
    }

    printf ( "-------------------------------------------------------------------\n" );

    flush  ();
}

/*
//=============================================================================
//      Statistischer Referee
//=============================================================================
*/

static void   TestRule ( termpair *rule )
{
    if (!MaxRefRule)
        return;

    if (State (rule) & INITIAL)
        return;

    rule->ref_weight =   (ref_red_count * rule->ref_red_count)
                       + (ref_red_right * rule->ref_red_right)
                       + (ref_red_left  * rule->ref_red_left )
                       + (ref_red_equ   * rule->ref_red_equ  )
                       + (ref_red_goal  * rule->ref_red_goal )
                       + (ref_cp_count  * rule->ref_cp_count );

    if (rule->ref_weight < ref_rule_low)
        return;

    if (   (referee_report.rule_count < MaxRefRule) 
        || (referee_report.rule[MaxRefRule-1]->ref_weight < rule->ref_weight))
        InsertRefRule ( rule );
}


static void   TestEqu ( termpair *equ )
{
    if (!MaxRefEqu)
        return;

    if (State (equ) & INITIAL)
        return;

    equ->ref_weight =   (ref_red_count * equ->ref_red_count)
                      + (ref_red_right * equ->ref_red_right)
                      + (ref_red_left  * equ->ref_red_left )
                      + (ref_red_equ   * equ->ref_red_equ  )
                      + (ref_red_goal  * equ->ref_red_goal )
                      + (ref_cp_count  * equ->ref_cp_count )
                      + (ref_subsum    * equ->ref_subsum   );

    if (equ->ref_weight < ref_equ_low)
        return;

    if (   (referee_report.equ_count < MaxRefEqu) 
        || (referee_report.equ[MaxRefEqu-1]->ref_weight < equ->ref_weight))
        InsertRefEqu ( equ );
}


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  TestGoal                                                 */
/*                                                                           */
/*  Parameter    :  Pointer auf ein Ziel                                     */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Es wird getestet, ob das uebergebene Ziel an den Leiter  */
/*                  uebergeben wird. Dies ist der Fall, wenn es in diesem    */
/*                  Zyklus erzeugt wurde ( erste if-Abfrage ) und entweder   */
/*                  die maximale Anzahl von Zielen, die uebergeben werden    */
/*                  darf, noch nicht erreicht wurde oder das Ziel ein groes- */
/*                  seres Gewicht hat, d.h. hier bzgl. der Funktion          */
/*                  max_unif besser bewertet wird.                           */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

static void TestGoal ( termpair *goal )
{
  if (!MaxRefGoal)
  {
    return;
  }

  if (State (goal) & INITIAL)
    return;

  goal->ref_weight = max_unif( goal->left, goal->right );

  if ( goal->ref_weight < ref_goal_low )
    return;

  if ( ( referee_report.goal_count < MaxRefGoal ) ||
       ( referee_report.goal[MaxRefGoal-1]->ref_weight < goal->ref_weight ) )
    InsertRefGoal( goal );
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       StatisticSelect
//
//  Parameter:      -keine-
//
//  Beschreibung:   Statistischer Auswahl-Referee
//-----------------------------------------------------------------------------
*/

static void  StatisticSelect ( void )
{
    short   i;

    MaxRefRule = parameter[REF_MAX_RULE];
    MaxRefEqu  = parameter[REF_MAX_EQU];
    MaxRefGoal = parameter[REF_MAX_GOAL];

    referee_report.rule_count = 0;
    referee_report.equ_count  = 0;
    referee_report.goal_count = 0;

    ref_red_count = parameter[REF_RED_COUNT];
    ref_red_right = parameter[REF_RED_RIGHT];
    ref_red_left  = parameter[REF_RED_LEFT ];
    ref_red_equ   = parameter[REF_RED_EQU  ];
    ref_red_goal  = parameter[REF_RED_GOAL ];
    ref_cp_count  = parameter[REF_CP_COUNT ];
    ref_subsum    = parameter[REF_SUBSUM   ];
    ref_rule_low  = parameter[REF_RULE_LOW ];
    ref_equ_low   = parameter[REF_EQU_LOW  ];
    ref_goal_low  = parameter[REF_GOAL_LOW ];

    if (MaxRefRule)
    {
      ForAllRulesDo ( TestRule );
    }

    if (MaxRefEqu)
    {
      ForAllEquDo   ( TestEqu  );
    }

    if (MaxRefGoal)
    {
      ForAllEquDo   ( TestEqu  );
    }

    if (DemoMode)
        return;

    printf ( "%d Regeln gewonnen.\n", referee_report.rule_count );
    for (i = 0; i < referee_report.rule_count; i++)
    {
        printf ( "(%4ld)  ", referee_report.rule[i]->ref_weight );
        printtpair ( referee_report.rule[i] );
    }
    printf ( "\n" );

    printf ( "%d Gleichungen gewonnen.\n", referee_report.equ_count );
    for (i = 0; i < referee_report.equ_count; i++)
    {
        printf ( "(%4ld)  ", referee_report.equ[i]->ref_weight );
        printtpair ( referee_report.equ[i] );
    }
    printf ( "\n" );

    if( Paramodulation )
    {
      printf ( "%d Ziele gewonnen.\n", referee_report.goal_count );
      for (i = 0; i < referee_report.goal_count; i++)
      {
	  printf ( "(%4ld)  ", referee_report.goal[i]->ref_weight );
	  printtpair ( referee_report.goal[i] );
      }
      printf ( "\n" );
    }

    printf ( "-------------------------------------------------------------------\n" );
    flush  ();
}



/*
//-----------------------------------------------------------------------------
//  Funktion:       GoalTestSelect
//
//  Parameter:      -keine-
//
//  Beschreibung:   ...
//-----------------------------------------------------------------------------
*/

static void  GoalTestSelect ( void )
{
    short   i;

    MaxRefRule = parameter[REF_MAX_RULE];
    MaxRefEqu  = parameter[REF_MAX_EQU];
    referee_report.rule_count = 0;
    referee_report.equ_count  = 0;
/*
    ForAllRulesDo ( TestRule );
    ForAllEquDo   ( TestEqu  );
*/
    printf ( "%d Regeln gewonnen.\n", referee_report.rule_count );
    for (i = 0; i < referee_report.rule_count; i++)
    {
        printf ( "(%4ld)  ", referee_report.rule[i]->ref_weight );
        printtpair ( referee_report.rule[i] );
    }

    printf ( "\n" );
    printf ( "%d Gleichungen gewonnen.\n", referee_report.equ_count );
    for (i = 0; i < referee_report.equ_count; i++)
    {
        printf ( "(%4ld)  ", referee_report.equ[i]->ref_weight );
        printtpair ( referee_report.equ[i] );
    }
    printf ( "\n" );
    printf ( "-------------------------------------------------------------------\n" );
    flush  ();
}




/*
//-----------------------------------------------------------------------------
//  Funktion:       StatisticReferee
//
//  Parameter:      -keine-
//
//  Beschreibung:   Standart Referee
//-----------------------------------------------------------------------------
*/

static long  StatisticReferee ( void )
{
    long    result;

    printf ( "\n\n" );
    printf ( "-------------------------------------------------------------------\n" );
    printf ( "    Statistischer Referee\n" );
    printf ( "-------------------------------------------------------------------\n" );
    printf ( "\n\n" );
    ClearCPs ( false );
    if (!DemoMode)
    {
        printf ( "    Werte des aktuellen Systems:\n" );
        printf ( "    ----------------------------\n\n" );
        printf ( "    Anzahl der Regeln:            %6ld * %6d = %10ld\n", 
                 SizeOf (SetOfRules[0]),
                 parameter[REF_SIZE_R],
                 (SizeOf (SetOfRules[0]) * parameter[REF_SIZE_R]) );
        printf ( "    Anzahl der Gleichungen:       %6ld * %6d = %10ld\n", 
                 SizeOf (SetOfEquations), 
                 parameter[REF_SIZE_E],
                 (SizeOf (SetOfEquations) * parameter[REF_SIZE_E]) );
        printf ( "    Anzahl der kritischen Paare:  %6ld * %6d = %10ld\n", 
                 SizeOf (SetOfCriticalPairs), 
                 parameter[REF_SIZE_CP],
                 (SizeOf (SetOfCriticalPairs) * parameter[REF_SIZE_CP]) );
        printf ( "    Anzahl der Reduktionen:       %6ld * %6d = %10ld\n", 
                 RedCount,
                 parameter[REF_REDCOUNT],
                 (RedCount * parameter[REF_REDCOUNT]) );
        printf ( "\n" );
    }

    result =   (SizeOf (SetOfRules[0])          * parameter[REF_SIZE_R]   )
             + (SizeOf (SetOfEquations)         * parameter[REF_SIZE_E]   )
             + (SizeOf (SetOfCriticalPairs)     * parameter[REF_SIZE_CP]  )
             + (RedCount                        * parameter[REF_REDCOUNT] );

    printf ( "    Bewertung:                                      %10ld\n",
             (long)result );
    printf ( "\n\n" );
/* Die Komponete referee_report.runtime wird bei einem REPRO-Lauf nicht */
/* versorgt, deswegen erfolgt auch keine Berechnung der CPU-Auslatung   */
    if(!ReproMode)
    {
       printf ( "    CPU-Auslastung:   %3d %%\n", 
	       (cputime*100)/referee_report.runtime );
       
       printf ( "\n\n" );
    }

    referee_report.master = 1;
    if (!parameter[REF_NOMASTER])
    {
        printf ( "Dieser Experte kann nicht Master werden !\n\n" );
        referee_report.master = 0;
    }

    printf ( "\n" );
    printf ( "-------------------------------------------------------------------\n" );
    flush  ();

    return result;
}



/*
//-----------------------------------------------------------------------------
//  Funktion:       ExtendedReferee
//
//  Parameter:      -keine-
//
//  Beschreibung:   Erweiterter Referee
//-----------------------------------------------------------------------------
*/

static long  ExtendedReferee ( void )
{
    long    result;

    printf ( "\n\n" );
    printf ( "-------------------------------------------------------------------\n" );
    printf ( "    Erweiterter statistischer Referee\n" );
    printf ( "-------------------------------------------------------------------\n" );
    printf ( "\n\n" );
    ClearCPs ( false );
    if (!DemoMode)
    {
        printf ( "    Werte des aktuellen Systems:\n" );
        printf ( "    ----------------------------\n\n" );
        printf ( "    Anzahl der Regeln:                 %6ld * %6d = %10ld\n", 
                 SizeOf (SetOfRules[0]),
                 parameter[REF_SIZE_R],
                 (SizeOf (SetOfRules[0]) * parameter[REF_SIZE_R]) );
        printf ( "    Anzahl der Gleichungen:            %6ld * %6d = %10ld\n", 
                 SizeOf (SetOfEquations), 
                 parameter[REF_SIZE_E],
                 (SizeOf (SetOfEquations) * parameter[REF_SIZE_E]) );
        printf ( "    Anzahl der kritischen Paare:       %6ld * %6d = %10ld\n", 
                 SizeOf (SetOfCriticalPairs), 
                 parameter[REF_SIZE_CP],
                 (SizeOf (SetOfCriticalPairs) * parameter[REF_SIZE_CP]) );
    
        printf ( "    Anzahl der neuen Regeln:           %6ld * %6d = %10ld\n", 
                 ref_new_rules,
                 parameter[REF_NEW_R],
                 (ref_new_rules * parameter[REF_NEW_R]) );
        printf ( "    Anzahl der reduzierten Regeln:     %6ld * %6d = %10ld\n", 
                 ref_red_rules,
                 parameter[REF_RED_R],
                 (ref_red_rules * parameter[REF_RED_R]) );
        printf ( "    Anzahl der geloeschten Regeln:     %6ld * %6d = %10ld\n", 
                 ref_del_rules,
                 parameter[REF_DEL_R],
                 (ref_del_rules * parameter[REF_DEL_R]) );
    
        printf ( "    Anzahl der neuen Gleichungen:      %6ld * %6d = %10ld\n", 
                 ref_new_equations,
                 parameter[REF_NEW_E],
                 (ref_new_equations * parameter[REF_NEW_E]) );
        printf ( "    Anzahl der reduzierten Gleichungen:%6ld * %6d = %10ld\n", 
                 ref_red_equations,
                 parameter[REF_RED_E],
                 (ref_red_equations * parameter[REF_RED_E]) );
        printf ( "    Anzahl der geloeschten Gleichungen:%6ld * %6d = %10ld\n", 
                 ref_del_equations,
                 parameter[REF_DEL_E],
                 (ref_del_equations * parameter[REF_DEL_E]) );
    
        printf ( "    Anzahl der neuen krit. Paare:      %6ld * %6d = %10ld\n", 
                 ref_new_cp,
                 parameter[REF_NEW_CP],
                 (ref_new_cp * parameter[REF_NEW_CP]) );
    
        printf ( "    Anzahl der geloeschten krit. Paare:%6ld * %6d = %10ld\n", 
                 ref_del_cp,
                 parameter[REF_DEL_CP],
                 (ref_del_cp * parameter[REF_DEL_CP]) );
    
        printf ( "    Anzahl der Reduktionen:            %6ld * %6d = %10ld\n", 
                 RedCount,
                 parameter[REF_REDCOUNT],
                 (RedCount * parameter[REF_REDCOUNT]) );
        printf ( "\n" );
    }

    result =   (SizeOf (SetOfRules[0])          * parameter[REF_SIZE_R]   )
             + (SizeOf (SetOfEquations)         * parameter[REF_SIZE_E]   )
             + (SizeOf (SetOfCriticalPairs)     * parameter[REF_SIZE_CP]  )
             + (ref_new_rules                   * parameter[REF_NEW_R]    ) 
             + (ref_red_rules                   * parameter[REF_RED_R]    ) 
             + (ref_del_rules                   * parameter[REF_DEL_R]    )
             + (ref_new_equations               * parameter[REF_NEW_E]    )
             + (ref_red_equations               * parameter[REF_RED_E]    )
             + (ref_del_equations               * parameter[REF_DEL_E]    )
             + (ref_new_cp                      * parameter[REF_NEW_CP]   )
             + (ref_del_cp                      * parameter[REF_DEL_CP]   )
             + (RedCount                        * parameter[REF_REDCOUNT] );

    printf ( "    Bewertung:                                           %10ld\n",
             (long)result );
    printf ( "\n\n" );

/* Die Komponete referee_report.runtime wird bei einem REPRO-Lauf nicht */
/* versorgt, deswegen erfolgt auch keine Berechnung der CPU-Auslatung   */
    
    if(!ReproMode)
    {
       printf ( "    CPU-Auslastung:   %3d %%\n", 
	       (cputime*100)/referee_report.runtime );
       
       printf ( "\n\n" );
    }
    
    referee_report.master = 1;
    if (!parameter[REF_NOMASTER])
    {
        printf ( "Dieser Experte kann nicht Master werden !\n\n" );
        referee_report.master = 0;
    }

    printf ( "\n" );
    printf ( "-------------------------------------------------------------------\n" );
    flush  ();

    return result;
}



/*
//-----------------------------------------------------------------------------
//  Funktion:       FeelGoodReferee
//
//  Parameter:      -keine-
//
//  Beschreibung:   Bewertung des Systems anhand der letzten und der
//                  naechsten kritischen Paare
//-----------------------------------------------------------------------------
*/

static long  FeelGoodReferee ( void )
{
    long    result;
    long    NextCPWeight = SumWeightCP ( parameter[2] );
    long    globlweight, pastweight, futureweight;

    printf ( "\n\n" );
    printf ( "-------------------------------------------------------------------\n" );
    printf ( "    FEELGOOD Gutachter\n" );
    printf ( "-------------------------------------------------------------------\n" );
    printf ( "\n\n" );
    ClearCPs ( false );
    if (!DemoMode)
    {
        printf ( "    Werte des aktuellen Systems:\n" );
        printf ( "    ----------------------------\n\n" );
        printf ( "    Bewertung der entw. kritischen Paare: (%4d)   = %10ld   (Faktor: %4ld)\n",
                 ref_devel_cps, ref_weight_cps, parameter[4] );
        printf ( "    Bewertung der letzten %3d kritischen Paare:    = %10f   (Faktor: %4ld)\n", 
                 parameter[0], LastCPWeight, parameter[1] );
        printf ( "    Bewertung der naechsten %3d kritischen Paare:  = %10ld   (Faktor: %4ld)\n\n", 
                 parameter[2], NextCPWeight, parameter[3] );
    }

    globlweight = (ref_devel_cps) ? (parameter[4] * ref_weight_cps) / ref_devel_cps
                                  : 0;
    
    pastweight  = (parameter[0])  ? (parameter[1] * LastCPWeight) / parameter[0]
                                  : 0;
    
    futureweight  = (parameter[2])  ? (parameter[3] * NextCPWeight) / parameter[2]
                                    : 0;

    if (!DemoMode)
    {
        printf ( "    Daraus ergeben sich folgende Gewichte:\n\n" );
        printf ( "                              globales Gewicht:    = %10ld\n", 
                                                                       globlweight );
        printf ( "                          Vergangenheit(lokal):    = %10ld\n", 
                                                                       pastweight );
        printf ( "                                Zukunft(lokal):    = %10ld\n\n", 
                                                                       futureweight );
    }

    result =   ((pastweight) ? (globlweight/pastweight) : 0)
             + ((futureweight) ? (globlweight/futureweight) : 0);

    printf ( "    Bewertung ueber alles:                           %10ld\n",
                                                                   result );
    printf ( "\n\n" );

/* Die Komponete referee_report.runtime wird bei einem REPRO-Lauf nicht */
/* versorgt, deswegen erfolgt auch keine Berechnung der CPU-Auslatung   */
    
    if(!ReproMode)
    {
       printf ( "    CPU-Auslastung:   %3d %%\n", 
	       (cputime*100)/referee_report.runtime );
       
       printf ( "\n\n" );
    }

    referee_report.master = 1;
    if (!parameter[REF_NOMASTER])
    {
        printf ( "Dieser Experte kann nicht Master werden !\n\n" );
        referee_report.master = 0;
    }

    printf ( "\n" );
    printf ( "-------------------------------------------------------------------\n" );
    flush  ();

    return result;
}


/*
//-----------------------------------------------------------------------------
*/

static void   TestDivRule ( termpair *rule )
{
    if (!MaxRefEqu)
        return;

    if (State (rule) & INITIAL)
        return;

    rule->ref_weight =   (parameter[0] * rule->ref_red_count)
                       + (parameter[1] * rule->ref_red_goal )
                       + (CPWeight (rule));

    if (rule->ref_weight < parameter[REF_RULE_LOW])
        return;

    if (   (referee_report.rule_count < MaxRefRule) 
        || (referee_report.rule[MaxRefRule-1]->ref_weight < rule->ref_weight))
        InsertRefRule ( rule );
}


/*
//-----------------------------------------------------------------------------
*/

static void   TestDivEqu ( termpair *equ )
{
    if (!MaxRefEqu)
        return;

    if (State (equ) & INITIAL)
        return;

    equ->ref_weight  =   (parameter[0] * equ->ref_red_count)
                       + (parameter[1] * equ->ref_red_goal )
                       + (CPWeight (equ) );

    if (equ->ref_weight < parameter[REF_EQU_LOW])
        return;

    if (   (referee_report.equ_count < MaxRefRule) 
        || (referee_report.equ[MaxRefRule-1]->ref_weight < equ->ref_weight))
        InsertRefEqu ( equ );
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       DivergenceReferee
//
//  Parameter:      -keine-
//
//  Beschreibung:   Bewertung des Systems anhand der letzten und der
//                  naechsten kritischen Paare
//-----------------------------------------------------------------------------
*/

static long  DivergenceReferee ( void )
{
    printf ( "\n\n" );
    printf ( "-------------------------------------------------------------------\n" );
    printf ( "    Divergenz Referee\n" );
    printf ( "-------------------------------------------------------------------\n" );
    printf ( "\n\n" );
    ClearCPs ( false );
    printf ( "    Auswahl der am weitesten entwickelten Regeln und Gleichungen !\n\n" );
    referee_report.master = 0;

    referee_report.master = 1;
    if (!parameter[REF_NOMASTER])
    printf ( "\n" );
    printf ( "-------------------------------------------------------------------\n" );
    flush  ();

    return 0;
}
