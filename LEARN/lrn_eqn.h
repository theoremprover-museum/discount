/*-------------------------------------------------------------------------

File        : lrn_eqn.h

Autor       : Stephan Schulz

Inhalt      : Deklarationen und Datentypen zu Gleichungen und
              Vorkommen der Gleichungen in Beweisen - die
	      Grund-Datenstruktur zum Beweislernen. Signaturen werden
	      ebenfalls behandelt.

Aenderungen : <1> 1.7.1994 neu

-------------------------------------------------------------------------*/

#ifndef _lrn_eqn

#define _lrn_eqn

#include "lrn_norm.h"
#include "lrn_eqnoccur.h"


/*-----------------------------------------------------------------------*/
/*                   Typdeklarationen                                    */
/*-----------------------------------------------------------------------*/


typedef enum 
{
   equal,           /* The two lists are equal */
   incomparable,    /*          ...      incomparable */
   subset,          /* List 1 is a (real) subset of list 2 */
   setsub           /* List 2 is a (real) subset of list 1 */
}CmpResult;


/* NormEqn-Listen sind, wie Step-Listen, doppelt verkettet und haben */
/* einen Anker, der nicht zur Liste gehoert. left und right koennen   */
/* auch zum Aufbau eines Baumes verwendet werden.                     */

typedef struct normeqncell
{
   Term_p               lside;
   Term_p               rside;
   EqnOccur_p           occur;
   long                 ave_ref;
   long                 tot_ref;
   long                 goal_dist;
   long                 balance;  /* Fuer AVL-Baeume */
   struct normeqncell*  left;
   struct normeqncell*  right;
}NormEqnCell,*NormEqn_p;


/*-----------------------------------------------------------------------*/
/*                 Deklaration exportierter Funktionen                   */
/*-----------------------------------------------------------------------*/


NormEqn_p  AllocNormEqnCell();
void       FreeNormEqnCell(NormEqn_p junk);
void       FreeNormEqn(NormEqn_p junk);
void       FreeNormEqnList(NormEqn_p junk);


NormEqn_p InsertFirstNormEqnList(NormEqn_p list, NormEqn_p cell);
NormEqn_p InsertLastNormEqnList(NormEqn_p list, NormEqn_p cell);
NormEqn_p RemoveFirstNormEqnList(NormEqn_p list);
long      EqnListSize(NormEqn_p list);

NormEqn_p SortNormEqnListInPlace(NormEqn_p list);
NormEqn_p SortNormEqnList(NormEqn_p list);

long      CmpNormEqns(NormEqn_p eqn1, NormEqn_p eqn2);

NormEqn_p CopyNormEqn(NormEqn_p eqn);

BOOL        OrientNormEqn(NormEqn_p eqn, BOOL renorm);
void        OrientNormEqns(NormEqn_p anchor);
NormSubst_p NormEqn(NormEqn_p eqn, NormSubst_p subst);
NormSubst_p NormEqnList(NormEqn_p anchor, NormSubst_p subst);
void        PartNormEqnList(NormEqn_p anchor, NormSubst_p subst);

NormSubst_p NormalizeEqnList(NormEqn_p list);

CmpResult   CmpSortedEqnLists(NormEqn_p list1, NormEqn_p list2);

void PrintNormEqn(NormEqn_p eqn);
void PrintNormEqnLine(NormEqn_p eqn);
void PrintNormEqnList(NormEqn_p anchor, BOOL add);


#endif


/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/





