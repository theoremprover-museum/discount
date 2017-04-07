/*
//=============================================================================
//      Projekt:        TEAM-COMPLETION
//
//      Modul:          complet
//
//      Author:         Werner Pitz, 1991
//
//      Beschreibung:   Vervollstaendigung
//-----------------------------------------------------------------------------
//      $Log: complet.c,v $
//
//      Revision 0.7 1994/01/18   15:01:00  stschulz
//      Per Hand eingetragen...das Changelog ist wahrscheinlich wertlos. Neben 
//      einer voelligen Umarbeitung des Systems haben wir heute die Reihenfolge
//      von Termination unfd Log-File-Schreiben vertauscht.
//
//      Revision 0.6  1992/01/28  14:26:35  pitz
//      Ergaenzung ym TIMETAG
//
//      Revision 0.5  1991/09/27  08:13:24  pitz
//      RulesOnly und EquationsOnly bei Parallelvervollstaendigung implementiert.
//
//      Revision 0.4  1991/09/19  13:03:56  pitz
//      Erfassung statistischer Informationen mittels:
//        ref_new_rules,      ref_red_rules,      ref_del_rules
//        ref_new_equations,  ref_red_equations,  ref_del_equations
//        ref_new_cp,                             ref_del_cp
//
//      Revision 0.3  1991/09/11  08:36:28  pitz
//      Meldung 'Spezielle Regel/Glsichung' bei SetOfSupport-Experten.
//
//      Revision 0.2  1991/08/26  13:43:37  pitz
//      ParallelCheckCPPs: Reduktionsspezialist wie ParallelCheckCPs.
//                         Allerdings wird der Startpunkt relativ festgelegt.
//
//      Revision 0.1  1991/08/19  09:49:20  pitz
//      Fehlermeldungen mit Dateiangabe.
//
//      Revision 0.0  1991/08/09  08:18:19  pitz
//      Initiale Version
//
//=============================================================================
*/

#include    <sys/time.h>
#include    <sys/resource.h>

#include    "memory.h"
#include    "list.h"
#include    "reduce.h"
#include    "pcl.h"
#include    "buildcp.h"
#include    "complet.h"
#include    "systime.h"
#include    "database.h"
#include    "history.h"
#include    "team.h"


/*
//-----------------------------------------------------------------------------
//      aktuelle Einstellung
//-----------------------------------------------------------------------------
*/

/* Merged REPRO version and normal version.... */

int             Cycles;                /* Zyklen f"ur den Repro-Lauf  */
int             *FirstStepsPerCycle = NULL; 
int             *SecondStepsPerCycle = NULL; 
int             *ConfigPerCycle = NULL;
int             *MasterPerCycle = NULL;
bool            ReproMode = false;

bool            MemStatistic    = false;

bool            Proofmode       = false;
bool            CancelActive    = false;
bool            Paramodulation  = false;
bool            KritKapur       = true;
/* ParaCount regelt das Verhaeltnis der Bearbeitung von kritischen Paaren     */
/* und kritischen Zielen. ParaCount wird in der Problemspezifikation angegeben*/
/* Ist ParaCount positiv, so werden stets immer soviele kritische Paare bear- */
/* beitet, bevor EIN kritisches Ziel bearbeitet wird. Ist ParaCount negativ,  */
/* so werden nach jedem kritischen Paar erst entsprechend viele kritischen    */
/* Ziele bearbeitet, bevor das naechste kritische Paar bearbeitet wird.       */
short           ParaCount       = 0;
bool            RedInst         = false;
bool            DemoMode        = false;
bool            ProtocolMode    = true;
int             FirstStepsDone;    /* Zahler fuer Protokoll */
int             SecondStepsDone;   /* Zahler fuer Protokoll */
StateType       WhereDone       = T_UNKNOWN;



bool            RulesOnly       = false;
bool            EquationsOnly   = false;

bool            ReduceCPs       = true;         /* Parameter fuer       */
bool            SubsumCPs       = false;        /* ParallelCheckCPs     */
bool            DoubleCPs       = false;
long            StartCP         = 0;

bool            ReduceCGs       = true;         /* Parameter fuer       */
bool            SubsumCGs       = false;        /* ParallelCheckCGs     */
bool            DoubleCGs       = false;
long            StartCG         = 0;

double          LastCPWeight    = 0;
short           LastCPCount     = 0;

long            ref_devel_cps   = 0;            /* Anzahl der entwickel-*/
                                                /* ten kritischen Paare */
long            ref_weight_cps  = 0;            /* Gewichtssumme der    */
                                                /* entw. krit. Paare    */

long            ref_new_rules       = 0;
long            ref_red_rules       = 0;
long            ref_del_rules       = 0;
long            ref_new_equations   = 0;
long            ref_red_equations   = 0;
long            ref_del_equations   = 0;
long            ref_new_cp          = 0;
long            ref_del_cp          = 0;


void            (*Terminate)(void) = NULL;

/* Enthaelt die gesamte Zeit, die fuer die Planung verwendet wurde!          */
long            GesamtPlanungszeit = 0;

/* Zeit die das System am Laufen ist */
long    itime;

/*
//-----------------------------------------------------------------------------
//      Modulinterne Definitionen
//-----------------------------------------------------------------------------
*/

#ifdef TIMETAG
  static  long    htime;
#endif

static void RedGoalR    ( termpair *ptr );
static void RedGoalE    ( termpair *ptr );

static void RedRuleR    ( termpair *ptr );
static void RedEquR     ( termpair *ptr );

static bool testnewgoal ( termpair *newgoal );
static bool testredgoal ( termpair *redgoal );

static void SubsumEqu   ( termpair *ptr );

static void RedRuleE    ( termpair *ptr );
static void RedEquE     ( termpair *ptr );

static void delrule     ( termpair *ptr);
static void delequ      ( termpair *ptr);
static void delcp       ( termpair *ptr);
static void delgoal     ( termpair *ptr);
static void delcg       ( termpair *ptr);


/*
//=============================================================================
//      Meldungen des Systems bei Aenderungen an Regel und Gleichungen
//=============================================================================
*/

static  short   step            = 0;
static  char    *stepchar       = "\\|/-";
static  char    *starttext      = "arbeite ... ";
static  char    *stoptext       = "\010\010\010\010\010\010\010\010\010\010\010\010\010"
                                  "             "
                                  "\010\010\010\010\010\010\010\010\010\010\010\010\010";

#define START()                                                         \
    printf ( starttext )                         

#define STOP()                                                          \
    printf ( stoptext )

#define STEP()                                                          \
    printf ( "%c%c", stepchar[step++ % 4], BS )                         


#define DEMOINFO()                                                           \
{                                                                               \
    printf ( "%4d kritische Paare entwickelt.\n",       ref_devel_cps );        \
    printf ( "%4d neue Regeln.\n",                      ref_new_rules );        \
    printf ( "%4d Regeln reduziert.\n",                 ref_red_rules );        \
    printf ( "%4d Regeln geloescht.\n",                 ref_del_rules );        \
    printf ( "%4d neue Gleichungen.\n",                 ref_new_equations );    \
    printf ( "%4d Gleichungen reduziert.\n",            ref_red_equations );    \
    printf ( "%4d Gleichungen geloescht.\n",            ref_del_equations );    \
    printf ( "%4d neue kritische Paare.\n",             ref_new_cp );           \
    printf ( "%4d geloeschte kritische Paare.\n\n",     ref_del_cp );           \
}


#ifdef TIMETAG
    #define timetag()                                                   \
        htime = systime()-itime;                                        \
        printf ( "%5ld.%03ld: ", htime/1000, htime%1000 );
#else
    #define timetag() 
#endif

#define RedRule( rule )                                                 \
{                                                                       \
    if (DemoMode)                                                       \
        STEP();                                                         \
    else                                                                \
    {                                                                   \
        timetag();                                                      \
        printf     ( "Reduziere Regel:    %4ld\n", rule->number );      \
    }                                                                   \
    flush ();                                                           \
    rule->weight = (RedInst) ? -30000                                   \
                             : CPWeight (rule);                         \
    PCL_SAVE   ( rule );                                                \
    InsertCP   ( rule );                                                \
    ref_red_rules++;                                                    \
}


#define DelRule( rule )                                                 \
{                                                                       \
    if (DemoMode)                                                       \
        STEP();                                                         \
    else                                                                \
    {                                                                   \
        timetag();                                                      \
        printf     ( "Loesche Regel:      %4ld\n", rule->number );      \
    }                                                                   \
    flush ();                                                           \
    deleteterm ( rule->left );                                          \
    deleteterm ( rule->right );                                         \
    deletepair ( rule );                                                \
    ref_del_rules++;                                                    \
}


#define RedEquation( equ )                                              \
{                                                                       \
    if (DemoMode)                                                       \
        STEP();                                                         \
    else                                                                \
    {                                                                   \
        timetag();                                                      \
        printf     ( "Reduziere Gleichung:%4ld\n", equ->number );       \
    }                                                                   \
    flush ();                                                           \
    equ->weight = (RedInst) ? -30000                                    \
                            : CPWeight (equ);                           \
    PCL_SAVE   ( equ );                                                 \
    InsertCP   ( equ );                                                 \
    ref_red_equations++;                                                \
}


#define DelEquation( equ )                                              \
{                                                                       \
    if (DemoMode)                                                       \
        STEP();                                                         \
    else                                                                \
    {                                                                   \
        timetag();                                                      \
        printf     ( "Loesche Gleichung:  %4ld\n", equ->number );       \
    }                                                                   \
    flush ();                                                           \
    deleteterm ( equ->left );                                           \
    deleteterm ( equ->right );                                          \
    deletepair ( equ );                                                 \
    ref_del_equations++;                                                \
}


#define RedGoal( goal )                                                 \
{                                                                       \
    if (DemoMode)                                                       \
        STOP();                                                         \
    timetag();                                                          \
    printf     ( "Vereinfache Ziel:   %4ld  ", goal->number );          \
    printtpair ( goal );                                                \
    flush ();                                                           \
    if (DemoMode)                                                       \
        START();                                                        \
}


#define ProvedBy( equ )                                                 \
{                                                                       \
    if (DemoMode)                                                       \
        STOP();                                                         \
    timetag();                                                          \
    printf     ( "Dies gilt da:       %4ld  ", equ->number );           \
    printtpair ( equ );                                                 \
    flush ();                                                           \
    Proved     ();                                                      \
}



#define Paramodul( goal )                                               \
{                                                                       \
    if (DemoMode)                                                       \
        STEP();                                                         \
    else                                                                \
    {                                                                   \
        timetag();                                                      \
        printf     ( "Paramoduliere Ziel: %4ld  ", goal->number );      \
        printtpair ( goal );                                            \
    }                                                                   \
    flush ();                                                           \
}



/*
//-----------------------------------------------------------------------------
//  Funktion:       RedGoalR ( termpair *ptr )
//
//  Parameter:      ptr     Ziel
//
//  Beschreibung:   Versucht Goal mit prule zu reduzieren
//-----------------------------------------------------------------------------
*/

static void    RedGoalR ( termpair *ptr )
{
    termpair   *xptr;
    bool       change = false;
    subst       *tau = NULL;

    PCL_REDUCE_RIGHT (ptr);
    if (Rreduce (prule, &(ptr->right)))
    {
        Referee_Red_Goal ( prule );
        change = true;
        reduce ( &(ptr->right) );
    }

    PCL_REDUCE_LEFT (ptr);
    if (Rreduce (prule, &(ptr->left)))
    {
        Referee_Red_Goal ( prule );
        change = true;
        reduce ( &(ptr->left) );
    }

    if (change)
    {
        if (CPWeight==occnest)
            init_goal_fn_m();
        Modify (ptr);
        change = false;
        if (CancelActive)
            Cancellation (ptr);

        RedGoal ( ptr );
        if (equal (ptr->left, ptr->right))
            Proved ();

        if (Paramodulation && unify (ptr->left, ptr->right,&tau))
        {
            printf ( "Instanziere Ziel:   %4ld  ", ptr->number );

            getweight (ptr->left  = substitute (tau, ptr->left));
            getweight (ptr->right = substitute (tau, ptr->right));
            printtpair ( ptr );
            Proved ();
        }

        if ((xptr = FindSubsum (&SetOfEquations, ptr)) != NULL)
        {
            PCL_SUBSUM (xptr,ptr);
            ProvedBy ( xptr );
        }

	if( Paramodulation && testredgoal( ptr ) )
	{
	  ParaGoal ( ptr );
	}
    }
    else
    {
        pgoal = ptr;
        ParaRule ( prule );
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       RedGoalE ( termpair *ptr )
//
//  Parameter:      ptr     Ziel
//
//  Beschreibung:   Versucht Goal mit pequ zu reduzieren
//-----------------------------------------------------------------------------
*/

static void    RedGoalE ( termpair *ptr )
{
    termpair   *xptr;
    bool       change = false;
    subst       *tau = NULL;

    if (tpsubsum (pequ, ptr))
    {
        PCL_SUBSUM (pequ,ptr);
        Proved ();
    }
    else
    {
        PCL_REDUCE_RIGHT (ptr);
        if (Ereduce (pequ, &(ptr->right)))
        {
            Referee_Red_Goal ( pequ );
            change = true;
            reduce ( &(ptr->right) );
        }

        PCL_REDUCE_LEFT (ptr);
        if (Ereduce (pequ, &(ptr->left)))
        {
            Referee_Red_Goal ( pequ );
            change = true;
            reduce ( &(ptr->left) );
        }

        if (change)
        {
            if (CPWeight==occnest)
                init_goal_fn_m();
            Modify (ptr);
            change = false;
            if (CancelActive)
                Cancellation (ptr);
            RedGoal ( ptr );
            if (equal (ptr->left, ptr->right))
                Proved ();

            if (Paramodulation && unify (ptr->left, ptr->right,&tau))
            {
                printf ( "Instanziere Ziel:   %4ld  ", ptr->number );

                getweight (ptr->left  = substitute (tau, ptr->left));
                getweight (ptr->right = substitute (tau, ptr->right));
                printtpair ( ptr );
                Proved ();
            }

            if ((xptr = FindSubsum (&SetOfEquations, ptr)) != NULL)
            {
                PCL_SUBSUM (xptr,ptr);
                ProvedBy ( xptr );
            }

	    if( Paramodulation && testredgoal( ptr ) )
            {
	      ParaGoal ( ptr );
	    }
        }
        else
        {
            pgoal = ptr;
            ParaEqu ( pequ );
        }
    }
}



/*
//-----------------------------------------------------------------------------
//  Funktion:       RedRuleR ( termpair *ptr )
//
//  Parameter:      ptr    Zeiger auf eine Regel
//
//  Beschreibung:   Die Regel ptr wird wenn moeglich mit prule reduziert
//-----------------------------------------------------------------------------
*/

static void    RedRuleR ( termpair *ptr )
{
    if (ptr->count != prule->count) /* Damit eine Regel nicht sich selbst red. */
    {
        PCL_REDUCE_RIGHT (ptr);
        if (Rreduce (prule, &(ptr->right)))
        {
            Referee_Red_Right ( prule );
            Modify (ptr);
            reduce ( &(ptr->right) );

            if (CancelActive && Cancellation ( ptr ))
                ReInsertRule ( ptr );
        }

        PCL_REDUCE_LEFT (ptr);
        if (Rreduce (prule, &(ptr->left)))
        {
            Referee_Red_Left ( prule );
            Modify (ptr);
            DeleteRule ( ptr );
            reduce ( &(ptr->left) );
            if (equal (ptr->left, ptr->right))
            {
                DelRule ( ptr )
            }
            else
                RedRule ( ptr )
        }
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       RedEquR ( termpair *ptr )
//
//  Parameter:      ptr    Zeiger auf eine Gleichung
//
//  Beschreibung:   Die Gleichung ptr wird wenn moeglich mit prule reduziert
//-----------------------------------------------------------------------------
*/

static void    RedEquR ( termpair *ptr )
{
    bool    notdel = true;

    PCL_REDUCE_LEFT (ptr);
    if (Rreduce (prule, &(ptr->left)))
    {
        Referee_Red_Equation ( prule );
        Modify (ptr);
        DeleteEqu ( ptr );
        reduce ( &(ptr->left) );
        PCL_REDUCE_RIGHT (ptr);
        if (Rreduce (prule, &(ptr->right)))
            reduce ( &(ptr->right) );

        if (equal (ptr->left, ptr->right))
        {
            DelEquation ( ptr )
            notdel = false;
        }
        else
            RedEquation ( ptr )
    }

    PCL_REDUCE_RIGHT (ptr);
    if (notdel && Rreduce (prule, &(ptr->right)))
    {
        Referee_Red_Equation ( prule );
        Modify (ptr);
        DeleteEqu ( ptr );
        reduce ( &(ptr->right) );
        if (equal (ptr->left, ptr->right))
        {
            DelEquation ( ptr )
        }
        else
            RedEquation ( ptr )
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       SubsumEqu ( termpair *ptr )
//
//  Parameter:      ptr    Zeiger auf eine Gleichung
//
//  Beschreibung:   Es wird geprueft, ob die Gleichung ptr von pequ
//                  subsumiert wird.
//-----------------------------------------------------------------------------
*/

static void    SubsumEqu ( termpair *ptr )
{
    if ((pequ != ptr) && (tpsubsum (pequ, ptr)))
    {
        PCL_SUBSUM (pequ,ptr);
        Referee_Subsum ( pequ );
        DeleteEqu ( ptr );
        DelEquation ( ptr );
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       RedRuleE ( termpair *ptr )
//
//  Parameter:      ptr    Zeiger auf eine Regel
//
//  Beschreibung:   Die Regel ptr wird wenn moeglich mit pequ reduziert
//-----------------------------------------------------------------------------
*/

static void    RedRuleE ( termpair *ptr )
{
    PCL_REDUCE_RIGHT (ptr);
    if (Ereduce (pequ, &(ptr->right)))
    {
        Referee_Red_Right ( pequ );
        Modify (ptr);
        reduce ( &(ptr->right) );
    }

    PCL_REDUCE_LEFT (ptr);
    if (Ereduce (pequ, &(ptr->left)))
    {
        Referee_Red_Left ( pequ );
        Modify (ptr);
        DeleteRule ( ptr );
        reduce ( &(ptr->left) );
        if (equal (ptr->left, ptr->right))
        {
            DelRule ( ptr );
        }
        else
            RedRule ( ptr )
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       RedEquR ( termpair *ptr )
//
//  Parameter:      ptr    Zeiger auf eine Gleichung
//
//  Beschreibung:   Die Gleichung ptr wird wenn moeglich mit pequ reduziert
//-----------------------------------------------------------------------------
*/

static void    RedEquE ( termpair *ptr )
{
    bool    notdel = true;

    if (ptr != pequ)
    {
        PCL_REDUCE_LEFT (ptr);
        if (Ereduce (pequ, &(ptr->left)))
        {
            Referee_Red_Equation ( pequ );
            Modify (ptr);
            DeleteEqu ( ptr );

            reduce ( &(ptr->left) );
            PCL_REDUCE_RIGHT (ptr);
            if (Ereduce (pequ, &(ptr->right)))
                reduce ( &(ptr->right) );

            if (equal (ptr->left, ptr->right))
            {
                DelEquation ( ptr );
                notdel = false;
            }
            else
                RedEquation ( ptr );
        }

        PCL_REDUCE_RIGHT (ptr);
        if (notdel && Ereduce (pequ, &(ptr->right)))
        {
            Referee_Red_Equation ( pequ );
            Modify (ptr);
            DeleteEqu ( ptr );
            reduce ( &(ptr->right) );
            if (equal (ptr->left, ptr->right))
            {
                DelEquation ( ptr );
            }
            else
                RedEquation ( ptr );
        }
    }
}



/*
//=============================================================================
//      Implementierung der exportierten Funktionen
//=============================================================================
*/

/*
//-----------------------------------------------------------------------------
//  Funktion:       NewRule ( termpair *rule )
//
//  Parameter:      rule    Orientiertes Kritisches Paar
//
//  Beschreibung:   Aktionen, die eine neue Regel zur Folge hat
//-----------------------------------------------------------------------------
*/

void    NewRule ( termpair *rule )
{
    rule->parent1 = NULL;
    rule->parent2 = NULL;
    rule->type    = RULE;

    InsertRule ( rule );
    ref_new_rules++;

    if (!DemoMode && special_factor && rule->special_flag)
        printf ( "Spezielle Regel:\n" );

    if (DemoMode)
        STEP();
    else
    {
        timetag();
        printf ( "Neue Regel:         %4ld  ", (long)(rule->number));
        printtpair ( rule );
    }
    flush ();
    ClrReferee ( rule );

    prule = rule;

    ForAllRulesDo ( RedRuleR );
    ForAllEquDo   ( RedEquR );

    if (Proofmode)
        ForAllGoalsDo ( RedGoalR );

    ForAllRulesDo ( CPRuleRule );
    ForAllEquDo   ( CPRuleEqu );

    if (Paramodulation)
        ForAllGoalsDo ( ParaNewRule );
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       NewEquation ( termpair *equ )
//
//  Parameter:      equ     Unrichtbares kritisches Paar
//
//  Beschreibung:   Aktionen, die eine neue Gleichung zur Folge hat
//-----------------------------------------------------------------------------
*/

void    NewEquation ( termpair *equ )
{
    termpair    *ptr;

    if ((ptr = FindSubsum (&SetOfEquations, equ)) != NULL)
    {
        PCL_SUBSUM ( ptr, equ );
        deleteterm ( equ->left );
        deleteterm ( equ->right );
        deletepair ( equ );
        return;
    }

    PCL_SAVE ( equ );
    equ->parent1 = NULL;
    equ->parent2 = NULL;
    equ->type    = EQUATION;

    InsertEqu ( equ );
    ref_new_equations++;
    if (DemoMode && special_factor && equ->special_flag)
        printf ( "Spezielle Gleichung:\n" );

    if (DemoMode)
        STEP();
    else
    {
        timetag();
        printf (  "Neue Gleichung:     %4ld  ", (long)(equ->number) );
        printtpair ( equ );
    }
    flush ();                                                           
    ClrReferee ( equ );

    pequ = equ;
    ForAllEquDo   ( SubsumEqu );
    ForAllRulesDo ( RedRuleE  );
    ForAllEquDo   ( RedEquE   );

    if (Proofmode)
        ForAllGoalsDo ( RedGoalE );

    ForAllRulesDo ( CPEquRule );
    ForAllEquDo   ( CPEquEqu );

    if (Paramodulation)
        ForAllGoalsDo ( ParaNewEqu );
}


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  testnewgoal                                              */
/*                                                                           */
/*  Parameter    :  neues Ziel, das getestet werden soll                     */
/*                                                                           */
/*  Returnwert   :  true, falls das neue Ziel aufgenommen werden muss        */
/*                  false, falls nicht                                       */
/*                                                                           */
/*  Beschreibung :  Ein Ziel g1 wird nicht mehr ben"otigt, falls es ein an-  */
/*                  deres Ziel g2 und eine Substitution sigma gibt mit       */
/*                     sigma( g2 ) = g1.                                     */
/*                  Diese Substituion mu"s f"ur beide Seiten der Ziele gelten*/
/*                                                                           */
/*                  Es wird die Menge SetOfGoals durchlaufen. Dabei wird     */
/*                  zun"achst getestet, ob das neue Ziel wegfallen kann, weil*/
/*                  f"ur es die obige Bedingung zutrifft. Gibt es ein        */
/*                  entsprechendes Ziel in SetOfGoals wird das neue Ziel     */
/*                  nicht aufgenommen und false zur"uckgeliefert.            */
/*                                                                           */
/*                  Ist dies nicht der Fall, so wird "uberpr"uft, ob das alte*/
/*                  Ziel aus SetOfGoals wegen der obigen Bedingung wegfallen */
/*                  kann.                                                    */
/*                  Beachte: Ist dies der Fall f"ur ein altes Ziel, so kann  */
/*                           es in SetOfGoals kein Ziel mehr geben, das be-  */
/*                           wirken kann, da"s das neue Ziel wegf"allt.      */
/*                           Deshalb wird der erste Fall dann nicht mehr be- */
/*                           trachtet.                                       */
/*                           Da"s dies nicht mehr vorkommen kann, liegt da-  */
/*                           ran, da"s die obige Bedingung auf keine 2 Ziele,*/
/*                           die in SetOfGoals vorkommen, zutrifft, d.h. die */
/*                           Menge SetOfGoals ist stets minimal. Man macht   */ 
/*                           sich leicht klar, da"s, falls es ein solches    */ 
/*                           Ziel in SetOfGoals g"abe, dieses bereits vorher */ 
/*                           daf"ur gesogt h"atte, da"s das Ziel, das auf-   */ 
/*                           grund des neuen Ziels eliminiert wird, gel"oscht*/ 
/*                           worden w"are.                                   */ 
/*                           Dabei beachte man, da"s alle Ziele variablen-   */ 
/*                           disjunkt sind.                                  */ 
/*                                                                           */ 
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  SetOfGoals.first                                         */
/*                                                                           */
/*****************************************************************************/

static bool testnewgoal( termpair *newgoal )
{
  bool       delete_old_goal = false;
  termpair  *oldgoal, 
	    *next;

  oldgoal = SetOfGoals.first;
  while( oldgoal && !delete_old_goal )
  {
    next = oldgoal->next;

    if( goal_in_goal_match( oldgoal, newgoal ) )
    {
      deleteterm( newgoal->left );
      deleteterm( newgoal->right );
      deletepair( newgoal );
      return false;
    }

    if( goal_in_goal_match( newgoal, oldgoal ) )
    {
      delgoal( oldgoal );
      delete_old_goal = true;
    }
    
    oldgoal = next;
  } /* Ende von while */

  if( delete_old_goal )
  {
    while ( oldgoal )
    {
      next = oldgoal->next;

      if( goal_in_goal_match( newgoal, oldgoal ) )
      {
	delgoal( oldgoal );
      }

      oldgoal = next;
    } /* Ende von while */
  } /* Ende von if( delete_old_goal ) */

  return true;

} /* Ende von testnewgoal */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  testredgoal                                              */
/*                                                                           */
/*  Parameter    :  reduziertes Ziel, das getestet werden soll               */
/*                                                                           */
/*  Returnwert   :  true, falls das reduzierte Ziel beibehalten werden muss  */
/*                  false, falls nicht                                       */
/*                                                                           */
/*  Beschreibung :  Die Beschreibung stimmt mit der f"ur testnewgoal "uberein*/
/*                  Der Unterschied besteht darin, da"s nicht ein neues      */
/*                  Ziel untersucht wird, sondern stattdessen ein Ziel be-   */
/*                  trachtet wird, das zuvor mit einer Regel oder Gleichung  */
/*                  reduziert wurde.                                         */
/*                  Mit dieser Funktion wird also garantiert, da"s die Menge */
/*                  SetOfGoals stets minimal ist.                            */
/*                                                                           */ 
/*                  Das reduzierte Ziel wird also also aus SetOfGoals ge-    */ 
/*                  l"oscht, falls es ein Match von einem anderen Ziel auf   */ 
/*                  dieses reduzierte Ziel gibt.                             */ 
/*                  Das L"oschen eines anderes Zieles verl"auft analog zu    */ 
/*                  der Beschriebung in der Funktion testnewgoal.            */ 
/*                                                                           */ 
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  SetOfGoals.first                                         */
/*                                                                           */
/*****************************************************************************/

static bool testredgoal( termpair *redgoal )
{
  bool       delete_old_goal = false;
  termpair  *oldgoal, 
	    *next;

  oldgoal = SetOfGoals.first;
  while( oldgoal && !delete_old_goal )
  {
    if( redgoal->number == oldgoal->number )
    {
      oldgoal = oldgoal->next;
      continue;
    }

    next = oldgoal->next;

    if( goal_in_goal_match( oldgoal, redgoal ) )
    {
      delgoal( redgoal );
      return false;
    }

    if( goal_in_goal_match( redgoal, oldgoal ) )
    {
      delgoal( oldgoal );
      delete_old_goal = true;
    }
    
    oldgoal = next;
  } /* Ende von while */

  if( delete_old_goal )
  {
    while ( oldgoal )
    {
      if( redgoal->number == oldgoal->number )
      {
	oldgoal = oldgoal->next;
	continue;
      }

      next = oldgoal->next;

      if( goal_in_goal_match( redgoal, oldgoal ) )
      {
	delgoal( oldgoal );
      }

      oldgoal = next;
    } /* Ende von while */
  } /* Ende von if( delete_old_goal ) */

  return true;

} /* Ende von testredgoal */


/*
//-----------------------------------------------------------------------------
//  Funktion:       NewGoal ( termpair *goal )
//
//  Parameter:      goal    neues Ziel
//
//  Beschreibung:   Aktionen die ein neues Ziel zur Folge hat.
//                  - Ziel reduzieren
//                  - Testen, ob es einen Match von einem bereits existierenden
//                    Ziel auf dieses neue Ziel gibt, wenn ja, so wird das neue
//                    Ziel nicht ben"otigt  --> Ende
//                  - Testen, ob es einen Match von diesem neuen Ziel auf ein
//                    altes gibt, wenn ja, so kann dieses alte Ziel gel"oscht werden
//                   Diese beiden Tests werden gemeinsam durch die Funktion 
//                   testnewgoal durchgef"uhrt
//                  - neues Ziel hinzuf"ugen
//                  - Testen, ob mit diesem Ziel die Beweisaufgabe gel"ost wurde;
//                    d.h. Ziel auf Gleichheit und Unifizierbarkeit testen
//                    --> Beweis evtl. beendet
//                  - neue krit. Ziele mit diesem Ziel bilden
//                  
//-----------------------------------------------------------------------------
*/

void    NewGoal ( termpair *goal )
{
    if ( IsAct( goal ) )
    {
	/*******************/
	/* Ziel reduzieren */
	/*******************/
        PCL_REDUCE_LEFT (goal);
        reduce ( &(goal->left) );
        PCL_REDUCE_RIGHT (goal);
        reduce ( &(goal->right) );

        if (CancelActive)
            Cancellation (goal);

	/******************************************************************/
	/* Testen, ob neues Ziel oder alte Ziele gel"oscht werden k"onnen */
	/******************************************************************/
	if ( testnewgoal( goal ) )
        {
	    /**************************/
	    /* Neues Ziel hinzuf"ugen */
	    /**************************/
	    AddGoal ( goal );
	    ClrReferee ( goal );
	    Paramodul ( goal );

	    /*************************/
	    /* Auf Gleichheit testen */
	    /*************************/
	    if (equal ( goal->left, goal->right ))
	    {
		Proved ();
	    }

	    /*******************************/
	    /* Auf Unifizierbarkeit testen */
	    /*******************************/
	    UnifyGoal ( goal );

	    /***************************/
	    /* Neue krit. Ziele bilden */
	    /***************************/
	    ParaGoal ( goal );
	}
    } /* Ende von if ( IsAct( goal ) ) */
    else
    {
        deleteterm ( goal->left );
        deleteterm ( goal->right );
        deletepair ( goal );
    }
} /* Ende von NewGoal */


/*
//=============================================================================
//      Initialisierung und Termination
//=============================================================================
*/

/*
//-----------------------------------------------------------------------------
//  Funktion:       Initialize ( bool output )
//
//  Parameter:      output      Flag fuer Ausgabe
//
//  Beschreibung:   Initialisierung der Vervollstaendigung
//-----------------------------------------------------------------------------
*/

void    Initialize ( bool output )
{
    register termpair   *ptr;
    register function   i;
    static   bool       firstcall = true;

    WhereDone = T_INITIALIZE;

    if (output && !Empty (RULE))
    {
        printf   ( "Regeln:\n" );
        printf   ( "-------\n\n" );
        PrintRules ();
        printf   ( "\n\n" );
    }

    if (output && !Empty (EQUATION))
    {
        printf   ( "Gleichungen:\n" );
        printf   ( "------------\n\n" );
        PrintEquations ();
        printf   ( "\n\n" );
    }

    if (output)
    {
        printf   ( "Unbehandelte Gleichungen:\n" );
        printf   ( "-------------------------\n\n" );
        PrintSet ( &SetOfCriticalPairs );
        printf   ( "\n\n" );
    }

    if (output && Proofmode)
    {
        printf  ( "Ziele:\n" );
        printf  ( "-----\n\n" );
        PrintSet ( &SetOfGoals );
        printf  ( "\n" );
    }

    if (output && Proofmode && Paramodulation )
    {
        printf  ( "Kritische Ziele:\n" );
        printf  ( "-----\n\n" );
        PrintSet ( &SetOfCriticalGoals );
        printf  ( "\n" );
    }

    if (firstcall)
    {
        itime = systime ();
        firstcall = false;
    }

    if (Proofmode)
    {
        ptr = SetOfGoals.first;
        if (ptr)
        {
            if (CancelActive && Cancellation ( ptr ))
            {
                printf  ( "Cancellation liefert:\n" );
                RedGoal ( ptr );
            }
            if (equal (ptr->left, ptr->right))
            {
                printf      ( "Ziel (%4ld) ist trivial.\n", ptr->number );
                DeleteGoal  ( ptr );
                deleteterm  ( ptr->left );
                deleteterm  ( ptr->right );
                deletepair  ( ptr );
                Completed   ( true );
            }

            if (Paramodulation)
                UnifyGoal ( ptr );
        }
        printf  (  "\n" );
    }

    RedCount = 0;

    for (i = 0; i <= FuncCount;
        InititalSet ( &(SetOfRules[i++]) ));
    InititalSet ( &SetOfEquations );
    InititalSet ( &SetOfCriticalPairs );
    InititalSet ( &SetOfGoals );
    if( Paramodulation )
      InititalSet ( &SetOfCriticalGoals );

    WhereDone = T_UNKNOWN;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       Proved ()
//
//  Parameter:      -keine-
//
//  Beschreibung:   Der Beweis ist gelungen.
//-----------------------------------------------------------------------------
*/

void    Proved  ( void )
{
    if (!DemoMode)
    {
        printf  ( "\n" );
        printf  ( "        +----------------------------------+\n" );
        printf  ( "        |   Damit ist das Ziel bewiesen.   |\n" );
        printf  ( "        +----------------------------------+\n\n" );
    }

    Completed ( true );
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       Completed ( bool success )
//
//  Parameter:      success     true, falls ein Beweis geglueckt ist
//                              false sonst
//
//  Beschreibung:
//-----------------------------------------------------------------------------
*/

void    Completed ( bool success )
{
    int i;

    itime   = systime ()-itime;

    PCL_COMMENT("Vervollstaendigung abgeschlossen, Ergebnisse",true);
    PCL_EXIT ( Proofmode );
    PCL_COMMENT("Ergebnisse Ende",true);
    PCL_CLOSE();
    PCL_EXTRACT(!ParallelMode);  /* Do not extract if in sequential
				    mode (treat as a non-master),
				    because whole PCL file will be
				    extracted later */
				   

    if(ProtocolMode)
    {
       printf("Logfile: %s\n\n",logfile);
       if (!(log_f = fopen (logfile, "a")))
              Error ( __FILE__ ": "  "Completed", "LOGFILE-Fehler.");

       fprintf(log_f,"\ncycle %d\n\n",CycleCount-1);
       fprintf(log_f,"master:%d\n",MasterHost);
       for (i = 0; i < HostCount; i++)
       {
          if(i!=ThisHost)
          {
             fprintf(log_f,"process %d using configuration %d (NO_CONFIG) did %d steps and %d steps\n",
                          i,NO_CONFIG,0,0);
          }
          else
          {
	     if( database_config )  /* Database-Experte lief zuerst auf diesem Prozess */
	     {
	       OwnConfig.exp_nr = DATABASE;
	     }

             fprintf(log_f,"process %d using configuration %d (%s) did %d steps and %d steps\n",
                          i,OwnConfig.exp_nr,
                          get_exp_name(OwnConfig.exp_nr),FirstStepsDone,SecondStepsDone);
          }
       }
       fprintf(log_f,"\n");
       switch(WhereDone)
       {
       case T_INITIALIZE:     fprintf(log_f,
                                      "team terminated by process %ld during initialization\n",
                                      ThisHost);
                              break;
       case T_COMPLETION:     fprintf(log_f,
                                      "team terminated by process %ld during completion\n",
                                      ThisHost);
                              break;
       case T_INTERREDUKTION: fprintf(log_f,
                                   "team terminated by process %ld during interreduction\n",
                                   ThisHost);
                              break;
       case T_UNKNOWN:        fprintf(log_f,
                                      "team terminated by process %ld from unknown function\n",
                                      ThisHost);
                              break;
       default:               Error ( __FILE__ ": "  "Completed",
				     "Illegal Code for logging termination.");
       }
       fclose(log_f);
    }


    if (Terminate && ParallelMode) /* Von StS nach hier verlegt...*/
       Terminate ();

    if (DemoMode)
    {
        STOP();
        putchar ( '\n' );

        if (!Empty (RULE) && (!Proofmode))
        {
            printf ( "Endgueltiges Regelsystem:\n" );
            printf ( "-------------------------\n\n" );
            PrintRules ();
            printf ( "\n\n" );
        }

        if (!Empty (EQUATION) && (!Proofmode))
        {
            printf ( "Aktuelles Gleichungssystem:\n" );
            printf ( "---------------------------\n\n" );
            PrintEquations ();
            printf ( "\n\n" );
        }
    }
    else
    {
        if (MemStatistic)
            Statistic ();

        printf ( "\n\n" );
        if (!Empty (RULE))
        {
            if (Proofmode)
            {
                printf ( "Aktuelles Regelsystem:\n" );
                printf ( "----------------------\n\n" );
            }
            else
            {
                printf ( "Endgueltiges Regelsystem:\n" );
                printf ( "-------------------------\n\n" );
            }
            PrintRules ();
            printf ( "\n\n" );
        }

        if (!Empty (EQUATION))
        {
            if (Proofmode)
            {
                printf ( "Aktuelles Gleichungssystem:\n" );
                printf ( "---------------------------\n\n" );
            }
            else
            {
                printf ( "Endgueltiges Gleichungssystem:\n" );
                printf ( "------------------------------\n\n" );
            }
            PrintEquations ();
            printf ( "\n\n" );
        }
    }

    if (Proofmode && success)
        printf ( "Das Ziel konnte bewiesen werden.\n\n" );
    else if (Proofmode)
        printf ( "Der Beweis ist fehlgeschlagen.\n\n" );
    printf ( "Laufzeit                 : %ld.%03ld s\n\n", itime/1000, itime%1000 );
    printf ( "Davon fuer Planungsphase : %ld.%03ld s\n\n", GesamtPlanungszeit/1000, GesamtPlanungszeit%1000 );

    if (!DemoMode)
    {
        struct rusage   ruse;

        getrusage (RUSAGE_SELF, &ruse );
        printf ( "System Information:\n" );
        printf ( "-------------------\n\n" );
        printf ( "user time used:                        %6d.%03d s\n",
                                                         ruse.ru_utime.tv_sec,
                                                         ruse.ru_utime.tv_usec/1000);
        printf ( "system time used:                      %6d.%03d s\n",
                                                         ruse.ru_stime.tv_sec,
                                                         ruse.ru_stime.tv_usec/1000);
        printf ( "maximum of resident set size utilized: %6d\n", ruse.ru_maxrss    );
        printf ( "page reclaims:                         %6d\n", ruse.ru_minflt    );
        printf ( "page faults:                           %6d\n", ruse.ru_majflt    );
        printf ( "swaps:                                 %6d\n", ruse.ru_nswap     );
        printf ( "block input operations:                %6d\n", ruse.ru_inblock   );
        printf ( "block output operations:               %6d\n", ruse.ru_oublock   );
        printf ( "messages sent:                         %6d\n", ruse.ru_msgsnd    );
        printf ( "messages received:                     %6d\n", ruse.ru_msgrcv    );
        printf ( "signals received:                      %6d\n", ruse.ru_nsignals  );
        printf ( "voluntary context switches:            %6d\n", ruse.ru_nvcsw     );
        printf ( "involuntary context switches:          %6d\n", ruse.ru_nivcsw    );
    }

    printf ( "\n\n\n" );
    printf ( "%5d Regeln gebildet.\n",          SetOfRules[0].count );
    printf ( "%5d Gleichungen gebildet.\n",     SetOfEquations.count );
    printf ( "%5d kritische Paare gebildet.\n", SetOfCriticalPairs.count );
    printf ( "%5d kritische Ziele gebildet.\n", SetOfCriticalGoals.count );
    printf ( "%5d Reduktionen.\n",              RedCount );

    flush ();

    if (ParallelMode)
    {
        PrintHistory ();

/*        if (Terminate)
            Terminate (); Nach vorne gelegt, damit es keine
	    wechselseitigen Absch"usse gibt... StS*/
        flush ();
        if (!(SilentMode || NullMode))
            getchar ();
    }
    PCL_FEXTRACT(); /* Extract ALL files...*/
    exit ( 0 );
}



/*
//=============================================================================
//      Vervollstaendigung
//=============================================================================
*/

/*
//-----------------------------------------------------------------------------
//  Funktion:       Completion
//
//  Parameter:      -keine-
//
//  Beschreibung:   Vervollstaendigung
//-----------------------------------------------------------------------------
*/

void    Completion ( void  )
{
    termpair    *cp, *ptr;
    termpair    *next;
    short       pcounter = ParaCount - 1; /* Nur relevant, wenn ParaCount > 0 */

    if (DemoMode)
        START();

    itime = systime ();

    if((CPWeight == CSpecBoundLearnWeight)||
       (CPWeight == LSpecBoundLearnWeight))
    {  /* Initialize the Learning functions */
       CPWeight(NULL);
    }

    cp = DelFirst (&SetOfCriticalPairs);
    while (cp)
    {
#ifdef SHOWCP
       putchar ( '.' );
       flush ();
#endif
       if (IsAct (cp))
       {
#ifndef NOCPSEQUENCE
	  ptr = SetOfCriticalPairs.first;
	  while (ptr && (cp->weight == ptr->weight))
	  {
	     next = ptr->next;
	     if (!(IsAct (ptr)) || (tpequal (cp, ptr)))
	     {
		delcp ( ptr );
#ifdef SHOWCP
		putchar ( ':' );
		flush ();
#endif /* SHOWCP */
	     }
	     ptr = next;
	  }
#endif /* NOCPSEQUENCE */
	  
	  PCL_REDUCE_LEFT (cp);
	  reduce ( &(cp->left) );
	  PCL_REDUCE_RIGHT (cp);
	  reduce ( &(cp->right) );
	  
	  if (equal ( cp->left, cp->right ))
	  {
	     deleteterm ( cp->left );
	     deleteterm ( cp->right );
	     deletepair ( cp );
	  }
	  else
	  {
	     if ( CancelActive && Cancellation ( cp ) )
		printf ( "---- Cancellation. ----\n" );
	     
#ifdef SHOWCP
	     putchar ( '\n' );
	     printf ( "Bearbeite: " );
	     printtpair ( cp );
	     flush ();
#endif
	     
	     New (cp);
	     switch (Compare (cp->left, cp->right))
	     {
	     case TLESS:
		SwapSides ( cp );
		PCL_ORIENT ( cp ,'x');
		NewRule ( cp );
		break;
	     case TGREATER:
		PCL_ORIENT ( cp ,'u');
		NewRule ( cp );
		break;
	     default:
		NewEquation ( cp );
		break;
	     }
	     if (Paramodulation)
	     {
		if ( ParaCount > 0 ) /* Betonung der krit. Paare */
		{
		   if ( !pcounter && !Empty( CRITGOAL ) )
		   {
                      ptr = DelFirst (&SetOfCriticalGoals);
                      NewGoal ( ptr );
                      pcounter = ParaCount;
		   }
		   pcounter = (pcounter) ? pcounter-1 : 0;
		}
		else /* Betonung der krit. Ziele */
		{
		   for ( pcounter=ParaCount; pcounter<0; pcounter++ )
		   {
                      ptr = DelFirst (&SetOfCriticalGoals);
                      NewGoal ( ptr );
		   }
		}   
	     } /* Ende von if (Paramodulation) */
	     
	     flush ();
	  }
       }
       else
       {
	  deleteterm ( cp->left );
	  deleteterm ( cp->right );
	  deletepair ( cp );
       }
       cp = DelFirst (&SetOfCriticalPairs);
    }
    
    while ( Paramodulation && !Empty (CRITGOAL))
    {
       ptr = DelFirst (&SetOfCriticalGoals);
       NewGoal ( ptr );
    }
    
    if (DemoMode)
       STOP();
}




/*
//=============================================================================
//      Parallelvervollstaendigung (nicht bei ATARI-Version)
//=============================================================================
*/

void prt ( termpair *ptr )
{
     printf ( "weight = %5ld   ", ptr->weight );
     printtpair ( ptr );
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       ParallelCompletion ( long sec )
//
//  Parameter:      sec     Beschraenkung der Laufzeit
//
//  Beschreibung:   Vervollstaendigung
//-----------------------------------------------------------------------------
*/

void    ParallelCompletion ( long sec )
{

    static bool tflag = false;

    termpair    *cp, *ptr;
    termpair    *next;
    short       pcounter = ParaCount - 1; /* Nur relevant, wenn ParaCount > 0 */
    DList_p     lptr   = NULL;
    DList_p     lptr2  = NULL;
    short       i;
    pairset     SaveCPs = EmptySet;     /* Falls R, G zurueckgestellt werden */

    ref_devel_cps       = 0;
    ref_weight_cps      = 0;
    ref_new_rules       = 0;
    ref_red_rules       = 0;
    ref_del_rules       = 0;
    ref_new_equations   = 0;
    ref_red_equations   = 0;
    ref_del_equations   = 0;
    ref_new_cp          = 0;
    ref_del_cp          = 0;

    SecondStepsDone     = 0;
    WhereDone           = T_COMPLETION;

    if (DemoMode)
        START();

    if (LastCPCount)
    {
        lptr2 = lptr = newdlist (0.0);
        for (i = 1; i < LastCPCount; i++)
        {
            lptr->next = newdlist (0.0);
            lptr = lptr->next;
        }
        lptr->next = lptr2;
        LastCPWeight = 0;
    }


    if(!ReproMode)
    {
       settimer ( sec, &tflag );
    }

    if(CPWeight == CSpecBoundLearnWeight)
    {
       CPWeight(NULL);
    }
    
    cp = DelFirst (&SetOfCriticalPairs);
    while (((!ReproMode && (!tflag ||
			    ( HostCount == 1))) 
	    ||
	    ( ReproMode && (SecondStepsDone <
			    SecondStepsPerCycle[CycleCount-1])))
	   && cp)
    {
        SecondStepsDone++;
        #ifdef SHOWCP
            putchar ( '.' );
            flush ();
        #endif
        if (IsAct (cp))
        {
            #ifndef NOCPSEQUENCE
            if (cp->weight > -20000)
            {
                ptr = SetOfCriticalPairs.first;
                while (ptr && (cp->weight == ptr->weight))
                {
                    next = ptr->next;
                    if ((!IsAct (ptr)) || (tpequal (cp, ptr)))
                    {
                        delcp ( ptr );
                        ref_del_cp++;
                    }
                    ptr = next;
                }
            }
            #endif

            PCL_REDUCE_LEFT (cp);
            reduce ( &(cp->left) );
            PCL_REDUCE_RIGHT (cp);
            reduce ( &(cp->right) );

            if (equal ( cp->left, cp->right ))
            {
                deleteterm ( cp->left );
                deleteterm ( cp->right );
                deletepair ( cp );
                ref_del_cp++;
            }
            else
            {
	       if ( CancelActive && Cancellation ( cp ) )
	       {
		  printf ( "---- Cancellation. ----\n" );
	       }
	       
#ifdef SHOWCP
	       putchar ( '\n' );
#endif
	       
	       New (cp);
	       
	       if (cp->weight >= -20000)
	       {
		  ref_devel_cps++;
		  ref_weight_cps += cp->weight;
		  if (LastCPCount)
		  {
		     LastCPWeight -= lptr->info;
		     LastCPWeight += lptr->info = cp->weight;
		     lptr = lptr->next;
		  }
	       }
	       
	       switch (Compare (cp->left, cp->right))
	       {
	       case TLESS:
		     SwapSides ( cp );
		     PCL_ORIENT(cp,'x'); /* Hier kann es Aerger geben,
					    weil das CP in PCL_ORIENT
					    schon mit -> ausgegeben
					    wird */ 
		     
		     if (EquationsOnly && !Empty (CRITPAIR))
		     {
			cp->weight = CPWeight ( cp );
			cp->type = RULE;
			Insert ( &SaveCPs, cp );
			break;
		     }
		     NewRule ( cp );
		     break;
	       case TGREATER:  
		     PCL_ORIENT(cp,'u');
		     if (EquationsOnly && !Empty (CRITPAIR))
		     {
			cp->weight = CPWeight ( cp );
			cp->type = RULE;
			Insert ( &SaveCPs, cp );
			break;
		     }
		     NewRule ( cp );
		     break;
               default:
		     if (RulesOnly && !Empty (CRITPAIR))
		     {
			cp->weight = CPWeight ( cp );
			cp->type = EQUATION;
			Insert ( &SaveCPs, cp );
			break;
		     }
		     NewEquation ( cp );
		     break;
	       }
	       
	       if (Paramodulation)
	       {
                  if ( ParaCount > 0 ) /* Betonung der krit. Paare */
                  {
		     if ( !pcounter && !Empty( CRITGOAL ) )
		     {
			ptr = DelFirst (&SetOfCriticalGoals);
			NewGoal ( ptr );
			pcounter = ParaCount;
		     }
		     pcounter = (pcounter) ? pcounter-1 : 0;
                  }
                  else /* Betonung der krit. Ziele */
                  {
		     for ( pcounter=ParaCount; pcounter<0; pcounter++ )
		     {
			printf("pcounter : %d\n", pcounter);
			ptr = DelFirst (&SetOfCriticalGoals);
			NewGoal ( ptr );
		     }
                  }   
	       } /* Ende von if (Paramodulation) */
	       
	       flush ();
            }
        }
        else
        {
	   deleteterm ( cp->left );
	   deleteterm ( cp->right );
	   deletepair ( cp );
	   ref_del_cp++;
        }
        cp = DelFirst (&SetOfCriticalPairs);
	
        if (!cp && SaveCPs.first)
        {
	   cp = DelFirst ( &SaveCPs );
	   while (cp)
	   {
	      cp->type = CRITPAIR;
	      PCL_REDUCE_LEFT (cp);
	      reduce ( &(cp->left) );
	      PCL_REDUCE_RIGHT (cp);
	      reduce (&(cp->right));
	      if (equal ( cp->left, cp->right ))
	      {
		 deleteterm ( cp->left );
		 deleteterm ( cp->right );
		 deletepair ( cp );
		 ref_del_cp++;
	      }
	      else
	      {
		 cp->weight = CPWeight ( cp );
		 InsertCP( cp );
		 /*Insert (&SetOfCriticalPairs, cp);*//* Geaendert 7.4.94 MK */
	      }
	      cp = DelFirst ( &SaveCPs );
	   }
	   cp = DelFirst (&SetOfCriticalPairs);
        }
    }
    
    while(((!ReproMode && (!tflag || 
			  ( HostCount == 1)))
	   ||
	   ( ReproMode&& (SecondStepsDone <
			  SecondStepsPerCycle[CycleCount-1])))
	  && Paramodulation && !Empty (CRITGOAL))
    {
       SecondStepsDone++;
       ptr = DelFirst (&SetOfCriticalGoals);
       NewGoal ( ptr );
    }
    
    if (!cp)
       Completed ( false );
    
    if (DemoMode)
    {
       STOP();
       DEMOINFO();
    }
    WhereDone = T_UNKNOWN;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       ParallelCheckCPs
//
//  Parameter:      sec     Beschraenkung der Laufzeit
//
//  Beschreibung:   Prueft alle Initialen kritischen Paare auf
//                  Zusammenfuehrbarkeit
//-----------------------------------------------------------------------------
*/

void    ParallelCheckCPs ( long sec )
{

    static bool tflag           = false;

    bool        redflag         = false;
    long        red_counter     = 0;
    long        sub_counter     = 0;
    long        dub_counter     = 0;
    long        mark_counter    = 0;
    long        i;
    termpair    *cp, *ptr;


    if(!ReproMode)
    {
       settimer ( sec, &tflag );
    }
    
    cp = SetOfCriticalPairs.first;
    for (i = 0; (i < StartCP) && cp; i++)
        cp = cp->next;

    FirstStepsDone = 0;
    SecondStepsDone = 0;

    while (((!ReproMode&&!tflag)||
	    (ReproMode&&
	     (FirstStepsDone < FirstStepsPerCycle[CycleCount-1])))
	    && cp)
    {
        FirstStepsDone++;
        if (SubsumCPs && FindSubsum (&SetOfEquations, cp))
        {
            Clear ( cp );
            sub_counter++;
        }
        else if (ReduceCPs) /* I'm doing this only for the reductions, */
			    /* as no step from this Expert will ever */
			    /* be encounterd in a actual proof...  StS */
        {
           PCL_REDUCE_LEFT (cp);
           redflag = redflag | reduce (&(cp->left));
           PCL_REDUCE_RIGHT (cp);
           redflag = redflag | reduce (&(cp->right));
           if(redflag)
	   {
	      redflag = false;

	      Modify (cp);

	      if (equal ( cp->left, cp->right ))
	      {
		 Clear ( cp );
		 red_counter++;
	      }
	      else
		 if (SubsumCPs && FindSubsum (&SetOfEquations, cp))
	      {
		 Clear ( cp );
		 sub_counter++;
	      }
           }
        }
        cp = cp->next;
    }
    printf ( "\n%ld kritische Paare konnten zusammengefuehrt werden.\n", red_counter );
    printf (   "%ld kritische Paare konnten subsumiert werden.\n", sub_counter );
    flush ();


    if (DoubleCPs)
    {
	SecondStepsDone = 0;
        cp = SetOfCriticalPairs.first;
        for (i = 0; (i < StartCP) && cp; i++)
            cp = cp->next;
        while (((!ReproMode&&!tflag)||
		(ReproMode&&
		 (SecondStepsDone < SecondStepsPerCycle[CycleCount-1]))) 
	       && cp)
        {

            if (cp->state != CLEARED)
            {
                ptr = cp->next;
                while (((!ReproMode&&!tflag)||
			(ReproMode&&
			 (SecondStepsDone <
			  SecondStepsPerCycle[CycleCount-1])))
		       && ptr)
                {
                    if ((cp != ptr) &&
                        (ptr->state != CLEARED) &&
                        (tpequal (cp, ptr)))
                    {
                        Clear (ptr);
                        Mark  (cp);
                        dub_counter++;
                    }
                    ptr = ptr->next;
                }

                if (cp->state == MARKED)
                    mark_counter++;
            }
            cp = cp->next;

	    SecondStepsDone++;
        }

        printf (   "%ld doppelte kritische Paare.\n"            , dub_counter );
        printf (   "%ld markierte kritische Paare.\n\n"         , mark_counter );
        flush ();
    }


    if(!ReproMode)
    {
       if (tflag)
	  printf ( "Zeit ist abgelaufen.\n\n" );
    }

    flush ();
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       ParallelCheckCPPs
//
//  Parameter:      sec     Beschraenkung der Laufzeit
//
//  Beschreibung:   Prueft alle Initialen kritischen Paare auf
//                  Zusammenfuehrbarkeit.
//                  Im Gegensatz zu ParallelCheckCPs wird allerdings
//                  der Startpunkt prozentual festgelegt, nicht absolut.
//-----------------------------------------------------------------------------
*/

void    ParallelCheckCPPs ( long sec )
{
    printf ( "\nStarte mit %ld %% von %ld = ", 
                           StartCP,   SizeOf (SetOfCriticalPairs));
    StartCP = (StartCP * SizeOf (SetOfCriticalPairs))/ 100;
    printf ( "    kritisches Paar %ld\n\n", StartCP );
    ParallelCheckCPs ( sec );
}

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  ParallelCheckCGs                                         */
/*                                                                           */
/*  Parameter    :  Zeitvorgabe durch den Leiter                             */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Pr"uft alle kritischen Ziele auf Zusammenf"uhrbarkeit.   */
/*                  Der Startpunkt wird absolut angegeben.                   */
/*                                                                           */
/*  Globale Var. :  StartCG                                                  */
/*                                                                           */
/*  Externe Var. :  SetOfCriticalGoals                                       */
/*                                                                           */
/*****************************************************************************/
void    ParallelCheckCGs ( long sec )
{

  static bool   tflag           = false;

  bool          redflag         = false;
  long          red_counter     = 0;
  long          sub_counter     = 0;
  long          dub_counter     = 0;
  long          mark_counter    = 0;
  long          i;
  termpair     *cg, *ptr; 

  settimer ( sec, &tflag );

  /*********************/
  /* Startpunkt suchen */
  /*********************/
  cg = SetOfCriticalGoals.first;
  for( i=0; (i < StartCG) && cg; i++ )
  {
    cg = cg->next;
  }

  FirstStepsDone = 0;
  SecondStepsDone = 0;

  while(((!ReproMode&&!tflag)||
	 (ReproMode&&
	  (FirstStepsDone < FirstStepsPerCycle[CycleCount-1])))
	&& cg)
  {
    FirstStepsDone++;

    /**************************/
    /* Testen auf Subsumption */
    /**************************/
    if( SubsumCGs && FindSubsum( &SetOfGoals, cg ) )
    {
      Clear( cg );
      sub_counter++;
    }
    /* Testen auf Reduzierbarkeit */
    else if ( ReduceCGs )
    {
      redflag = reduce( &(cg->left) );
      redflag = redflag || reduce( &(cg->right) );
      
      if( redflag )
      {
	Modify( cg );

	if( equal( cg->left, cg->right ) )
	{
	  Proved( );
	}
	else if( SubsumCGs && FindSubsum( &SetOfGoals, cg ) )
	{
	  Clear( cg );
	  sub_counter++;
	}
	else
	{
	  red_counter++;
	}
      } /* Ende von if( redflag ) */
    } /* Ende von else if */
  } /* Ende von while */

  printf("\n %ld kritische Ziele konnten subsumiert werden.\n", sub_counter );
  printf("\n %ld kritische Ziele konnten reduziert werden.\n", sub_counter );
  flush();


  /*************************************/
  /* Test auf doppelte kritische Ziele */
  /*************************************/
  if( DoubleCGs )
  {
    /*********************/
    /* Startpunkt suchen */
    /*********************/
    cg = SetOfCriticalGoals.first;
    for( i=0; (i < StartCG) && cg; i++ )
    {
      cg = cg->next;
    }

    while(((!ReproMode&&!tflag)||
	   (ReproMode&&
	    (SecondStepsDone < SecondStepsPerCycle[CycleCount-1]))) 
	  && cg)
  {
      if( cg->state != CLEARED )
      {
	ptr = cg->next;
	while( ((!ReproMode&&!tflag)||
		(ReproMode&&
		 (SecondStepsDone <
		  SecondStepsPerCycle[CycleCount-1]))) 
	      && ptr )

	{
	  if( (cg != ptr) && (ptr->state != CLEARED) && (tpequal( cg, ptr )) )
	  {  /* kritisches Ziel kommt doppelt vor */
	    Clear( ptr );
	    Mark( cg );
	    dub_counter++;
	  }

	  ptr = ptr->next;
	} /* Ende des inneren whiles */

	if( cg->state == MARKED )
	{
	  mark_counter++;
	}
      } /* Ende von if( cg->state != CLEARED ) */

      cg = cg->next;

      SecondStepsDone++;
    } /* Ende von while */

    printf("%ld doppelte kritische Ziele.\n"            , dub_counter );
    printf("%ld markierte kritische Ziele.\n"            , mark_counter );
    flush();

  } /* Ende von if( DoubleCGs ) */

  if(ReproMode)
  {
     if( tflag )
     {
	printf ( "Zeit ist abgelaufen.\n\n" );
     }
  }

  flush ();
} /* Ende von ParallelCheckCGs */

/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  ParallelCheckCGPs                                        */
/*                                                                           */
/*  Parameter    :  Zeitvorgabe durch den Leiter                             */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Pr"uft alle kritischen Ziele auf Zusammenf"uhrbarkeit.   */
/*                  Der Startpunkt wird prozentual angegeben.                */
/*                                                                           */
/*  Globale Var. :  StartCG                                                  */
/*                                                                           */
/*  Externe Var. :  SetOfCriticalGoals                                       */
/*                                                                           */
/*****************************************************************************/
void    ParallelCheckCGPs ( long sec )
{
    printf ( "\nStarte mit %ld %% von %ld = ",
                           StartCG,   SizeOf (SetOfCriticalGoals));
    StartCG = (StartCG * SizeOf (SetOfCriticalGoals))/ 100;
    printf ( "    kritisches Paar %ld\n\n", StartCG );
    ParallelCheckCGs ( sec );
  
} /* Ende von ParallelCheckCGPs */


/*
//-----------------------------------------------------------------------------
//  Funktion:       Interreduce ( pairset *set )
//
//  Parameter:      set     Menge der neuen Gleichnungen
//
//  Beschreibung:   in das aktuelle System werden die neuen Aussagen aus
//                  der Termpaarmenge set eingefuegt.
//                  Dabei wird die Menge gleichzeitig geloescht. (MK)
//-----------------------------------------------------------------------------
*/

void    Interreduce ( pairset *set )
{
    termpair    *cp;

    if (!set->first)
        return;

    WhereDone = T_INTERREDUKTION;

    if (DemoMode)
    {
        printf ( "Interreduktion der Gleichungen.\n" );
        START();
    }
    else
    {
        printf ( "-------------------------------------------------------------------\n"
                 "    Interreduktion der Gleichungen\n" 
                 "-------------------------------------------------------------------\n\n\n");
        flush ();
    }
    cp = DelFirst (set);
    while (cp)
    {
        #ifdef SHOWCP
            putchar ( '.' );
            flush ();
        #endif

        
       PCL_REDUCE_LEFT (cp);
       reduce ( &(cp->left) );
       PCL_REDUCE_RIGHT (cp);
       reduce ( &(cp->right) );

        if (equal ( cp->left, cp->right ))
        {
            deleteterm ( cp->left );
            deleteterm ( cp->right );
            deletepair ( cp );
        }
        else
        {
            if ( CancelActive && !DemoMode && Cancellation ( cp ) )
                 printf ( "---- Cancellation. ----\n" );

            #ifdef SHOWCP
                putchar ( '\n' );
            #endif

            switch (Compare (cp->left, cp->right))
            {
            case TLESS:     SwapSides ( cp );
                            PCL_ORIENT ( cp ,'x');
                            NewRule ( cp );
                            break;
            case TGREATER:  PCL_ORIENT( cp ,'u');
                            NewRule ( cp );
                            break;
            default:        NewEquation ( cp );
                            break;
            }
            flush ();
        }
        cp = DelFirst (set);
    }
    if (DemoMode)
        STOP();
   WhereDone = T_UNKNOWN;
}


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  InterreduceGoals                                         */
/*                                                                           */
/*  Parameter    :  Menge von kritischen Zielen                              */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  In das aktuelle System werden die neuen kritischen Ziele */
/*                  aus der Termpaarmenge set eingefuegt.                    */
/*                  Dabei wird die uebergebene Menge gleichzeitig geloescht! */
/*                  Diese Funktion wird vom Leiter beim Teammeating aufgeru- */
/*                  fem.                                                     */
/*                                                                           */
/*  Globale Var. :  alle, die es gibt ( fast alle )                          */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

void InterreduceGoals ( pairset *set )
{
  termpair *cg;

  if (!set->first)
    return;

  /*****************/
  /* Dokumentation */
  /*****************/
  if (DemoMode)
    {
        printf ( "Interreduktion der Ziele.\n" );
        START();
    }
    else
    {
        printf ( "-------------------------------------------------------------------\n"
                 "    Interreduktion der Ziele\n"
                 "-------------------------------------------------------------------\n\n\n");
        flush ();
    }
  
  /*************************************************************/
  /* Durchlaufen der kritischen Ziele und Aufnehmen ins System */
  /*************************************************************/
  cg = DelFirst( set );
  while( cg )
  {
    #ifdef SHOWCP
        putchar ( '.' );
        flush ();
    #endif

    NewGoal( cg );

    cg = DelFirst( set );
  }

  if (DemoMode)
    STOP();
} /* Ende von InterreduceGoals */



/*
//=============================================================================
//      Spezialist fuer Zusammenfuehrung kritischer Paare
//=============================================================================
*/

/*
//-----------------------------------------------------------------------------
//  Funktion:       CheckCPs
//
//  Parameter:      -keine-
//
//  Beschreibung:   Prueft alle Initialen kritischen Paare auf
//                  zusammenfuehrbarkeit
//-----------------------------------------------------------------------------
*/

void    CheckCPs ( void )
{
    termpair    *cp;

    cp = SetOfCriticalPairs.first;
    while (cp)
    {
        if (cp->state == INITIAL)
        {
            PCL_REDUCE_LEFT (cp);
	    reduce ( &(cp->left) );
            PCL_REDUCE_RIGHT (cp);
	    reduce ( &(cp->right) );

            if (equal ( cp->left, cp->right ))
                Clear ( cp );
        }
        cp = cp->next;
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       ClearCPs
//
//  Parameter:      -keine-
//
//  Rueckgabewert:  true    Es wurde mindestens ein CP geloescht
//                  false   sonst
//
//  Beschreibung:   Alle veralteten CPs werden geloescht
//-----------------------------------------------------------------------------
*/

bool    ClearCPs ( bool msg )
{
    termpair    *cp, *ncp;
    bool        act;
    long        count = 0;
    long        size  = SizeOf (SetOfCriticalPairs);

    if (msg)
    {
        printf ( "\n" );
        printf ( "Speichermangel:\n" );
        printf ( "---------------\n\n" );
    }

    cp = SetOfCriticalPairs.first;
    if (cp)
       cp = cp->next;

    while (cp)
    {
        ncp = cp->next;
        act = IsAct (cp);

        if (!act)
        {
            DeleteCP   ( cp );
            deleteterm ( cp->left );
            deleteterm ( cp->right );
            deletepair ( cp );
            count++;
        }
        cp = ncp;
    }

    if (msg)
    {
        printf ( "%ld von %ld kritische Paaren geloescht.\n\n",
                 count,   size );
    }

    return (count > 0);
}


/*
//-----------------------------------------------------------------------------
//      ClearData
//-----------------------------------------------------------------------------
*/

/*
//-----------------------------------------------------------------------------
//  Funktionen:     delrule, delequ, delcp, delgoal, delcg
//
//  Parameter:      ptr     Zeiger auf ein Termpaar
//
//  Beschreibung:   Loeschen des entsprchenden Termpaares aus der Menge
//                  und loeschen der Terme.
//-----------------------------------------------------------------------------
*/

static void delrule ( termpair *ptr )
{
    DeleteRule ( ptr );
    deleteterm ( ptr->left );
    deleteterm ( ptr->right );
    deletepair ( ptr );
}


static void delequ ( termpair *ptr )
{
    DeleteEqu ( ptr );
    deleteterm ( ptr->left );
    deleteterm ( ptr->right );
    deletepair ( ptr );
}


static void delcp ( termpair *ptr )
{
    DeleteCP ( ptr );
    deleteterm ( ptr->left );
    deleteterm ( ptr->right );
    deletepair ( ptr );
}


static void delgoal ( termpair *ptr )
{
    DeleteGoal ( ptr );
    deleteterm ( ptr->left );
    deleteterm ( ptr->right );
    deletepair ( ptr );
}


static void delcg ( termpair *ptr )
{
    DeleteCritGoal ( ptr );
    deleteterm ( ptr->left );
    deleteterm ( ptr->right );
    deletepair ( ptr );
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       ClearData
//
//  Parameter:      -keine-
//
//  Beschreibung:   Alle Termpaare werden geloescht.
//-----------------------------------------------------------------------------
*/

void    ClearData ( void )
{
    ForAllRulesDo     ( delrule );
    ForAllEquDo       ( delequ );
    ForAllCPsDo       ( delcp );
    ForAllGoalsDo     ( delgoal );
    ForAllCritGoalsDo ( delcg );

    ReorgTermpair ( DemoMode );
}
