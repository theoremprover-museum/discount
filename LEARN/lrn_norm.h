/*-------------------------------------------------------------------------

File        : lrn_norm.h

Autor       : Stephan Schulz

Inhalt      : Deklarationen zu normierten Termen  d.h.
              Termen, in denen die Funktionssymbole in aufsteigender
	      Reihenfolge auftreten.

Aenderungen : <1> 17.6.1994 neu

-------------------------------------------------------------------------*/

#ifndef _lrn_norm

#define _lrn_norm

#include "pcl_terms.h"
#include "pcl_doio.h"

/*-----------------------------------------------------------------------*/
/*                         Typ-Deklarationen                             */
/*-----------------------------------------------------------------------*/

/* Keep administration information about the identifiers already
   assigned for each arity. This basically simulates an endless
   array. */

#define DEFAULT_MAX_ARITY 10

typedef struct normadmincell
{
  long *array;
  long size;
}NormAdminCell, *NormAdmin_p;

/* Store the association between external identifiers and norm-idents
   (currently integers, but this will change. Full normsubstitutions
   are linear lists of these cells */

typedef struct normsubstcell
{
   char*                id;
   long                 arity;    /* Added 14.12.1996 for new foramt
				     of normindents that makes them
				     maintain the same arity even for
				     independend terms or
				     equations. */
   long                 norm_id;  /* Positive Zahlen... */
   long                 v_count;  /* Vergebene Variablen - nur im Kopf*/
   NormAdmin_p          f_admin;  /* Vergebene Funktionssymbole */

   struct normsubstcell *next;
}NormSubstCell,*NormSubst_p;


/*-----------------------------------------------------------------------*/
/*         Deklaration exportierter Funktionen und Variablen             */
/*-----------------------------------------------------------------------*/


extern BOOL PreserveArity;


NormAdmin_p AllocNormAdminCell();
void        FreeNormAdminCell(NormAdmin_p junk);
NormAdmin_p CreateNormAdmin();
void        FreeNormAdmin(NormAdmin_p junk);
NormAdmin_p CopyNormAdmin(NormAdmin_p source);
long        SetNormAdminVal(NormAdmin_p cell, long arity, 
			      long value);
long        GetNormAdminVal(NormAdmin_p cell, long arity);
long        IncNormAdminVal(NormAdmin_p cell, long arity);


NormSubst_p AllocNormSubstCell();
void        FreeNormSubstCell(NormSubst_p junk);
void        FreeNormSubst(NormSubst_p junk);

NormSubst_p FunNormTermList(Term_p t_list, NormSubst_p subst, 
			    BOOL modify); 
NormSubst_p VarNormTermList(Term_p t_list, NormSubst_p subst, 
			    BOOL modify);

NormSubst_p NormTermList(Term_p t_list, NormSubst_p subst, BOOL modify);

NormSubst_p CopyNormSubst(NormSubst_p subst);

long        WeightTerm(Term_p term);

Term_p      NormTerm(Term_p term);

Term_p      ApplyNormSubst(Term_p term, NormSubst_p subst);

long        CmpNormTerms(Term_p term1, Term_p term2);

void        DebugPrintNormSubst(NormSubst_p subst);
void        PrintNormTerm(Term_p term);
void        PrintNormArgList(Term_p list);

#endif

/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/





