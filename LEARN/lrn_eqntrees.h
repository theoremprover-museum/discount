/*-------------------------------------------------------------------------

File        : lrn_eqntrees.h

Autor       : Stephan Schulz

Inhalt      : Deklarationen zu Funktionen zum Umgang mit AVL-Baeumen
              ueber NormEqns. 

Aenderungen : <1> 10.8.1994 neu

-------------------------------------------------------------------------*/

#ifndef _lrn_eqntrees

#define _lrn_eqntrees

#include "lrn_eqn.h"


/*-----------------------------------------------------------------------*/
/*                   Typdeklarationen                                    */
/*-----------------------------------------------------------------------*/




/*-----------------------------------------------------------------------*/
/*                 Deklaration exportierter Funktionen                   */
/*-----------------------------------------------------------------------*/


void      FreeEqnTree(NormEqn_p junk);

BOOL      TreeInsertEqn(NormEqn_p *root, NormEqn_p eqn);
NormEqn_p TreeFindEqn(NormEqn_p eqn, NormEqn_p root);

NormEqn_p EqnListToTree(NormEqn_p* tree, NormEqn_p list);
NormEqn_p TreeToSortList(NormEqn_p tree, NormEqn_p list);

NormEqn_p MergeEqnTrees(NormEqn_p* tree, NormEqn_p new);

void      PartNormEqnTree(NormEqn_p tree, NormSubst_p subst);

void      PrintEqnTree(NormEqn_p root);
void      PrintOptEqnTree(NormEqn_p root);

#endif


/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/





