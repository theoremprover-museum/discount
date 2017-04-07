/*-------------------------------------------------------------------------

File        : learn_lterms.h

Autor       : Stephan Schulz

Inhalt      : This files describes the implementation of norm-terms
              for the representation of the learned knowledge. These
	      terms differ from the DISCOUNT terms - each function
	      symbol is represented by a long integer, and may have
	      arities that change between terms (theoretically even
	      within terms...). They are derived from the pcl-terms
	      and do not require any signature information to decode
	      (however, (partial) signatures may be useful for the
	      later case-matching algorithms). 

Aenderungen : <1> 7.12.1994 neu

-------------------------------------------------------------------------*/

#ifndef _learn_lterms

#define _learn_lterms

#include "newmem.h"
#include "s_scanner.h"
#include "vartree.h"
#include "learn_nterms.h"

/*-----------------------------------------------------------------------*/
/*                   Typdeklarationen                                    */
/*-----------------------------------------------------------------------*/


typedef struct ltermcell
{
   long            norm_id;    /* Ident print-form of function symbol  */
			       /* is fnorm_id (farity_norm_id if       */
   short           arity;      /* NormTermPreserveArity is true)       */
   bool            isvar;      
   long            term_weight; /* 1 for variables, 2+sum over */
				/* arguments otherwise...used for */
				/* efficient search and reduction */
				/* tests */
   struct ltermcell *chain;     /* Chaining arguments, "cdr" */
   struct ltermcell *args;      /* Pointer to argument list */
}LtermCell, *Lterm_p;


/* Data structures for abstraction=tree based learning...perhaps I'll */
/* finish it yet ;-) */


typedef struct tet_cell
{  /* A full abstraction tree - the top function symbol of the term is */
   /* not yet fixed. This is done by keeping an array of all operators */
   /* possible here... */

   struct tat_cell    *branches[MAXARITY];
/*   long               variables;*/
}TETCell, *TET_p;


typedef struct tat_cell
{ /* An tree whose top operator is fixed, but whose arguments may be */
  /* full abstraction trees. */
   long            proofs,
                   tot_ref,
                   count;
   double          proofs_r,
                   tot_ref_r,
                   count_r,
                   cell_weight;
   TET_p           args[MAXARITY];
   
}TATCell, *TAT_p;



/*-----------------------------------------------------------------------*/
/*                 Deklaration exportierter Funktionen                   */
/*-----------------------------------------------------------------------*/


#define AllocLtermCell() (LtermCell*)\
        SizeMalloc(sizeof(LtermCell))
#define FreeLtermCell(junk)\
        SizeFree(junk, sizeof(LtermCell))

#define AllocTETCell() (TETCell*)\
        SizeMalloc(sizeof(TETCell))
#define FreeTETCell(junk)\
        SizeFree(junk, sizeof(TETCell))

#define AllocTATCell() (TATCell*)\
        SizeMalloc(sizeof(TATCell))
#define FreeTATCell(junk)\
        SizeFree(junk, sizeof(TATCell))

extern void    FreeLterm(Lterm_p junk);

extern Lterm_p ParseLterm();
extern long    CmpLterms(Lterm_p term1, Lterm_p term2);
extern void    PrintLterm(Lterm_p term);
extern short   LtermArity(Lterm_p term);

extern TAT_p   AllocEmptyTAT();
extern TET_p   AllocEmptyTET();

extern void    FreeTET(TET_p junk);
extern void    FreeTAT(TAT_p junk);
extern void    PrintTET(TET_p tree);
extern void    PrintTAT(TAT_p tree);

extern void    LtermTATInsert(TAT_p *tree, Lterm_p t, long proofs,
			      long tot_ref);
extern void    LtermTETInsert(TET_p *tree, Lterm_p t, long proofs,
			      long tot_ref);
#endif


/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/





