


/*************************************************************************/
/*                                                                       */
/*   File:        pcl_eqchains.c                                         */
/*                                                                       */
/*   Autor:       Stephan Schulz                                         */
/*                                                                       */
/*   Inhalt:      Funktionen zu Gleichheitsketten                        */
/*                                                                       */
/*   Aenderungen: <1> 30.1.1991                                          */
/*                                                                       */
/*************************************************************************/

#include "pcl_eqchains.h"




/*----------------------------------------------------------------------------*/
/*                   Forward-Deklaration interner Funktionen                  */
/*----------------------------------------------------------------------------*/

void print_eq_just(EqChain_p prt, BOOL prsubst,BOOL prplace,BOOL prstep);


/*----------------------------------------------------------------------------*/
/*                     Interne Funktionen                                     */
/*----------------------------------------------------------------------------*/


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void print_eq_just(EqChain_p prt, BOOL prsubst,         */
/*                                       BOOL prplace,BOOL prstep)            */
/*                    IN     EqChain_p prt                                    */ 
/*                    IN     BOOL      prsubst                                */
/*                    IN     BOOL      prplace                                */
/*                    IN     BOOL      prstep                                 */
/*                                                                            */
/* Beschreibung     : Gibt Begruendung fuer einen einzelnen Gleichheitschritt */ 
/*                    mit den gew"unschten Informationen aus.                 */
/*                                                                            */
/* Globale Variable : out (aus pcl_printer)                                   */
/*                                                                            */
/* Seiteneffekte    : -                                                       */
/*                                                                            */
/* Aenderungen      : <1> 31.1.1992 neu                                       */
/*                                                                            */
/******************************************************************************/

void print_eq_just(EqChain_p prt, BOOL prsubst,BOOL prplace,BOOL prstep)
{
   if(prstep)
   {
      fprintf(out,"  using equation ");
      PrintNumList(prt->tp->id); 
      if(prt->tp->ax_lem_no)
      {
         if(prt->tp->type == tes_lemma)
         {
            fprintf(out," (Lemma %ld) ",prt->tp->ax_lem_no);
         }
         else
         {
            fprintf(out," (Axiom %ld) ",prt->tp->ax_lem_no);
         }
      }
   }
   else
   {
      if(prt->tp->type == tes_lemma)
      {
         fprintf(out, " by Lemma %ld ",prt->tp->ax_lem_no);
      }
      else if(prt->tp->just->operation == initial)
      {
         fprintf(out," by Axiom %ld ",prt->tp->ax_lem_no);
      }
      else if(prt->tp->type == tes_final)
      {
	 fprintf(out," by Theorem %ld ",prt->tp->ax_lem_no);
      }
      else
      {
         Error("Non-terminal equation used in print_eq_just (pcl_eq-chains.c)");
      }
   }
   if(prt->swapped)
   {
      fprintf(out,"RL ");
   }
   else
   {
      fprintf(out,"LR ");
   }
   if(prplace)
   {
      fprintf(out,"at ");
      PrintNumList(prt->place);
   }
   if(prsubst)
   {
      fprintf(out," with ");
      PrintSubst(prt->sigma);
   }
}


/*----------------------------------------------------------------------------*/
/*                  Exportierte Funktionen                                    */
/*----------------------------------------------------------------------------*/

MakeAlloc(EqChainCell);

MakeFree(EqChainCell);

MakeAlloc(EqTermCell);

MakeFree(EqTermCell);


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void FreeEqChain(EqChain_p junk)                        */
/*                    IN     EqChain_p junk                                   */
/*                                                                            */
/* Beschreibung     : Gibt eine Gleichheitskette an die Speicherverwaltung    */
/*                    zurueck.                                                */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Durch FreeTerm,FreeSubst,FreeNumList,...                */
/*                                                                            */
/* Aenderungen      : <1> 30.1.1992                                           */
/*                                                                            */
/******************************************************************************/

void FreeEqChain(EqChain_p junk)
{
   EqChain_p chainhandle;
   EqTerm_p  termhandle;
   
   if(!junk)
   {
      fprintf(stderr,"Warning: NULL-Pointer returned to FreeEqChain (pcl_eqchains.c)...\n");
      return;
   }
   else
   {
      termhandle = junk->right;
      chainhandle = termhandle->right;
      FreeTerm(termhandle->term);
      FreeEqTermCell(termhandle);
      while(chainhandle != junk)
      {
         termhandle = chainhandle->right;
         FreeNumList(chainhandle->place);
         FreeSubst(chainhandle->sigma);
         FreeEqChainCell(chainhandle);
         /*  tp ist nur eine weitere Referenz auf einen PCL-Schritt im Listing */
         chainhandle = termhandle->right;
         FreeTerm(termhandle->term);
         FreeEqTermCell(termhandle);
      }
   }
   FreeEqChainCell(junk);
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void PrintEqChain(EqChain_p prt,BOOL prsubst,           */
/*                                      BOOL prplace, BOOL prstep)            */ 
/*                    IN     EqChain_p prt                                    */
/*                    IN     BOOL      prsubst                                */
/*                    IN     BOOL      prplace                                */
/*                    IN     BOOL      prstep                                 */
/*                                                                            */
/* Beschreibung     : Formatierte Ausgabe einer Gleichheitskette mit den      */
/*                    gewuenschten Informationen                              */
/*                                                                            */
/* Globale Variable : out (aus pcl_printer)                                   */
/*                                                                            */
/* Seiteneffekte    : -                                                       */
/*                                                                            */
/* Aenderungen      : <1> 31.1.1992 neu                                       */
/*                                                                            */
/******************************************************************************/

void PrintEqChain(EqChain_p prt,BOOL prsubst,BOOL prplace, BOOL prstep)
{
   EqTerm_p termhandle;
   EqChain_p chainhandle;
   long indent,f;

   termhandle = prt->right;
   chainhandle = termhandle->right;
   indent = TermLength(termhandle->term);
   if(!prstep)
   {
      fprintf(out,"    ");
      indent += 4;
   }
   PrintTerm(termhandle->term);
   if(chainhandle == prt)
   {
      putc('\n',out);
      fprintf(stderr,"Warning: Isolated term in PrintEqChain (pcl_eqchains.c)...\n");
      return;
   }
   else
   {
      termhandle = chainhandle->right;
      fprintf(out," = ");
      PrintTerm(termhandle->term);
      print_eq_just(chainhandle,prsubst,prplace,prstep);
      putc('\n',out);
      chainhandle = termhandle->right;
      while(chainhandle != prt)
      {
         termhandle = chainhandle->right;
         for(f=1;f<=indent;f++)
         {
            putc(' ',out);
         }
         fprintf(out," = ");
         PrintTerm(termhandle->term);
         print_eq_just(chainhandle,prsubst,prplace,prstep);
         putc('\n',out);
         chainhandle = termhandle->right;
      }
   }
}

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : EqChain_p CopyEqChain(EqChain_p chain)                  */
/*                    IN        EqChain_p chain                               */ 
/*                                                                            */
/* Beschreibung     : Kopiert eine Gleichheitskette                           */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : -                                                       */
/*                                                                            */
/* Aenderungen      : <1> 3.2.1992                                            */
/*                                                                            */
/******************************************************************************/

EqChain_p CopyEqChain(EqChain_p chain)
{
   EqChain_p chainhandle1,chainhandle2,handle;
   EqTerm_p  termhandle1,termhandle2;

   if(!chain)
   {
      fprintf(stderr,"Warning: NULL-Pointer given to CopyEqChain (pcl_eqchains.c)...\n");
      return NULL;
   }
   else
   {
      chainhandle1 = chain;
      handle = AllocEqChainCell();
      termhandle1 = chainhandle1->right;
      termhandle2 = AllocEqTermCell();
      handle->right = termhandle2;
      termhandle2->left = handle;
      termhandle2->term = CopyTerm(termhandle1->term);
  
      chainhandle1 = termhandle1->right;

      while(chainhandle1 != chain)
      {
         chainhandle2 = AllocEqChainCell();
         termhandle2->right = chainhandle2;
         chainhandle2->left = termhandle2;
         chainhandle2->tp = chainhandle1->tp;
         chainhandle2->place = CopyNumList(chainhandle1->place);
         chainhandle2->sigma = CopySubst(chainhandle1->sigma);
         chainhandle2->swapped = chainhandle1->swapped;

         termhandle1 = chainhandle1->right;

         termhandle2 = AllocEqTermCell();
         chainhandle2->right = termhandle2;
         termhandle2->left = chainhandle2;
         termhandle2->term = CopyTerm(termhandle1->term);

         chainhandle1 = termhandle1->right;
      }
      termhandle2->right = handle;
      handle->left = termhandle2;

      return handle;
   }
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : EqChain_p ConcatEqChains(EqChain_p chain1,              */
/*                                             EqChain_p chain2)              */
/*                    IN          EqChain_p chain1                            */
/*                    IN          EqChain_p chain2                            */
/*                                                                            */
/* Beschreibung     : chain2 wird destruktiv rechts an chain1 angehaengt.     */
/*                    Rueckgabewert ist chain1, an dem nun die neue Kette     */
/*                    haengt, chain2 wird voellig zerstoert.                  */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Zerstoerung von chain1, chain2                          */
/*                                                                            */
/* Aenderungen      : <1> 3.2.1992 neu                                        */
/*                                                                            */
/******************************************************************************/

EqChain_p ConcatEqChains(EqChain_p chain1,EqChain_p chain2)
{
   if(!TermEqual((chain1->left)->term,(chain2->right)->term))
   {
      Error("Chains not compatible in ConcatEqChains (pcl_eqchains.c)");
   }
   else if(chain2->right->right != chain2)
   {
      chain2->right->right->left = chain1->left;
      /* Der linke Nachtbar der ersten echten BEGRUENDUNGSZELLE in chain2
         ist der letzte TERM in chain1.                                     */ 

      chain1->left->right = chain2->right->right;
      /* Gegenpointer: Der rechte Nachtbar der LETZTEN Termzelle in chain1
         ist die erste echte BEGRUENDUNGSZELLE in chain2                    */

      chain2->left->right = chain1;
      chain1->left = chain2->left;
      /* Hinteres Ende verankern...  */
   }
      
   FreeTerm(chain2->right->term);
   FreeEqTermCell(chain2->right);
   FreeEqChainCell(chain2);
   
   return chain1;
}
   

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : EqChain_p SwapEqChain(EqChain_p chain)                  */
/*                    IN        EqChain_p chain                               */
/*                                                                            */
/* Beschreibung     : Dreht eine Gleichheitskette um.                         */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Kette wird veraendert                                   */
/*                                                                            */
/* Aenderungen      : <1> 3.2.1992 neu                                        */
/*                                                                            */
/******************************************************************************/

EqChain_p SwapEqChain(EqChain_p chain)
{
   EqChain_p chainhandle,chainhelp;
   EqTerm_p  termhandle,termhelp;

   chainhandle = chain;
   termhandle = chainhandle->right;
   do
   {
      chainhandle->swapped = !chainhandle->swapped;  /* swapped des Ankers wird auch */
                                                     /* veraendert, ist aber egal... */
      termhelp = chainhandle->left;
      chainhandle->left = chainhandle->right;
      chainhandle->right = termhelp;
      
      chainhelp = termhandle->left;
      termhandle->left = termhandle->right;
      termhandle->right = chainhelp;
   
      chainhandle = termhandle->left;  /* left, weil bereits umgedreht...  */
      termhandle = chainhandle->right; /* weil noch nicht umgedreht...     */
   }while(chainhandle != chain);
          
 

   return chain; 
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void CutEqChain(EqChain_p source, EqChain_p cut,        */
/*                                    EqChain_p *stt, EqChain_p *rst)         */
/*                    IN     EqChain_p source                                 */
/*                    IN     EqChain_p cut                                    */
/*                    OUT    EqChain_p *stt                                   */
/*                    OUT    EqChain_p *rst                                   */
/*                                                                            */
/* Beschreibung     : Zerlegt die Gleichheitskette, auf die source zeigt, in  */
/*                    die beiden Ketten, die entstehen, wenn man die Zelle,   */
/*                    auf die cut zeigt, entfernt.                            */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Die Kette wird destruktiv zerlegt, *stt und *rst werden */
/*                    veraendert.                                             */
/*                                                                            */
/* Aenderungen      : <1> 5.2.1992 neu                                        */
/*                                                                            */
/******************************************************************************/

void CutEqChain(EqChain_p source, EqChain_p cut,EqChain_p *stt, EqChain_p *rst)
{
   EqChain_p handle;
   EqTerm_p  termhandle; 

   if(!source)
   {
      fprintf(stderr,"Warning: NULL-Pointer given to CutEqChain (pcl_eqchains.c)...\n");
      *stt = NULL;
      *rst = NULL;
   }
   else
   {
      handle = source->right->right;
      while(handle!=source && handle!=cut)
      {
         handle = handle->right->right;
      }
      if(handle == source)
      {
         Error("Position to cut in CutEqChain is not in EqChain");
      }
      FreeNumList(cut->place);
      FreeSubst(cut->sigma);

      termhandle = source->left;
      source->left = cut->left;
      (cut->left)->right = source;

      cut->left = termhandle;
      termhandle->right = cut;

      *stt = source;
      *rst = cut;
   }
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : EqChain_p CleanEqChain(EqChain_p source)                */
/*                    IN    EqChain_p source                                  */
/*                                                                            */
/* Beschreibung     : Bereinigt destruktiv die Gleichheitskette, in dem Teile */
/*                    der Form t = s = ... =s' =t entfernt werden.            */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Speicheroperationen                                     */
/*                                                                            */
/* Aenderungen      : <1> 7.7.1992 neu                                        */
/*                                                                            */
/******************************************************************************/

EqChain_p CleanEqChain(EqChain_p source)
{
   EqTerm_p handle,
            found,
            help,
            junk;
   EqChain_p useless;

   for(handle = source->right;handle->right != source;handle = handle->right->right)
   {
      found = NULL;
      for(help = handle->right->right; help->right!=source;help = help->right->right)
      {
         if(TermEqual(handle->term,help->term))
         {
            found = help;
         }
      }
      if(found)
      {
         useless = handle->right;
         junk = useless->right;

         handle->right = found->right;
         (found->right)->left = handle;
         
         do
         {
            FreeSubst(useless->sigma);
            FreeNumList(useless->place);
            FreeEqChainCell(useless);
            useless = junk->right;
            FreeTerm(junk->term);
            FreeEqTermCell(junk);
            junk = useless->right;
         }
         while(junk != found->right->right);

      }
   }
   return source;
}

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : EqChain_p S(Term_p w, NumList_p q, Subst_p tau,         */
/*                                EqChain_p chain)                            */
/*                                                                            */
/* Beschreibung     : Schachtelt eine Begruendung an der Stelle q mit der     */
/*                    Substitution tau in den Term w.                         */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Aendert die Gleichheitskette.                           */
/*                                                                            */
/* Aenderungen      : <1> 5.2.1992 neu                                        */
/*                                                                            */
/******************************************************************************/

EqChain_p S(Term_p w, NumList_p q, Subst_p tau, EqChain_p chain)
{
   EqChain_p chainhandle;
   EqTerm_p  termhandle;

   if(!chain)
   {
      fprintf(stderr,"Warning: NULL-Pointer given to S (pcl_eqchains.c)...\n");
      return NULL;
   }
   else
   {
      termhandle = chain->right;
      termhandle->term = Replace(CopyTerm(w),
                                 q,
                                 ApplySubst(termhandle->term,tau));
      chainhandle = termhandle->right;
      while(chainhandle!=chain)
      {
         chainhandle->sigma = ApplySubstSubst(tau,chainhandle->sigma);
            /* Dies ist nur korrekt, wenn chainhandle->sigma alle in der */
            /* Gleichung vorkommenden Variablen substituiert, denn dann  */
            /* gilt ApplySubstSubst(tau,chainhandle->sigma)              */
            /* ConcatSubst(tau,chainhandle->sigma)|Var(tp) (hoffe ich    */
         chainhandle->place = ConcatNumList(CopyNumList(q),chainhandle->place);
         termhandle = chainhandle->right;
         termhandle->term = Replace(CopyTerm(w),
                                    q,
                                    ApplySubst(termhandle->term,tau));
         chainhandle = termhandle->right;
      }
   } 
   return chain;
}



/******************************************************************************/
/*                                                                            */
/* FUNCTION         : Subst_p EqChainVars(Subst_p yet,EqChain_p chain)        */
/*                                                                            */
/* Beschreibung     : Liefert eine Liste mit den in den TERMEN verwendeten    */
/*                    Variablen.                                              */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : yet wird veraendert                                     */
/*                                                                            */
/* Aenderungen      : <1> 13.2.1992 neu                                       */
/*                                                                            */
/******************************************************************************/

Subst_p EqChainVars(Subst_p yet,EqChain_p chain)
{
   EqChain_p chainhandle;

   chainhandle = chain->right->right;

   yet = VarList(yet,chain->right->term);

   while(chainhandle != chain)
   {
      yet = VarList(yet,chainhandle->right->term);
      chainhandle = chainhandle->right->right;
   }

   return yet;
}


/*----------------------------------------------------------------------------*/
/*                         Ende des Files                                     */
/*----------------------------------------------------------------------------*/





