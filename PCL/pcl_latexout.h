

/*************************************************************************/
/*                                                                       */
/*   File:        pcl_latexout.h                                         */
/*                                                                       */
/*   Autor:       Stephan Schulz                                         */
/*                                                                       */
/*   Inhalt:      Definitionen zur Beweisausgabe in LaTeX                */
/*                                                                       */
/*   Aenderungen: <1> neu                                                */
/*                                                                       */
/*************************************************************************/

#include "pcl_eqchains.h"


#ifndef _pcl_latexout

#define _pcl_latexout


/*----------------------------------------------------------------------------*/
/*                    Deklaration exportierter Funktionen                     */
/*----------------------------------------------------------------------------*/

void L_PrintTerm(Term_p prt,BOOL markmatch,NumList_p match,BOOL markrep,NumList_p rep);

void L_PrintEqChain(EqChain_p prt,BOOL prsubst,BOOL prplace);

void L_PrintSubst(Subst_p prt);


#endif

/*----------------------------------------------------------------------------*/
/*                         Ende des Files                                     */
/*----------------------------------------------------------------------------*/


