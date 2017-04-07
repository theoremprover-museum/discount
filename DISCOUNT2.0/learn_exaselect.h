/*-------------------------------------------------------------------------

File        : learn_exaselect.h

Autor       : Felix Brandt

Inhalt      : Functions for example selection.

Aenderungen : <1> 9.3.1998 new
              <2> 30.6.1998 bintree for example sorting

-------------------------------------------------------------------------*/

#ifndef _learn_exaselect

#define _learn_exaselect

#include "learn_lpairs.h"
#include "learn_tsm.h"

/*-----------------------------------------------------------------------*/
/*                   Typdeklarationen                                    */
/*-----------------------------------------------------------------------*/

#define delta(x,y)  ((!x && !y) ? 0 : (ABS((double)x-(double)y)/max((double)x,(double)y)))

typedef struct treecell
{
   struct examplecell* example;  /* Pointer to Example */
   double fitness;               /* Used for sorting */
   struct treecell*    left;
   struct treecell*    right;
}TreeCell, *exampletree_p;

/*-----------------------------------------------------------------------*/
/*                  Deklaration exportierter Variablen                   */
/*-----------------------------------------------------------------------*/

extern bool   no_exasel;
extern long   maxexamples;
extern double maxdelta,
              weightNA, weightAD, weightDD, weightGD, weightAF, weightTSM;

/*-----------------------------------------------------------------------*/
/*                 Deklaration exportierter Funktionen                   */
/*-----------------------------------------------------------------------*/

#define AllocTreeCell() (TreeCell*)\
        SizeMalloc(sizeof(TreeCell))

example_p  GetExaList(char* PathNameHelp, bool select);
bool       IsEmptyIntersection(example_p list1, example_p list2);

#endif

/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/





