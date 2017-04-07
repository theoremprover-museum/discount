/*-------------------------------------------------------------------------

File        : learn_specific.h

Autor       : Stephan Schulz

Inhalt      : Definitions for the goal- and specification dependend
              learning experts. Includes general infrastructure
	      (global variables, space for filename construction...)

Aenderungen : <1> 6.1.1995 neu

-------------------------------------------------------------------------*/

#ifndef _learn_specific

#define _learn_specific

#include "learn_mixterms.h"
#include "database.h"
#include "exp_def_next_cycletime.h"



/*-----------------------------------------------------------------------*/
/*                     Exportierte Variable                              */
/*-----------------------------------------------------------------------*/


extern char         KnowledgeBase[MAXPATHLEN],
                    TmpPathName[MAXPATHLEN],
                    PathNameHelp[MAXPATHLEN];

#define MAX_KB_VARIABLES 15

/*-----------------------------------------------------------------------*/
/*                 Deklaration exportierter Funktionen                   */
/*-----------------------------------------------------------------------*/


char*           KBFullName();
bool            GetBoolKBVariable(char* var);

DNormSubst_p    FindEquivalentGoal(Lpair_p testgoal);
LpairTreeParams InitGoalDomExpert(Lpair_p *tree, DNormSubst_p
			   *goal_bound_subst, bool complete);
LpairTreeParams InitSpecDomExpert(Lpair_p *tree, DNormSubst_p
			   *spec_bound_subst, bool complete, bool
			       all);

#endif


/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/





