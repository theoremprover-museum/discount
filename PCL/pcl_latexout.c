


/*************************************************************************/
/*                                                                       */
/*   File:        pcl_latexout.c                                         */
/*                                                                       */
/*   Autor:       Stephan Schulz                                         */
/*                                                                       */
/*   Inhalt:      Funktionen zur Beweisausgabe in LaTeX                  */
/*                                                                       */
/*   Aenderungen: <1> neu                                                */
/*                                                                       */
/*************************************************************************/

#include "pcl_latexout.h"


/*----------------------------------------------------------------------------*/
/*                    Forward-Deklarationen interner Funktionen               */
/*----------------------------------------------------------------------------*/

void L_print_eq_just(EqChain_p prt,BOOL prsubst);


/*----------------------------------------------------------------------------*/
/*                       Interne Funktionen                                   */
/*----------------------------------------------------------------------------*/

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void L_print_eq_just(EqChain_p prt, BOOL prsubst)       */
/*                    IN     EqChain_p prt                                    */ 
/*                    IN     BOOL      prsubst                                */
/*                                                                            */
/* Beschreibung     : Gibt Begruendung fuer einen einzelnen Gleichheitschritt */ 
/*                    aus.                                                    */
/*                                                                            */
/* Globale Variable : out (aus pcl_printer)                                   */
/*                                                                            */
/* Seiteneffekte    : -                                                       */
/*                                                                            */
/* Aenderungen      : <1> 18.3.1992 neu                                       */
/*                                                                            */
/******************************************************************************/

void L_print_eq_just(EqChain_p prt, BOOL prsubst)
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
      Error("Non-terminal equation used in L_print_eq_just (pcl_latexout.c)");  
   }
   if(prt->swapped)
   {
      fprintf(out,"RL ");
   }
   else
   {
      fprintf(out,"LR ");
   }
   if(prsubst)
   {
      fprintf(out," with ");
      L_PrintSubst(prt->sigma);
   }

}




/*----------------------------------------------------------------------------*/
/*                    ExportierteFunktionen                                   */
/*----------------------------------------------------------------------------*/


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void L_PrintTerm(Term_p prt,BOOL markmatch,             */
/*                                     NumList_p match,BOOL markrep,          */
/*                                     NumList_p rep                          */
/*                    IN      Term_p    prt                                   */
/*                    IN      BOOL      markmatch                             */
/*                    IN      NumList_p match                                 */
/*                    IN      BOOL      markrep                               */
/*                    IN      Term_p    prt                                   */
/*                                                                            */
/* Beschreibung     : Gibt einen Term in Latex-Format aus, dabei werden,      */
/*                    falls gew"unscht, die durch match und rep beschriebenen */
/*                    Teilterme durch unterstreichen bzw. Fettdruck gekenn-   */
/*                    zeichnet.                                               */ 
/*                                                                            */
/* Globale Variable : NoBrackets (aus pcl_terms.c),out (aus pcl_doio.c)       */
/*                                                                            */
/* Seiteneffekte    : -                                                       */
/*                                                                            */
/* Aenderungen      : <1> 17.3.1992 neu                                       */
/*                                                                            */
/******************************************************************************/

void L_PrintTerm(Term_p prt,BOOL markmatch,NumList_p match,BOOL markrep,NumList_p rep)
{
   long   termcount = 1;
   Term_p handle;

   if(!prt)
   {
      return;
   }
   if(markmatch && !match)
   {
      fprintf(out,"\\underline{");
   }
   if(markrep && !rep)
   {
      fprintf(out,"{\\bf ");
   }
      
   fprintf(out,"%s",prt->id);
   
   if(!prt->args)
   {
      if(!prt->isvar && !NoBrackets)
      {
         fprintf(out,"()");
      }
   }
   else
   {
      fprintf(out,"(");
      handle = prt->args;

      while(handle)
      {
         if(match && (termcount == match->value))
         {
            if(rep && (termcount == rep->value))
            {
               L_PrintTerm(handle,markmatch,match->rest,markrep,rep->rest);
            }
            else
            {
               L_PrintTerm(handle,markmatch,match->rest,FALSE,NULL);
            }
         }
         else
         {
            if(rep && (termcount == rep->value))
            {
               L_PrintTerm(handle,FALSE,NULL,markrep,rep->rest);
            }
            else
            {
               L_PrintTerm(handle,FALSE,NULL,FALSE,NULL);
            }
         }  
         
         if(handle->chain)
         {
            fprintf(out,", ");
         }
         handle = handle->chain;
         termcount++;
      }
      
      fprintf(out,")");
   }
   if(markmatch && !match)
   {
      fprintf(out,"}");
   }  
   if(markrep && !rep)
   {
      fprintf(out,"}");
   }
}

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void L_PrintEqChain(EqChain_p prt, BOOL prsubst,        */
/*                                        BOOL prplace)                       */
/*                    IN     EqChain_p prt                                    */
/*                    IN     BOOL      prsubst                                */
/*                    IN     BOOL      prplace                                */
/*                                                                            */
/* Beschreibung     : Formatierte Ausgabe einer Gleichheitskette in LaTeX     */
/*                                                                            */
/* Globale Variable : out (aus pcl_printer)                                   */
/*                                                                            */
/* Seiteneffekte    : -                                                       */
/*                                                                            */
/* Aenderungen      : <1> 18.3.1992 neu                                       */
/*                                                                            */
/******************************************************************************/

void L_PrintEqChain(EqChain_p prt,BOOL prsubst,BOOL prplace)
{
   EqTerm_p handle;
   EqChain_p right,left;

   left = prt;
   handle = prt->right;
   right = handle->right;

   fprintf(out,"\\begin{tabular}{rcll}\n");

   if(right == prt)
   {
      Error("Isolated term in L_PrintEqChain (pcl_latexout.c)");
   }
   else
   {
      L_PrintTerm(handle->term,prplace,right->place,FALSE,NULL);
      fprintf(out," & = & ");

      handle = right->right;
      left = right;
      right = handle->right;
      if(right == prt)
      {
         L_PrintTerm(handle->term,FALSE,NULL,prplace,left->place);
      }
      else
      {
         L_PrintTerm(handle->term,prplace,right->place,prplace,left->place);
      } 
      fprintf(out," & ");
      L_print_eq_just(handle->left,prsubst);
      fprintf(out,"\\\\\n");
      while(right != prt)
      {
         fprintf(out," & = & ");
         handle = right->right;
         left = right;
         right = handle->right;
         if(right == prt)
         {
            L_PrintTerm(handle->term,FALSE,NULL,prplace,left->place);
         }
         else
         {
            L_PrintTerm(handle->term,prplace,right->place,prplace,left->place);
         }
         fprintf(out," & ");
         L_print_eq_just(handle->left,prsubst);
         fprintf(out,"\\\\\n");
      }
   }
   fprintf(out,"\\end{tabular}\n");
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void    L_PrintSubst(Subst_p prt)                       */
/*                    IN      Subst_p prt                                     */
/*                                                                            */
/* Beschreibung     : Gibt eine Substitution in LaTeX aus                     */
/*                                                                            */
/* Globale Variable : out  (aus pcl_printer.c !)                              */
/*                                                                            */
/* Seiteneffekte    : -                                                       */
/*                                                                            */
/* Aenderungen      : <1> 18.3.1992 neu                                       */
/*                                                                            */
/******************************************************************************/

void L_PrintSubst(Subst_p prt)
{
   fprintf(out,"\\{");
   while(prt)
   {
      fprintf(out,"%s$\\leftarrow$",prt->varid);
      PrintTerm(prt->sterm);
      prt = prt->chain;
      if(prt)
      {
         fprintf(out, ", ");
      }
   }
   fprintf(out, "\\}");
}

/*----------------------------------------------------------------------------*/
/*                         Ende des Files                                     */
/*----------------------------------------------------------------------------*/


