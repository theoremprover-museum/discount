/*************************************************************************/
/*                                                                       */
/*   File:        pcl_parse.h                                            */
/*                                                                       */
/*   Autor:       Stephan Schulz                                         */
/*                                                                       */
/*   Inhalt:      Parser fuer pcl                                        */
/*                                                                       */
/*   Aenderungen: <1> 03.4.1991  Uebernahme von parse.h                  */
/*                                                                       */
/*************************************************************************/

#ifndef _pcl_parse

#define _pcl_parse

#include "pcl_types.h"
#include "pcl_terms.h"


/*----------------------------------------------------------------------------*/
/*                  Typdeklarationen                                          */
/*----------------------------------------------------------------------------*/

typedef union argtype
{
   struct justcell* rarg;
   struct targ
   {
      NumList_p        sarg;
      struct stepcell* parg;
   }Targ;
}JArg_p;


typedef struct justcell
{
   OpType     operation;
   JArg_p     arg1;
   Place_p    place1;
   JArg_p      arg2;
   Place_p    place2;
}JustCell,*Just_p;


typedef struct step_plistcell
{
   struct stepcell       *this;
   struct step_plistcell *pred;
   struct step_plistcell *succ;
}Step_pListCell,*Step_pList_p;


typedef struct stepcell
{
   NumList_p          id;             /* Identifier */
   StepType           type;           /* Type (tes-rule,tes-eqn...) */
   Pair_p             pair;           /* Pointer to Term Pair (the fact) */
   Step_pList_p       children;       /* List of Pointers to children */
   long               children_no;    /* Number of children */
#ifdef LEARN_VERSION
   long               goal_dist;      /* The LONGEST way from this */
				      /* step to a tes-final - useful */
				      /* for evaluating a step, but */
				      /* might also be useful for */
				      /* lemma generation... */
#endif
   BOOL               used;
   long               rw_distance;    /* longest distance in
					 rewrite-Steps from a
					 contributing step to this one
					 */ 
   long               cp_distance;    /* Ditto in number of
					 critical-pair-inferences */ 
   long               cp_cost;        /* Number of useless cps
					 generated from this fact (or
					 0, if the fact is necessary)
					 */
   long               op_count;       /* Weighted sum of Operations in proof tree */
   Just_p             just;           /* Pointer to justification of fact */
   long               chain_len;      /* Length of proof chain for fact */
   NumListList_p      tree_exits;     /* Exits of the proof tree below step */
   NumListList_p      used_steps;     /* Used to determine isolated subtrees */
   char*              comment;        /* Exactly that   */
   char*              name;           /* If axiom or lemma can get a name of its own */
   long               ax_lem_no;      /* ... and certainly has a number for reference */
   struct eqchaincell *eqproof;       /* Pointer to proof chain */
   struct stepcell    *pred;
   struct stepcell    *succ;
}StepCell,*Step_p;


/*----------------------------------------------------------------------------*/
/*                 Forward-Deklarationen exportierter Funktionen              */
/*----------------------------------------------------------------------------*/

extern StringCell ErrCell;

Just_p       AllocJustCell();
Step_p       AllocStepCell();
void         FreeJustCell(Just_p junk);
void         FreeJust(Just_p junk);
void         FreeStepCell(Step_p junk);
void         FreeStep(Step_p junk);
Step_pList_p AllocStep_pListCell();
void         FreeStep_pListCell(Step_pList_p junk);
void         FreeStep_pList(Step_pList_p junk);

void PrintPlace(Place_p prt);

void PrintJust(Just_p prt);

void PrintStepPure(Step_p prt);
void PrintStep(Step_p prt);

void   NextRealToken();
long   ParseInt();
Step_p ParseStep();

/* Die folgenden Funktionen wurden nachtraeglich exportiert, um zum */
/* Parsen fuer die Lern-Version wiederverwendet zu werden!          */

Term_p    parse_term();

void      AcceptTok(TokenType tok,char* lit);
void      check(TokenType tok, char* lit);
BOOL      test(TokenType tok);

void      AcceptId(TokenType tok,char* id);
void      check_id(TokenType tok,char* id);
BOOL      test_id(TokenType tok,char* id);



#endif

/*----------------------------------------------------------------------------*/
/*                         Ende des Files                                     */
/*----------------------------------------------------------------------------*/


