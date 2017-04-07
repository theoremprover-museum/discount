/*-------------------------------------------------------------------------

File        : learn_nterms.h

Autor       : Stephan Schulz

Inhalt      : Funktionen fuer Normterme und Normsubstitutionen zu
              DISCOUNT-Termen. 

Aenderungen : <1> 22.11.1994 neu

-------------------------------------------------------------------------*/

#ifndef _learn_nterms

#define _learn_nterms

#include "termpair.h"


/*-----------------------------------------------------------------------*/
/*                   Typdeklarationen                                    */
/*-----------------------------------------------------------------------*/


/* Diese Struktur ermoeglicht es, jedem Funktionssymbol in einem    */
/* DISCOUNT-Term eine Zahl als norm_id zuzuordnen. Der Vartree wird */
/* verwendet, um auch die Variablen zu normieren.                   */

/* norm_id[MAXFUNCTION] enthaelt an der Stelle i den dem           */
/*                      Funktionssymbol i zugeordneten Norm-id.    */
/*                      Fuer die TSM-Implementierung gehoert die   */
/*			Stelligkeit (FunctionInfo[i].arity         */
/*		        bzw. t->arity (wenn t ein poiner auf eine  */
/*			Termzelle ist) implizit zum Identifier.    */
/* f_count und v_count  enthalten die hoechsten bisher (in dieser  */
/*                      Substitution) vergebenen Identifier fuer   */
/*                      Funktionssymbole bzw. Variablen.           */
/* arity_id[MAXARITY]   Uebernimmt die Rolle von f_count fuer die  */
/*                      TSMs (Groesster Ident, der fuer eine       */
/*			bestimmte Stelligkeit vergeben wurde.      */
/* variables            stellt die Zuordnung zwischen normalen und */
/*                      Norm-Variablenbezeichnern her.             */
/* next                 dient nur zur Verkettung der Freilisten.   */


typedef struct dnormsubstcell
{
   short    norm_id[MAXFUNCTION];
   short    arity_id[MAXARITY];
   short    f_count;
   vartree* variables;
   long     v_count;
   struct   dnormsubstcell* next;
}DNormSubstCell, *DNormSubst_p;




/*-----------------------------------------------------------------------*/
/*   Deklaration exportierter Funktionen, Variablen (und Macros)         */
/*-----------------------------------------------------------------------*/


/* Allocation functions (well, Macros...). See newmem.h */

#define AllocDNormSubstCell() (DNormSubstCell*) \
        SizeMalloc(sizeof(DNormSubstCell))
#define FreeDNormSubstCell(junk) \
        SizeFree(junk, sizeof(DNormSubstCell))


DNormSubst_p AllocEmptyDNormSubst();

DNormSubst_p DNormTermVars(term* t, DNormSubst_p subst);
DNormSubst_p DNormTermFuns(term* t, DNormSubst_p subst);
DNormSubst_p DNormTerm(term* t, DNormSubst_p subst);

long         CmpNormTerms(term* t1, DNormSubst_p subst1, term* t2,
			  DNormSubst_p subst2); 

DNormSubst_p OrderNormTerms(term **t1, term **t2);

void         PrintDNormSubst(DNormSubst_p subst);
void         PrintDNormTerm(term* t, DNormSubst_p subst);

/* Use arity-preserving normidents (especially for TSMs) */
extern bool  NormTermPreserveArity;

#endif


/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/





