

/*************************************************************************/
/*                                                                       */
/*   File:        pcl_subst.h                                            */
/*                                                                       */
/*   Autor:       Stephan Schulz                                         */
/*                                                                       */
/*   Inhalt:      Deklarationen im Zusammenhang mit Substitutionen und   */
/*                verwandten Funktionen: match, mgu, Anwendung einer     */
/*                Substitution...                                        */
/*                                                                       */
/*   Aenderungen: <1> 17.2.1992 neu                                      */
/*                                                                       */
/*************************************************************************/

#include "pcl_parse.h"


#ifndef _pcl_subst

#define _pcl_subst


/*----------------------------------------------------------------------------*/
/*                 Typ-Deklarationen                                          */
/*----------------------------------------------------------------------------*/

typedef struct substcell
{
   char*             varid;
   Term_p            sterm;
   struct substcell* chain;
}SubstCell, *Subst_p;


/*-----------------------------------------------------------------------*/
/*                 Exportierte Variable                                  */
/*-----------------------------------------------------------------------*/


extern BOOL PrologVars;




/*----------------------------------------------------------------------------*/
/*                 Deklaration exportierter Funktionen                        */
/*----------------------------------------------------------------------------*/

Subst_p AllocSubstCell();
void    FreeSubstCell(Subst_p junk);
void    FreeSubst(Subst_p junk);

Subst_p ApplySubstSubst(Subst_p ssb, Subst_p subst);

Subst_p CopySubst(Subst_p subst);

void    PrintSubst(Subst_p prt);

Term_p  ApplySubst(Term_p term, Subst_p subst);

Subst_p ConcatSubst(Subst_p outer, Subst_p inner);

BOOL    Match(Term_p mterm, Term_p target, Subst_p* subst);

BOOL    Mgu(Term_p term1, Term_p term2, Subst_p* subst);

Subst_p UniqSubst(Subst_p yet, Term_p term);

Subst_p PrettySubst(Subst_p yet, Term_p term);

Term_p  MakeUniq(Term_p term);

Term_p  MakePretty(Term_p term);

Pair_p  MakePrettyPair(Pair_p pair);

Subst_p VarList(Subst_p yet, Term_p term);

#endif

/*----------------------------------------------------------------------------*/
/*                         Ende des Files                                     */
/*----------------------------------------------------------------------------*/


