/*-------------------------------------------------------------------------

File        : learn_mixterms.h

Autor       : Stephan Schulz

Inhalt      : Functions dealing with both lterms and nterms (that is:
              DISCOUNT terms, possibly extended by DNormSubsts, and
	      the arbitrary arity terms inherited from the pcl
	      programs and necessary for the knowledge base
	      maintainance. 

Aenderungen : <1>

-------------------------------------------------------------------------*/

#ifndef _learn_mixterms

#define _learn_mixterms

#include "term.h"
#include "parsedom.h"
#include "learn_nterms.h"
#include "learn_lpairs.h"



/*-----------------------------------------------------------------------*/
/*                 Deklaration exportierter Funktionen                   */
/*-----------------------------------------------------------------------*/


long      CmpLtermNterm(Lterm_p lterm, term* nterm, DNormSubst_p subst);
Lpair_p   FindTwoNtermsLtermTree(term* lside, term* rside, DNormSubst_p
			       subst, Lpair_p root);
term*     LtermToDomTerm(Lterm_p term, LearnSig_p sig);
Lterm_p   NtermToLterm(term* t, DNormSubst_p subst);
termpair* LpairToDomPair(Lpair_p pair, LearnSig_p sig);

void      LpairTreeAdd(Lpair_p tree, LearnSig_p sig, pairset *set);

#endif

/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/





