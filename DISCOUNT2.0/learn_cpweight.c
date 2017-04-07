/*-------------------------------------------------------------------------

File        : learn_cpweight.c

Autor       : Stephan Schulz

Inhalt      : Functions evaluating critical pairs (well, actually two
              terms that are though of as a critical pair) according
	      to learned knowledge stored in a knowledge base and
	      return a value.

Aenderungen : <1> 14.12.1995 neu

-------------------------------------------------------------------------*/


#include "learn_cpweight.h"

/*-----------------------------------------------------------------------*/
/*                         Globale Variable                              */
/*-----------------------------------------------------------------------*/

long   LearnInfluence,
       ProofsWeight,
       UnknownWeight;

/* Possible values: aggressive, opportunistic, adaptive, conservative */

DistanceStrategy DistStrat = conservative;

double ProofsLimit,
       TotalLimit,
       GoalLimit,
       CPCostLimit,
       NegativeLimit;

bool   CompleteKB = true;
bool   PosLimitsNeg = false,
       TSMAbsolutes = false;

long             tot_weight,
                 ave_weight,
                 proofs_weight,
                 goal_dist_weight,
                 unk_weight,
                 unk_base;
bool             use_global_as_backup,
                 use_all_spec_doms;

double           (*EvalFoundEqn)(Lpair_p known, long goal_dist) = NULL;



/*-----------------------------------------------------------------------*/
/*           Forward-Deklaration interner Funktionen                     */
/*-----------------------------------------------------------------------*/


long read_global_knowledge(Lpair_p * root, bool complete,
				      LpairTreeParams_p max_values);
long calc_new_goal_dist(long old_goal_dist, 
				   long found_goal_dist);

/*-----------------------------------------------------------------------*/
/*                     Interne Funktionen                                */
/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: read_global_knowledge()
//
//  Reads the file realpath(KnowledgeBase/Xdata) if KnowledgeBase !=
//  NULL, realpath("KNOWLEDGE/Xdata") otherwise. X is either c or p,
//  depending on the value of complete (complete == true -> cdata
//  (complete data) is read). The fuction will skip the syntactic
//  sugar and go directly for the facts. These facts will be inserted
//  into the tree at *root. Return value is the maximal goal distance
//  found in the input file.
//
// Global Variables: KnowledgeBase, TmpPathName
//
// Side Effects    : Memory, IO, building the tree
//
/----------------------------------------------------------------------*/


long read_global_knowledge(Lpair_p *root, bool complete,
			   LpairTreeParams_p max_values)
{
   long tree_size;

   NormTermPreserveArity = GetBoolKBVariable("PreserveArity");
   PosAndNeg = GetBoolKBVariable("PosAndNeg");
   
   strcpy(PathNameHelp, KBFullName());
   strcat(PathNameHelp, complete ? "/cdata":"/pdata");

   InitScanner(PathNameHelp);
   printf("Reading global knowledge from %s.\n", PathNameHelp);
   
   while(!(TestIdent(ident, "facts") && ColonFollows)) /* Skip junk */
   {
      NextRealToken();
      if(TestToken(NoToken))
      {
	 ScannerError("Unexpected end of file");
      }
   }
   
   AcceptIdent(ident, "facts");
   AcceptToken(colon);

   tree_size = ParseLpairTree(root, max_values);
   
   EndScanner();
   printf("Knowledge read.\n");

   return tree_size;
}
   

/*-------------------------------------------------------------------------

FUNCTION         : long calc_new_goal_dist(long old_goal_dist, 
                                           long found_goal_dist)

Beschreibung     : Calculates the new goal-distance to use as a
                   reference for the evaluation.

Globale Variable : DistStrat

Seiteneffekte    : Changes GoalDist

Aenderungen      : <1> 19.12.1994 neu

-------------------------------------------------------------------------*/

long calc_new_goal_dist(long old_goal_dist, long found_goal_dist)
{
   switch(DistStrat)
   {
   case adaptive:
      return (3*old_goal_dist + found_goal_dist)/4;
      break;
   case opportunistic:
      return found_goal_dist;
      break;
   default: /* Might be used later... */
      return found_goal_dist;
      break;
   }
}


/*-------------------------------------------------------------------------

FUNCTION         : eval_linear_original()

Beschreibung     : Returns a weight for the critical pair in known,
                   based on the information stored in the cell. This
		   is the original function as evaluated in Stephan
		   Schulz' Diploma Thesis "Explanation Based Learning
		   for Distributed Equational Deduction". Due to
		   Debugging in the loading function (Calculation of
		   the average proof distance) it may return different
		   results now.

Globale Variable : tot_weight, ave_weight, ProofsWeight,
                   goal_dist_weight, DistStrat

Seiteneffekte    : -

Aenderungen      : <1> 21.12.1994 neu

-------------------------------------------------------------------------*/

double eval_linear_original(Lpair_p known, long goal_dist)
{
   long gd_influence,
        experience;

   switch(DistStrat)
   {
   case aggressive:
      gd_influence = -known->goal_dist;
      break;
   case opportunistic: 
   case adaptive:
      gd_influence = abs(known->goal_dist - goal_dist);
      break;
   case conservative:
      gd_influence =  known->goal_dist;
      break;
   default: 
      Error ( __FILE__ ": "  "EvalFoundEqn", 
             "Only (0..3) legal for DIST_STRAT" );
      gd_influence = 0; /* Just to stiffle the Warning */
      break;
   }
   experience = (goal_dist_weight * -gd_influence)
        + (tot_weight       * -known->tot_ref)
        + (ave_weight       * -known->ave_ref) 
        + (ProofsWeight    * -known->proofs); 
   return 20*experience +
      (known->lside)->term_weight+(known->rside)->term_weight;
}

/*-------------------------------------------------------------------------

FUNCTION         : eval_linear_improved()

Beschreibung     : Returns a weight for the critical pair in known,
                   based on the information stored in the cell. This
		   is the version that was in use for the CADE-13 ATP
		   competition. 

Globale Variable : tot_weight, ave_weight, ProofsWeight,
                   goal_dist_weight, DistStrat

Seiteneffekte    : -

Aenderungen      : <1> 21.12.1994 neu
                       20.10.1996 Umwandlung in eine interne Funktion,
		       Evaluierung ab jetzt ueber Funktionspointer!

-------------------------------------------------------------------------*/

double eval_linear_improved(Lpair_p known, long goal_dist)
{
   long gd_sum,
        experience;

   switch(DistStrat)
   {
   case aggressive:
      gd_sum = -known->goal_dist;
      break;
   case opportunistic: 
   case adaptive:
      gd_sum = abs(known->goal_dist - goal_dist);
      break;
   case conservative:
      gd_sum =  known->goal_dist;
      break;
   default: 
      Error ( __FILE__ ": "  "EvalFoundEqn", 
	     "Only (0..3) legal for DIST_STRAT" );
      gd_sum = 0; /* Just to stiffle the Warning */
      break;
   }
   experience = (goal_dist_weight * -gd_sum)
        + (tot_weight       * -known->tot_ref)
	+ (ave_weight       * -known->ave_ref) 
	+ (ProofsWeight    * -known->proofs); 
   return experience*LearnInfluence +
      10*((known->lside)->term_weight+(known->rside)->term_weight);
}


/*-----------------------------------------------------------------------*/
/*                  Exportierte Funktionen                               */
/*-----------------------------------------------------------------------*/


/*-------------------------------------------------------------------------

FUNCTION         : void SetLearnEvalParams(DistanceStrategy strat,
                                           long tot_w, long ave_w,
					   long proofs_w, 
					   long goal_dist_weight, 
					   bool global_backup, 
					   bool all_spec_doms,
                                           long unk_w, 
					   long learn_inf,
                                           long unk_b,
                                           long eval_fct) 

Beschreibung     : Sets the Parameters guiding the evaluation of
                   critical pairs found in the database.

Globale Variable : tot_weight, ave_weight, ProofsWeight,
                   goal_dist_weight, DistStrat, use_global_as_backup,
		   use_all_spec_doms, UnknownWeight, unk_base,
		   EvalFoundEqn;

Seiteneffekte    : Global variables are changed

Aenderungen      : <1> 21.12.1994 neu

-------------------------------------------------------------------------*/

void SetLearnEvalParams(DistanceStrategy strat, long tot_w, long
			ave_w, long proofs_w, long goal_dist_w, 
			bool global_backup, bool all_spec_doms, long
			unk_w, long learn_inf, long unk_b,long eval_fct) 
{
   if(strat > conservative)
   {
      Error ( __FILE__ ": "  "SetLearnEvalParams", 
	     "Only (0..3) legal for DIST_STRAT" );
   }
   
   tot_weight           = tot_w;
   ave_weight           = ave_w;
   ProofsWeight         = proofs_w;
   goal_dist_weight     = goal_dist_w;
   DistStrat            = strat;
   use_global_as_backup = global_backup;
   use_all_spec_doms    = all_spec_doms;
   UnknownWeight        = unk_w;             
   LearnInfluence       = learn_inf;
   unk_base             = unk_b;
   switch(eval_fct)
   {
   case 0:
	 EvalFoundEqn   = eval_linear_improved;
	 break;
   case 1:
	 EvalFoundEqn   = eval_linear_original;
	 break;
   default:
	 Error ( __FILE__ ": "  "SetLearnEvalParams", 
		 "Only (0..1) legal for eval_fct" );
	 break;
   }
}


/*-------------------------------------------------------------------------
//
// Function: SetFLearnParams()
//
//   Sets parameters for the new float-learn
//
// Global Variables: DistStrat, ProofsLimit, TotalLimit, GoalLimit
//
// Side Effect     : -
//
//-----------------------------------------------------------------------*/

void SetFLearnParams(DistanceStrategy strat, double proof_l, 
		     double total_l, double goal_l, double cp_cost_l,
		     double neg_l, bool pos_l_neg, bool tsm_abs)
{
   DistStrat     = strat;
   ProofsLimit   = proof_l;
   TotalLimit    = total_l;
   GoalLimit     = goal_l;
   CPCostLimit   = cp_cost_l;
   NegativeLimit = neg_l;
   PosLimitsNeg  = pos_l_neg;
   TSMAbsolutes  = tsm_abs;
}

/*-------------------------------------------------------------------------
//
// Function: ErodeQuality()
//
//   Take a quality measure (0..1, 0 is good) and make it worse in a
//   well-defined way.
//
// Global Variables: -
//
// Side Effect     : -
//
//-----------------------------------------------------------------------*/

double ErodeQuality(double old_val)
{
   return (old_val+1)/2;
}


/*-------------------------------------------------------------------------
//
// Function: GoalDistAim()
//
//   Maintains the desired value for the distance of the current proof
//   process from the goal. Maximum is 1, 0 would indicate a complete
//   proof process. If set is true sets the value of the internal
//   variables, and returns the value, otherwise adapts the goal
//   distance according to the current strategy (in 
//   DistStrat) and returns the old value. 
//
// Global Variables: DistStrat
//
// Side Effect     : Maintains two internal variables (aim_gd and
//                   old_gd) that may change their valuee.
//
//-----------------------------------------------------------------------*/

double GoalDistAim(double new_gd, bool set)
{
   static double aim_gd;
   static double old_gd;

   if(!set)
   {
      old_gd = aim_gd;
      switch(DistStrat)
      {
      case aggressive:
	    /* Value stays at 0 */
	    break;
      case conservative :
	    aim_gd = 1;
	    break;
      case opportunistic:
	    aim_gd = new_gd;
	    break;
      case adaptive:
	    aim_gd = (3*aim_gd + new_gd)/4;
	    break;
      }
   }
   else
   {
      aim_gd = old_gd = new_gd;
   }
   return old_gd;
}


/*-------------------------------------------------------------------------

FUNCTION         : double CGlobalLearnWeight( termpair *tp )

Beschreibung     : Trys to find the critical pair in a tree containing
                   _all_ useful steps from the knowledge base and use
		   this knowledge to calculate a weight for the term.

Globale Variable : unk_base

Seiteneffekte    : Might load a part of the knowledge base, maintains
                   2 local static variables.

Aenderungen      : <1> 14.12.1994 neu

-------------------------------------------------------------------------*/

double CGlobalLearnWeight( termpair *tp )
{
   static Lpair_p         knowledge_tree = NULL;
   DNormSubst_p           subst;
   Lpair_p                known;
   static LpairTreeParams knowledge_params = {0,0,0,0,0,0};
   static bool            load_tried = false;
   term                   *left  = tp->left;
   term                   *right = tp->right;
   long                   tree_size;
   
   if(!load_tried)
   {
      tree_size = read_global_knowledge(&knowledge_tree, true,
					&knowledge_params); 
      load_tried = true;
   }

   subst = OrderNormTerms(&left, &right);

   known = FindTwoNtermsLtermTree(left, right, subst, knowledge_tree);

   FreeDNormSubst(subst);

   if(known)
   {
      printf("+");
      knowledge_params.goal_dist =
	 calc_new_goal_dist(knowledge_params.goal_dist, 
			    known->goal_dist);
      return EvalFoundEqn(known, knowledge_params.goal_dist);
   }
   
   printf(".");
   return 10*(left->weight+right->weight) + unk_base; /* Arbitrary bias ;-) */
}

/*-------------------------------------------------------------------------

FUNCTION         : double LGlobalLearnWeight( termpair *tp )

Beschreibung     : Trys to find the critical pair in a tree containing
                   only the lemmata from the knowledge base base and use
		   this knowledge to calculate a weight for the term.

Globale Variable : unk_base

Seiteneffekte    : Might load a part of the knowledge base, maintains
                   2 local static variables.

Aenderungen      : <1> 14.12.1994 neu

-------------------------------------------------------------------------*/

double LGlobalLearnWeight( termpair *tp )
{
   static Lpair_p         knowledge_tree = NULL;
   DNormSubst_p           subst;
   Lpair_p                known;
   static LpairTreeParams knowledge_params = {0,0,0,0,0,0};
   static bool            load_tried = false;
   term                   *left  = tp->left;
   term                   *right = tp->right;
   long                   tree_size;
   
   if(!load_tried)
   {
      tree_size = read_global_knowledge(&knowledge_tree, false,
					&knowledge_params); 
      load_tried = true;
   }

   subst = OrderNormTerms(&left, &right);

   known = FindTwoNtermsLtermTree(left, right, subst, knowledge_tree);

   FreeDNormSubst(subst);

   if(known)
   {
      printf("+");
      knowledge_params.goal_dist =
	 calc_new_goal_dist(knowledge_params.goal_dist, 
			    known->goal_dist);
      return EvalFoundEqn(known, knowledge_params.goal_dist);
   }
   
   printf(".");
   return 10*(left->weight+right->weight) + unk_base; /* Arbitrary bias ;-) */
}


#ifdef NEVER_DEFINED
/*-------------------------------------------------------------------------

FUNCTION         : double LGlobalLearnWeight( termpair *tp )

Beschreibung     : Trys to find the critical pair in a tree containing
                   only the lemmata from the knowledge base and use
		   this knowledge to calculate a weight for the term.

Globale Variable : unk_base

Seiteneffekte    : Might load a part of the knowledge base, maintains
                   2 local static variables.

Aenderungen      : <1> 14.12.1994 neu

-------------------------------------------------------------------------*/


double LGlobalLearnWeight( termpair *tp )
{ 
   static Lpair_p knowledge_tree = NULL;
   DNormSubst_p   subst;
   Lpair_p        known;
   static long    goal_dist = 0;
   term           *left  = tp->left;
   term           *right = tp->right;
   
   if(!knowledge_tree)
   {
      goal_dist = read_global_knowledge(&knowledge_tree, false); 
   }

   subst = OrderNormTerms(&left, &right);
   
   known = FindTwoNtermsLtermTree(left, right, subst, knowledge_tree);
   
   FreeDNormSubst(subst);
   
   if(known)
   {
      printf("+");
      goal_dist = calc_new_goal_dist(goal_dist, known->goal_dist);
      return EvalFoundEqn(known, goal_dist);
   }
   
   printf(".");
   return 10*(left->weight+right->weight) + unk_base; /* Arbitrary bias ;-) */
}
#endif


/*-------------------------------------------------------------------------

FUNCTION         : double CGoalBoundLearnWeight( termpair *tp )

Beschreibung     : This function evaluates a critical pair using
                   knowledge about goal domains. If first invoked it
		   will read the complete knowledge relevant to the
		   goal at hand, building knowledge_tree (A structure
		   containing knowledge about critical pairs with
		   repsect to the goal) and goal_bound_subst, a
		   substitution mapping the goal onto the old goal
		   from the knowledge base. 

		   The function will then (and on each subsequent
		   call) evaluate the critical pair handed to it by
		   first searching it in the knowledge tree and
		   evaluating it according to the found knowledge or,
		   if it cannot find it, by a backup strategy that is
		   either CGlobalLearnWeight or the usual variant of
		   add_weight, depending on the value of
		   use_global_as_backup. 

Globale Variable : use_global_as_backup, unk_base

Seiteneffekte    : By calling CGlobalLearnWeight(), Loading of the
                   knowledge tree, Memory

Aenderungen      : <1> 5.1.1994 neu

-------------------------------------------------------------------------*/

double CGoalBoundLearnWeight( termpair *tp )
{ 
   static Lpair_p         knowledge_tree = NULL;
   static DNormSubst_p    goal_bound_subst = NULL;
   static LpairTreeParams max_values = {0,0,0,0,0,0};
   DNormSubst_p           subst;
   Lpair_p                known;
   static bool            load_tried = false;
   term                   *left  = tp->left;
   term                   *right = tp->right;
   
   if(!load_tried)
   {
      max_values = InitGoalDomExpert(&knowledge_tree,
				     &goal_bound_subst, true ); 
      load_tried = true;
   }

   if(goal_bound_subst)
   {
      FreeDNormSubst(OrderNormTerms(&left, &right));
      
      subst = AllocEmptyDNormSubst();
      
      memcpy(&(subst->norm_id[0]), 
	     &(goal_bound_subst->norm_id[0]),
	     sizeof(short)*MAXFUNCTION); 
      subst->f_count = goal_bound_subst->f_count;

      subst = DNormTerm(left, subst);
      subst = DNormTerm(right, subst);

      known = FindTwoNtermsLtermTree(left, right, subst, knowledge_tree);
      
      FreeDNormSubst(subst);
      
      if(known)
      {
	 printf("*");
	 max_values.goal_dist =
	    calc_new_goal_dist(max_values.goal_dist,
			       known->goal_dist);  
	 return EvalFoundEqn(known, max_values.goal_dist);
	 /* This is the supposedly best judgement */   
      }
   }
   if(use_global_as_backup)
   {
      return CGlobalLearnWeight(tp) + unk_base;
   }
   printf(".");
   return 10*(left->weight+right->weight) + 2*unk_base; /* Arbitrary bias ;-) */
}


/*-------------------------------------------------------------------------

FUNCTION         : double LGoalBoundLearnWeight( termpair *tp )

Beschreibung     : This function evaluates a critical pair using
                   knowledge about goal domains. If first invoked it
		   will read the lemma knowledge relevant to the
		   goal at hand, building knowledge_tree (A structure
		   containing knowledge about critical pairs with
		   repsect to the goal) and goal_bound_subst, a
		   substitution mapping the goal onto the old goal
		   from the knowledge base. 

		   The function will then (and on each subsequent
		   call) evaluate the critical pair handed to it by
		   first searching it in the knowledge tree and
		   evaluating it according to the found knowledge or,
		   if it cannot find it, by a backup strategy that is
		   either CGlobalLearnWeight or the usual variant of
		   add_weight, depending on the value of
		   use_global_as_backup.

Globale Variable : use_global_as_backup, unk_base

Seiteneffekte    : By calling LGlobalLearnWeight(), Loading of the
                   knowledge tree, Memory

Aenderungen      : <1> 5.1.1994 neu

-------------------------------------------------------------------------*/

double LGoalBoundLearnWeight( termpair *tp )
{ 
   static Lpair_p         knowledge_tree = NULL;
   static DNormSubst_p    goal_bound_subst = NULL;
   static LpairTreeParams max_values = {0,0,0,0,0,0};
   DNormSubst_p           subst;
   Lpair_p                known;
   static bool            load_tried = false;
   term                   *left  = tp->left;
   term                   *right = tp->right;

   if(!load_tried)
   {
      max_values = InitGoalDomExpert(&knowledge_tree,
				     &goal_bound_subst, false );
      load_tried = true;
   }

   if(goal_bound_subst)
   {
      FreeDNormSubst(OrderNormTerms(&left, &right));
      
      subst = AllocEmptyDNormSubst();
      
      memcpy(&(subst->norm_id[0]),
	     &(goal_bound_subst->norm_id[0]),
	     sizeof(short)*MAXFUNCTION);
      subst->f_count = goal_bound_subst->f_count;

      subst = DNormTerm(left, subst);
      subst = DNormTerm(right, subst);

      known = FindTwoNtermsLtermTree(left, right, subst, knowledge_tree);
      
      FreeDNormSubst(subst);
      
      if(known)
      {
	 printf("*");
	 max_values.goal_dist =
	    calc_new_goal_dist(max_values.goal_dist,
			       known->goal_dist);
	 return EvalFoundEqn(known, max_values.goal_dist);
	 /* This is the supposedly best judgement */
      }
   }
   if(use_global_as_backup)
   {
      return LGlobalLearnWeight(tp)+unk_base;
   }
   printf(".");
   return 10*(left->weight+right->weight) + 2*unk_base; /* Arbitrary bias ;-) */
}




/*-------------------------------------------------------------------------

FUNCTION         : double CSpecBoundLearnWeight( termpair *tp )

Beschreibung     : This function evaluates a critical pair using
                   knowledge about specification domains. If invoked
		   with tp == NULL it will read the complete knowledge
		   relevant to the specification at hand, building
		   knowledge_tree (A structure containing knowledge
		   about critical pairs with repesct to the
		   specificationl) and spec_bound_subst, a
		   substitution mapping the specification onto the old
		   specification from the knowledge base.

		   The function will then (and on each subsequent
		   call) evaluate the critical pair handed to it by
		   first searching it in the knowledge tree and
		   evaluating it according to the found knowledge or,
		   if it cannot find it, by a backup strategy that is
		   either CGlobalLearnWeight or the usual variant of
		   add_weight, depending on the value of
		   use_global_as_backup.

		   The system will retry loading each time it is
		   invoked with tp == 0 as as long as no knowledge is
		   present. 

Globale Variable : use_global_as_backup, unk_base

Seiteneffekte    : By calling CGlobalLearnWeight(), Loading of the
                   knowledge tree, Memory

Aenderungen      : <1> 5.1.1994 neu

-------------------------------------------------------------------------*/

double CSpecBoundLearnWeight( termpair *tp )
{ 
   static Lpair_p         knowledge_tree = NULL;
   static DNormSubst_p    spec_bound_subst = NULL;
   static LpairTreeParams max_values = {0,0,0,0,0,0};
   static bool            load_tried = false;
   DNormSubst_p        subst;
   Lpair_p             known;
   term                *left; 
   term                *right;
   

   if(!tp)
   {
      if(!load_tried)
      {
	 max_values = InitSpecDomExpert(&knowledge_tree,
					&spec_bound_subst, true,
					use_all_spec_doms); 
	 load_tried = true;
      }
      return 0;
   }
   
   left  = tp->left;
   right = tp->right;


   if(spec_bound_subst)
   {
      FreeDNormSubst(OrderNormTerms(&left, &right));
      
      subst = AllocEmptyDNormSubst();
      
      memcpy(&(subst->norm_id[0]),
	     &(spec_bound_subst->norm_id[0]), 
	     sizeof(short)*MAXFUNCTION);
      subst->f_count = spec_bound_subst->f_count;
      
      subst = DNormTerm(left, subst);
      subst = DNormTerm(right, subst);
      
      known = FindTwoNtermsLtermTree(left, right, subst, knowledge_tree);
      
      FreeDNormSubst(subst);
      
      if(known)
      {
	 printf("!");
	 max_values.goal_dist =
	    calc_new_goal_dist(max_values.goal_dist,
			       known->goal_dist); 
	 return EvalFoundEqn(known, max_values.goal_dist);
	 /* This is the supposedly best judgement */ 
      }
   }
   if(use_global_as_backup)
   {
      return CGlobalLearnWeight(tp) + unk_base;
   }
   printf(".");
   return 10*(left->weight+right->weight) + 2*unk_base; /* Arbitrary bias ;-) */
}



/*-------------------------------------------------------------------------

FUNCTION         : double LSpecBoundLearnWeight( termpair *tp )

Beschreibung     : This function evaluates a critical pair using
                   knowledge about specification domains. If invoked
		   with tp == NULL it will read the lemma knowledge
		   relevant to the specification at hand, building
		   knowledge_tree (A structure containing knowledge
		   about critical pairs with repesct to the
		   specificationl) and spec_bound_subst, a
		   substitution mapping the specification onto the old
		   specification from the knowledge base.

		   The function will then (and on each subsequent
		   call) evaluate the critical pair handed to it by
		   first searching it in the knowledge tree and
		   evaluating it according to the found knowledge or,
		   if it cannot find it, by a backup strategy that is
		   either CGlobalLearnWeight or the usual variant of
		   add_weight, depending on the value of
		   use_global_as_backup.

		   The system will retry loading each time it is
		   invoked with tp == 0 as as long as no knowledge is
		   present. 

Globale Variable : use_global_as_backup, unk_base

Seiteneffekte    : By calling LGlobalLearnWeight(), Loading of the
                   knowledge tree, Memory

Aenderungen      : <1> 5.1.1994 neu

-------------------------------------------------------------------------*/

double LSpecBoundLearnWeight( termpair *tp )
{ 
   static Lpair_p      knowledge_tree = NULL;
   static DNormSubst_p spec_bound_subst = NULL;
   static LpairTreeParams max_values = {0,0,0,0,0,0};
   DNormSubst_p        subst;
   Lpair_p             known;
   term                *left; 
   term                *right;
   
   if(!knowledge_tree && !tp)
   {
      max_values = InitSpecDomExpert(&knowledge_tree, &spec_bound_subst,
				    false, use_all_spec_doms);
      return 0;
   }
   
   left  = tp->left;
   right = tp->right;


   if(spec_bound_subst)
   {
      FreeDNormSubst(OrderNormTerms(&left, &right));
      
      subst = AllocEmptyDNormSubst();
      
      memcpy(&(subst->norm_id[0]),
	     &(spec_bound_subst->norm_id[0]), 
	     sizeof(short)*MAXFUNCTION);
      subst->f_count = spec_bound_subst->f_count;
      
      subst = DNormTerm(left, subst);
      subst = DNormTerm(right, subst);
      
      known = FindTwoNtermsLtermTree(left, right, subst, knowledge_tree);
      
      FreeDNormSubst(subst);
      
      if(known)
      {
	 printf("!");
	 max_values.goal_dist =
	    calc_new_goal_dist(max_values.goal_dist,
			       known->goal_dist); 
	 return EvalFoundEqn(known, max_values.goal_dist); 
            /* This is the supposedly best judgement */  
      }
   }
   if(use_global_as_backup)
   {
      return LGlobalLearnWeight(tp) + unk_base;
   }
   printf(".");
   return 10*(left->weight+right->weight) + 2*unk_base; /* Arbitrary bias ;-) */
}



/*---------------------------------------------------------------------*/
/*         New Functions using doubles for more exact mapping          */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: CGFloatLearnWeight()
//
//   Evaluates a critical pair in a similar way as CGlobalLearn(), but
//   uses a more canonical way and makes no (or little) use of
//   arbitrary weights. The distribution of evaluations should be
//   (mostly) independent of the size of the knowledge base.
//
// Global Variables: 
//
// Side Effect     : May load global knowledge
//
//---------------------------------------------------------------------*/

double CGFloatLearnWeight( termpair *tp )
{
   static Lpair_p         knowledge_tree = NULL;
   DNormSubst_p           subst;
   Lpair_p                known;
   static LpairTreeParams k_maximums = {0,0,0,0,0,0};
   static bool            load_tried = false;
   term                   *left  = tp->left;
   term                   *right = tp->right;
   long                   tree_size;
   double                 weight_factor, new_quality;
   static double          avg_cp_cost = 1,
                          avg_proofs = 1;
   
   if(!load_tried)
   {
      tree_size = read_global_knowledge(&knowledge_tree, true,
					&k_maximums);
      load_tried = true;
      GoalDistAim(DistStrat==aggressive?1:0, true);
      avg_cp_cost =
	 (double)k_maximums.cp_cost_accumulated/(double)tree_size;
      avg_proofs = 
	 (double)k_maximums.proofs_accumulated/(double)tree_size;
      printf("Average CP-Cost: %f\n", avg_cp_cost);
   }
   subst = OrderNormTerms(&left, &right);
   known = FindTwoNtermsLtermTree(left, right, subst, knowledge_tree);
/*   PrintDNormTerm(left, subst);
     printf(" =");
     PrintDNormTerm(right, subst);
     printf("\n"); */
   FreeDNormSubst(subst);

   if(known)
   {
      double t_fak = 1,
	     p_fak = 1,
	     n_fak = 1;
	 
      
      /* This maps the counted value onto the interval [1-limit..1]:      */
      /* t_fak = 1 - TotalLimit  * ((double)known->tot_ref/k_maximums.tot_ref);   */
      /*                                                                  */
      /* This maps the counted value onto the interval [1-limit..1+limit] */
      /* c_fak = (1 + CPCostLimit) -                                      */
      /*         2 * CPCostLimit*((double)known->cp_cost/k_maximums.cp_cost);     */
      
      if(k_maximums.tot_ref)
      {
	 t_fak = 1 - TotalLimit  * ((double)known->tot_ref/k_maximums.tot_ref);
	 /* t_fak = (1 + TotalLimit) - 
	    2 * TotalLimit*((double)known->tot_ref/k_maximums.tot_ref);*/
      }
      if(k_maximums.proofs)
      {
	 p_fak = 1 - ProofsLimit * ((double)known->proofs/k_maximums.proofs);
	 /* p_fak = (1 + ProofsLimit) - 
	    2 * ProofsLimit*((double)known->proofs/k_maximums.proofs); */
      }
      if(k_maximums.negative)
      {
	 if(known->proofs)
	 {
	    n_fak = 1;
	 }
	 else
	 {
	    n_fak = 1 + NegativeLimit  *
	       ((double)known->negative/k_maximums.negative);
	 }
      }
	  
/*       new_quality = t_fak*p_fak*n_fak;
      weight_factor = new_quality*tp->quality;
      tp->quality = (tp->quality+new_quality)/2; */
      tp->quality = weight_factor = 0.2 * tp->quality;
      
/*      printf("\n(%ld:%f)(%ld:%f)(%ld:%f) -> %f -- %f\n", 
	     known->tot_ref, t_fak, known->proofs, p_fak,
	     known->negative, n_fak,
	     weight_factor, tp->quality); */
      
      if(known->negative&&known->proofs)
      {
	 putchar('*');
      }
      else if(known->negative)
      {
	 putchar('-');
      }
      else
      {
	 putchar('+');
      }
   }
   else
   {
      printf(".");
      weight_factor = tp->quality;
   }
/*    printf("qual: %f\n", tp->quality);*/
   return weight_factor*(left->weight+right->weight);
}


/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/


