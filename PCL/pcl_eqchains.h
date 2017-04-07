

/*************************************************************************/
/*                                                                       */
/*   File:        pcl_eqchains.h                                         */
/*                                                                       */
/*   Autor:       Stephan Schulz                                         */
/*                                                                       */
/*   Inhalt:      Deklarationen zu Gleichheitsketten                     */
/*                                                                       */
/*   Aenderungen: <1> 30.1.1991                                          */
/*                                                                       */
/*************************************************************************/

#include "pcl_subst.h"


#ifndef _pcl_eqchains

#define _pcl_eqchains



/*----------------------------------------------------------------------------*/
/*                  Typ-Deklarationen zu Gleichheitsketten                    */
/*----------------------------------------------------------------------------*/

typedef struct eqchaincell 
{
   Step_p             tp;
   NumList_p          place;
   Subst_p            sigma;
   BOOL               swapped;
   struct eqtermcell* left;
   struct eqtermcell* right;
}EqChainCell,*EqChain_p;


typedef struct eqtermcell
{
   Term_p term;
   struct eqchaincell* left;
   struct eqchaincell* right;
}EqTermCell,*EqTerm_p;


/* Gleichheitsketten bestehen abwechselnd aus EqTermCells und
   EqChainCells. Die eigentliche Kette wird von zwei EqTermCells
   begrenzt, um einen eindeutigen und symmetrischen Zugriff zu
   erlauben, wird eine zusaetzliche EqChainCell eingefuehrt,
   die die Kette zu einem Ring vollendet. In ihr stehen nur
   Pointer auf die erste und die letzte EqTermCell, die anderen
   Felder sind unbelegt.                                         */



/*----------------------------------------------------------------------------*/
/*                  Exportierte Funktionen                                    */
/*----------------------------------------------------------------------------*/

EqChain_p AllocEqChainCell();

void      FreeEqChainCell(EqChain_p junk);

EqTerm_p  AllocEqTermCell();

void      FreeEqTermCell(EqTerm_p junk);

void      FreeEqChain(EqChain_p junk);

void      PrintEqChain(EqChain_p prt,BOOL prsubst,BOOL prplace,BOOL prstep);

EqChain_p CopyEqChain(EqChain_p chain);

EqChain_p ConcatEqChains(EqChain_p chain1,EqChain_p chain2);

EqChain_p SwapEqChain(EqChain_p chain);

void      CutEqChain(EqChain_p source, EqChain_p cut,EqChain_p *stt, EqChain_p *rst);

EqChain_p CleanEqChain(EqChain_p source);

EqChain_p S(Term_p w, NumList_p p, Subst_p tau, EqChain_p chain);

Subst_p   EqChainVars(Subst_p yet,EqChain_p chain);



#endif


/*----------------------------------------------------------------------------*/
/*                         Ende des Files                                     */
/*----------------------------------------------------------------------------*/





