/*-----------------------------------------------------------------------

File  : pcl_miniparse.h

Author: Stephan Schulz

Contents
 
  Parser fuer pcl

Changes

<1> 07.9.1991 Uebernahme von pcl_parse.h 
<2> 30.9.1996 Neue Boxen, PrintAnnotations eingef"uhrt

-----------------------------------------------------------------------*/

#ifndef _pcl_miniparse

#define _pcl_miniparse

#include "pcl_types.h"
#include "pcl_miniterms.h"



/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

typedef union miniargtype
{
   struct minijustcell* rarg;
   NumList_p            targ;
}miniJArg_p;


typedef struct minijustcell
{
   OpType     operation;
   miniJArg_p arg1;
   char*      place1;
   miniJArg_p arg2;
   char*      place2;
}miniJustCell,*miniJust_p;


typedef struct ministepcell
{
   NumList_p  id;
   StepType   type; 
   char*      pair;
   miniJust_p just;
   char*      comment;
   BOOL       used;
   long       rw_distance;  /* longest distance in rewrite-Steps from a
			       contributing step to this one */
   long       cp_distance;  /* Ditto in number of
			      critical-pair-inferences */
   long       trivial;      /* Is the step trivial and has only
			       trivial successors? */
   long       cp_cost;      /* Number of useless cps generated from
			       this fact (or 0, if the fact is
			       necessary) */
   BOOL       generated_cps;
   struct ministepcell *pred;
   struct ministepcell *succ;
}miniStepCell,*miniStep_p;


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


extern BOOL PrintAnnotations;

miniJust_p AllocminiJustCell();
miniStep_p AllocminiStepCell();
void       FreeminiJust(miniJust_p junk);
void       FreeminiJustCell(miniJust_p junk);
void       FreeminiStep(miniStep_p junk);
void       FreeminiStepCell(miniStep_p junk);

BOOL IsTrivial(char* pair);
BOOL IsRedundant(miniStep_p step);

void PrintPlace(char* prt);

void PrintJust(miniJust_p prt);

void PrintStep(miniStep_p prt);


void       NextRealToken();
long       ParseInt();
miniStep_p ParseStep();


#endif

/*----------------------------------------------------------------------------*/
/*                         Ende des Files                                     */
/*----------------------------------------------------------------------------*/


