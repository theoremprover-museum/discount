/*-------------------------------------------------------------------------

File        : lrn_eqnoccur.h

Autor       : Stephan Schulz
 
Inhalt      : Deklarationen zu Listen von Vorkommen (von Gleichungen)
              in Beweisen. 

Aenderungen : <1> 10.8.1994 Ausgelagert aus lrn_eqn.h

-------------------------------------------------------------------------*/

#ifndef _lrn_eqnoccur

#define _lrn_eqnoccur


#include "pcl_doio.h"


/*-----------------------------------------------------------------------*/
/*                   Typdeklarationen                                    */
/*-----------------------------------------------------------------------*/

typedef struct eqnoccurcell
{
   char* example;
   BOOL  used;
   long  cp_cost;
   long  references;
   long  goal_dist;
   struct eqnoccurcell *next;
}EqnOccurCell,*EqnOccur_p;



/*-----------------------------------------------------------------------*/
/*                 Deklaration exportierter Funktionen                   */
/*-----------------------------------------------------------------------*/

EqnOccur_p AllocEqnOccurCell();
void       FreeEqnOccurCell(EqnOccur_p junk);
void       FreeEqnOccurList(EqnOccur_p junk);

EqnOccur_p CopyEqnOccur(EqnOccur_p source);
EqnOccur_p CopyEqnOccurList(EqnOccur_p source);

long       CmpEqnOccurs(EqnOccur_p occ1, EqnOccur_p occ2);
EqnOccur_p FindEqnOccur(EqnOccur_p occ, EqnOccur_p list);
EqnOccur_p FindEqnOccurName(EqnOccur_p occ, EqnOccur_p list);
     /* Sucht nur nach dem Namen! */
EqnOccur_p MergeEqnOccurLists(EqnOccur_p list1, EqnOccur_p list2, BOOL
			      names_only); 

EqnOccur_p EqnOccurNameIntersect(EqnOccur_p list1, EqnOccur_p list2);

long        AverageGoalDist(EqnOccur_p list);
long        AverageReferences(EqnOccur_p list);
long        TotalReferences(EqnOccur_p list);


void PrintEqnOccurList(EqnOccur_p occur); /* Komplette Infos */
void PrintOccurNames(EqnOccur_p occur);   /* Nur die Liste der */
					  /* Beispiele */

extern BOOL PosAndNeg;

#endif


/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/





