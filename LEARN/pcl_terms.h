/*************************************************************************/
/*                                                                       */
/*   File:        pcl_terms.h                                            */
/*                                                                       */
/*   Autor:       Stephan Schulz                                         */
/*                                                                       */
/*   Inhalt:      Deklarationen zu Funktionen auf Termen und Termpaaren  */
/*                                                                       */
/*   Aenderungen: <1> 30.10.19191 neu                                    */
/*                                                                       */
/*************************************************************************/


#include "pcl_types.h"
#include "pcl_printer.h"

#ifndef _pcl_terms

#define _pcl_terms


/*----------------------------------------------------------------------------*/
/*                     Typen zu Termen                                        */
/*----------------------------------------------------------------------------*/

typedef struct termcell
{
   char*          id; 
   BOOL           isvar;
#ifdef LEARN_VERSION
   long           arity;
   long           norm_id; /* Norm-Identifier sind longs, die dann als */
			   /* f<num> bzw x<num> ausgegeben werden  */
   long           term_weight;
#endif
   struct termcell *chain;
   struct termcell *args;
} TermCell, *Term_p;

typedef struct paircell
{
   PairType        type;
   struct termcell *lside,
                   *rside;
} PairCell, *Pair_p;




/*----------------------------------------------------------------------------*/
/*             Deklaration exportierter Funktionen und Variablen              */
/*----------------------------------------------------------------------------*/

Term_p AllocTermCell();

void   FreeTermCell(Term_p junk);

void   FreeTerm(Term_p junk);

void   FreeArgs(Term_p junk);

void   FreeTermList(Term_p junk);

long   GetTermCellArity(Term_p term);

long   TermLength(Term_p prt);

long   TermDepth(Term_p prt);

long   NumberOfFuncs(Term_p term);

Pair_p AllocPairCell();

void   FreePairCell(Pair_p junk);

void   FreePair(Pair_p junk);

void   PrintTerm(Term_p prt);
void   PrintArgList(Term_p prt);

void   PrintTermPair(Pair_p prt);


Term_p CopyTerm(Term_p Term);

Term_p Subterm(Term_p term, NumList_p place);

Term_p Replace(Term_p term, NumList_p place, Term_p repterm);

Term_p ReplaceThis(Term_p term, Term_p repterm);

BOOL   TermEqual(Term_p term1, Term_p t2);

Term_p ConcatTermLists(Term_p tl1, Term_p tl2);

extern BOOL NoBrackets;



#endif

/*----------------------------------------------------------------------------*/
/*                         Ende des Files                                     */
/*----------------------------------------------------------------------------*/


