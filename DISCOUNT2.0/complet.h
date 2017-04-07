/*
//=============================================================================
//      Projekt:        TEAM-COMPLETION
//
//      Header:         complet
//
//      Author:         Werner Pitz, 1991
//
//      Beschreibung:   Eigentliche Vervollstaendigung bzw. Beweiser
//-----------------------------------------------------------------------------
//      $Log: complet.h,v $
//      Revision 0.3  1991/09/27  08:14:07  pitz
//      RulesOnly und EquationsOnly bei Parallelvervollstaendigung implementiert.
//
//      Revision 0.2  1991/09/19  13:03:56  pitz
//      Erfassung statistischer Informationen mittels:
//        ref_new_rules,      ref_red_rules,      ref_del_rules
//        ref_new_equations,  ref_red_equations,  ref_del_equations
//        ref_new_cp,                             ref_del_cp
//
//      Revision 0.1  1991/08/26  13:45:09  pitz
//      ParallelCheckCPPs: Reduktionsspezialist wie ParallelCheckCPs.
//                         Allerdings wird der Startpunkt relativ festgelegt.
//
//      Revision 0.0  1991/08/09  08:18:19  pitz
//      Initiale Version
//
//=============================================================================
*/

#ifndef     __COMPLETION
#define     __COMPLETION

#include   "systime.h"
#include   "referee.h"
#include   "cpweight.h"
#include   "learn_nterms.h"
#include   "learn_lpairs.h"

/*****************************************************************************/
/*                                                                           */
/*                               Konstanten                                  */
/*                                                                           */
/*****************************************************************************/
               /************************************************/
               /* Nummer einer Konfiguration, die bei einem    */
	       /* Reprolauf nicht mehr ben"otigt wird, weil ein*/
	       /* anderer Proze"s den Beweis terminieren wird. */
               /************************************************/
#define NO_CONFIG    -1

/*----------------------------------------------------------------------------
                  Typvereinbarungen
----------------------------------------------------------------------------*/

typedef enum  /* Gibt an, in welchem Zustand sich der Prozess befindet */
	      /* und von  wo Completed() aufgerufen wurde  */
{
   T_COMPLETION,
   T_INTERREDUKTION,
   T_INITIALIZE,
   T_UNKNOWN
}StateType;

/*
//-----------------------------------------------------------------------------
//      aktuelle Einstellung
//-----------------------------------------------------------------------------
*/

extern int        Cycles;    /* Zyklen f"ur den Repro-Lauf  */
extern int       *FirstStepsPerCycle;
extern int       *SecondStepsPerCycle;
extern int       *ConfigPerCycle;
extern int       *MasterPerCycle;
extern bool      ReproMode;

extern bool       MemStatistic;
 
extern bool       Proofmode;
extern bool       CancelActive;
extern bool       Paramodulation;
extern bool       KritKapur;
extern short      ParaCount;
extern bool       RedInst;
extern bool       DemoMode;
extern bool       ProtocolMode;
extern int        FirstStepsDone;
extern int        SecondStepsDone;
extern StateType  WhereDone; 

extern bool       RulesOnly;
extern bool       EquationsOnly;

extern bool       ReduceCPs;
extern bool       SubsumCPs;
extern bool       DoubleCPs;
extern long       StartCP;

extern bool       ReduceCGs;
extern bool       SubsumCGs;
extern bool       DoubleCGs;
extern long       StartCG;

extern double     LastCPWeight;
extern short      LastCPCount;

extern long       ref_devel_cps;
extern long       ref_weight_cps;

extern long       ref_new_rules;
extern long       ref_red_rules;
extern long       ref_del_rules;
extern long       ref_new_equations;
extern long       ref_red_equations;
extern long       ref_del_equations;
extern long       ref_new_cp;
extern long       ref_del_cp;

extern void       (*Terminate)(void);

extern long       GesamtPlanungszeit;

extern long       itime;


/*
//-----------------------------------------------------------------------------
//      Funktionsvereinbarungen
//-----------------------------------------------------------------------------
*/

void    NewRule     ( termpair *rule );
void    NewEquation ( termpair *equ );
void    NewGoal     ( termpair *goal );

void    Initialize  ( bool output );
void    Proved      ( void );
void    Completed   ( bool success );

void    Completion  ( void );
void    ParallelCompletion ( long sec );
void    ParallelCheckCPs  ( long sec );
void    ParallelCheckCPPs ( long sec );
void    ParallelCheckCGs ( long sec );
void    ParallelCheckCGPs ( long sec );
void    Interreduce ( pairset *set );
void    InterreduceGoals ( pairset *set );

void    CheckCPs    ( void );

bool    ClearCPs    ( bool msg );
void    ClearData   ( void );

#endif
