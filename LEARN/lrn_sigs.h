/*-------------------------------------------------------------------------

File        : lrn_sigs.h

Autor       : Stephan Schulz

Inhalt      : Deklarationen zu (Norm-)Signaturen

Aenderungen : <1> 2.9.1994

-------------------------------------------------------------------------*/

#ifndef _lrn_sig

#define _lrn_sig


#include "lrn_eqn.h"


/*-----------------------------------------------------------------------*/
/*                          Macros & Konstanten                          */
/*-----------------------------------------------------------------------*/

#define SIG_IN "          "   /* Signature terms will be prepended by */
                              /* this... */

#define SigEquivalent(sig1,sig2)\
(SigIsPart((sig1),(sig2)) && SigIsPart((sig2),(sig1)))

/*-----------------------------------------------------------------------*/
/*                   Typdeklarationen                                    */
/*-----------------------------------------------------------------------*/


typedef struct symbolcell
{
   char*              id;
   long               arity;
   long               norm_id;
   BOOL               picked;
   struct symbolcell* next;
}SymbolCell,*Symbol_p;



/*-----------------------------------------------------------------------*/
/*                 Deklaration exportierter Funktionen                   */
/*-----------------------------------------------------------------------*/

Symbol_p AllocSymbolCell();
void     FreeSymbolCell(Symbol_p junk);
void     FreeSymbolList(Symbol_p junk);

Symbol_p FindSymbolCell(char* id, Symbol_p sig);
long     GetArity(char* id, Symbol_p sig);

Symbol_p GetFuns(Term_p term, Symbol_p sig);
Symbol_p GetSig(NormEqn_p pairlist, Symbol_p sig);
void     UnpickSig(Symbol_p sig);

void     PrintSig(Symbol_p sig);
 
BOOL     SigIsPart(Symbol_p sig1, Symbol_p sig2);

NormSubst_p EqnListSubsum(NormEqn_p list1, Symbol_p sig1, NormEqn_p
			  list2, Symbol_p sig2);
NormSubst_p EqnListEquiv(NormEqn_p list1, Symbol_p sig1, NormEqn_p
			 list2, Symbol_p sig2);

long     GetNumberOfArity(Symbol_p sig, long arity);
long     GetMaxArity(Symbol_p sig);

#endif


/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/





