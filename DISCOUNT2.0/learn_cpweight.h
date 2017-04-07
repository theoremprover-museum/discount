/*-------------------------------------------------------------------------

File        : learn_cpweight.h

Autor       : Stephan Schulz

Inhalt      : Functions evaluating critical pairs according to learned
              knowledge stored in a knowledge base and return a
	      value.

Aenderungen : <1> 14.12.1995 neu

-------------------------------------------------------------------------*/

#ifndef _learn_cpweight

#define _learn_cpweight

#include "learn_specific.h"

/*-----------------------------------------------------------------------*/
/*                   Typdeklarationen                                    */
/*-----------------------------------------------------------------------*/

typedef enum 
{
   aggressive,    /* Prefer low goal_dist */
   opportunistic, /* Prefer similar goal_dist, but take last found as */
		  /* the standard */ 
   adaptive,      /* As above, but only move slowly in the direction */
		  /* of the last found cp */
   conservative   /* Prefer high goal_dist */
}DistanceStrategy;

#define UNK_BASE_DEFAULT 20



/*-----------------------------------------------------------------------*/
/*       Deklaration exportierter Funktionen und Variablen               */
/*-----------------------------------------------------------------------*/

extern long             LearnInfluence,
                        ProofsWeight,
                        UnknownWeight;

extern DistanceStrategy DistStrat;

extern double           ProofsLimit,
                        TotalLimit,
                        GoalLimit, 
                        CPCostLimit,
                        NegativeLimit;

extern bool             CompleteKB; /* Used only by TSM eval functions
				       at the moment */

extern bool             PosLimitsNeg,
                        TSMAbsolutes;

void SetLearnEvalParams(DistanceStrategy strat, long tot_w, long
			ave_w, long proofs_w, long goal_dist_w, 
			bool global_backup, bool all_spec_doms, long
			unk_w, long learn_inf, long unk_b, long eval_funct); 

void SetFLearnParams(DistanceStrategy strat, double proof_l, 
		     double total_l, double goal_l, double cp_cost_l,
		     double neg_l, bool pos_l_neg, bool tsm_abs);

double ErodeQuality(double old_val);

double GoalDistAim(double new_gd, bool set);

#define DEFAULT_EVAL_FCT 0

extern double (*EvalFoundEqn)(Lpair_p known, long goal_dist);

double CGlobalLearnWeight( termpair *tp );
double LGlobalLearnWeight( termpair *tp );

double CGoalBoundLearnWeight( termpair *tp );
double LGoalBoundLearnWeight( termpair *tp );

double CSpecBoundLearnWeight( termpair *tp );
double LSpecBoundLearnWeight( termpair *tp );

double CGFloatLearnWeight( termpair *tp );

#endif

/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/





