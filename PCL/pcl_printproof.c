/*-------------------------------------------------------------------------

File        : pcl_printproof.c

Autor       : Stephan Schulz

Inhalt      : Funktionen zur Beweisausgabe      

Aenderungen : <1> 19.2.1992 neu  
              <2> 20.4.1993 Prolog-Ausgabe

-------------------------------------------------------------------------*/



#include "pcl_printproof.h"





/*-----------------------------------------------------------------------*/
/*                  Globale Variable                                     */
/*-----------------------------------------------------------------------*/



long Goals,
     Lemmas,
     Axioms,
     Theorems;

/*-----------------------------------------------------------------------*/
/*            Forward-Deklaration interner Funktionen                    */
/*-----------------------------------------------------------------------*/

EqChain_p find_goal(EqChain_p proof);

void      p_ascii(Step_p anchor,BOOL prsubst, BOOL prplace);
void      p_latex(Step_p anchor,BOOL prsubst, BOOL prplace);
void      p_prolog(Step_p anchor,BOOL prsubst, BOOL prplace);


/*-----------------------------------------------------------------------*/
/*                       Interne Funktionen                              */
/*-----------------------------------------------------------------------*/


/*-------------------------------------------------------------------------

FUNCTION         : EqChain_p find_goal(EqChain_p proof)

Beschreibung     : Sucht in einer Gleichheitskette nach der Verwendung     
                   eines initialen Goals. Rueckgabewert ist Pointer auf
                   den Schritt, der das Goal verwendet.     

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 20.2.1992 neu
                   <2> 7.3.1994 Debugged: Jetzt sucht er NUR NOCH
                                          goals... 

-------------------------------------------------------------------------*/


EqChain_p find_goal(EqChain_p proof)
{
   EqChain_p handle;
   
   handle = proof->right->right;

   while((handle != proof) && 
	 !((handle->tp->just->operation == hypothesis) &&
	   (handle->tp->type == tes_goal)))
   {
      handle = handle->right->right;
   }
   if(handle == proof)
   {
      return NULL;
   }
   else
   {
      return handle;
   }
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void p_ascii(Step_p anchor,BOOL prsubst, BOOL prplace)  */
/*                    IN     Step_p anchor                                    */
/*                    IN     BOOL   prsubst                                   */
/*                    IN     BOOL   prplace                                   */
/*                                                                            */
/* Beschreibung     : Gibt den kompletten Beweis in ASCII-Text mit den        */ 
/*                    gew"unschten Informationen aus.                         */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : -                                                       */
/*                                                                            */
/* Aenderungen      : <1> 19.2.1992 neu                                       */
/*                                                                            */
/******************************************************************************/

void p_ascii(Step_p anchor,BOOL prsubst, BOOL prplace)
{
   Step_p handle;

   fprintf(out,"\nConsider the following set of axioms:\n\n");

   handle = anchor->succ;

   while(handle!=anchor)
   {
      if(handle->ax_lem_no && (handle->just->operation == initial))
      {
         fprintf(out,"  Axiom %ld: ",handle->ax_lem_no);
         PrintTerm(handle->pair->lside);
         fprintf(out," = ");
         PrintTerm(handle->pair->rside); 
         fprintf(out,"\n");
      }
      handle = handle->succ;
   }
   if(Theorems>1)
   {
      fprintf(out,"\nThese theorems hold true:\n\n");
   }
   else
   {
      fprintf(out,"\nThis theorem holds true:\n\n");
   }
   handle = anchor->succ;
   while(handle!=anchor)
   {
      if(handle->type == tes_final)
      {
         fprintf(out,"  Theorem %ld: ",handle->ax_lem_no);
         PrintTerm(handle->eqproof->right->term);
         fprintf(out," = ");
         PrintTerm(handle->eqproof->left->term);
         fprintf(out,"\n");
      }
      handle = handle->succ;
   }
   fprintf(out,"\nProof:\n\n");
   
   handle = anchor->succ;
   while(handle != anchor)
   {
      if(handle->type == tes_lemma)
      {
         fprintf(out,"  Lemma %ld: ",handle->ax_lem_no);
         PrintTerm(handle->eqproof->right->term);
         fprintf(out," = ");
         PrintTerm(handle->eqproof->left->term);
         fprintf(out,"\n\n");
         PrintEqChain(handle->eqproof,prsubst,prplace,FALSE);
         fprintf(out,"\n");
      }
      else if(handle->type == tes_final)
      {
         fprintf(out,"  Theorem %ld: ",handle->ax_lem_no);
         PrintTerm(handle->eqproof->right->term);
         fprintf(out," = ");
         PrintTerm(handle->eqproof->left->term);
         fprintf(out,"\n\n");
         PrintEqChain(handle->eqproof,prsubst,prplace,FALSE);
         fprintf(out,"\n");
      }
      handle = handle->succ;
   }
}
      

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void p_latex(Step_p anchor,BOOL prsubst, BOOL prplace)  */
/*                    IN     Step_p anchor                                    */
/*                    IN     BOOL   prsubst                                   */
/*                    IN     BOOL   prplace                                   */
/*                                                                            */
/* Beschreibung     : Gibt den kompletten Beweis in LaTeX mit den             */
/*                    gewuenschten Informationen aus.                         */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : -                                                       */
/*                                                                            */
/* Aenderungen      : <1> 19.2.1992 neu                                       */
/*                                                                            */
/******************************************************************************/

void p_latex(Step_p anchor,BOOL prsubst, BOOL prplace)
{
   Step_p handle;

   fprintf(out,"\n\\noindent\n{\\bf Consider the following set of axioms:\\\\}\n\\begin{quote}\n");

   handle = anchor->succ;

   while(handle!=anchor)
   {
      if(handle->ax_lem_no && (handle->just->operation == initial))
      {
         fprintf(out,"Axiom %ld: ",handle->ax_lem_no);
         L_PrintTerm(handle->pair->lside,FALSE,NULL,FALSE,NULL);
         fprintf(out," = ");
         L_PrintTerm(handle->pair->rside,FALSE,NULL,FALSE,NULL);
         fprintf(out,"\\\\\n");
      }
      handle = handle->succ;
   }
   fprintf(out,"\\end{quote}\n");
   if(Theorems>1)
   {
      fprintf(out,"\n\\noindent\n{\\bf These theorems hold true:}\\\\\n");
   }
   else
   {
      fprintf(out,"\n\\noindent\n{\\bf This theorem holds true:}\\\\\n");
   }
   handle = anchor->succ;

   fprintf(out,"\\begin{quote}\n");

   while(handle!=anchor)
   {
      if(handle->type == tes_final)
      {
         fprintf(out,"Theorem %ld: ",handle->ax_lem_no);
         L_PrintTerm(handle->eqproof->right->term,FALSE,NULL,FALSE,NULL);
         fprintf(out," = ");
         L_PrintTerm(handle->eqproof->left->term,FALSE,NULL,FALSE,NULL);
         fprintf(out,"\\\\\n");
      }
      handle = handle->succ;
   }
   fprintf(out,"\\end{quote}\n");

   fprintf(out,"{\\bf Proof:}\\\\\n");
  
   handle = anchor->succ;
   while(handle != anchor)
   {
      if(handle->type == tes_lemma)
      {
         fprintf(out,"\\paragraph{Lemma %ld:}",handle->ax_lem_no);
         L_PrintTerm(handle->eqproof->right->term,FALSE,NULL,FALSE,NULL);
         fprintf(out," = ");
         L_PrintTerm(handle->eqproof->left->term,FALSE,NULL,FALSE,NULL);
         fprintf(out,"\\\\\n\\medskip\n");
         L_PrintEqChain(handle->eqproof,prsubst,prplace);
         fprintf(out,"\n");
      }
      else if(handle->type == tes_final)
      {
         fprintf(out,"\\paragraph{Theorem %ld:}",handle->ax_lem_no);
         L_PrintTerm(handle->eqproof->right->term,FALSE,NULL,FALSE,NULL);
         fprintf(out," = ");
         L_PrintTerm(handle->eqproof->left->term,FALSE,NULL,FALSE,NULL);
         fprintf(out,"\\\\\n\\medskip\n");
         L_PrintEqChain(handle->eqproof,prsubst,prplace);
         fprintf(out,"\n");
      }
      handle = handle->succ;
   }
}

     
/*-------------------------------------------------------------------------

FUNCTION         : void p_prolog(Step_p anchor,BOOL prsubst, BOOL prplace)
                   IN     Step_p anchor
                   IN     BOOL   prsubst (dummy)
                   IN     BOOL   prplace (dummy)
Beschreibung     : Gibt den Beweis in Prolog-feundlicher Form aus.

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 20.4.1993

-------------------------------------------------------------------------*/

void p_prolog(Step_p anchor,BOOL prsubst, BOOL prplace)
{
   Step_p handle;

   fprintf(out,"\n/* AXIOMS */\n");

   handle = anchor->succ;

   while(handle!=anchor)
   {
      if(handle->ax_lem_no && (handle->just->operation == initial))
      {
         fprintf(out,"axiom(%ld,",handle->ax_lem_no);
         PrintTerm(handle->pair->lside);
         fprintf(out," = ");
         PrintTerm(handle->pair->rside); 
         fprintf(out,").\n");
      }
      handle = handle->succ;
   }

   fprintf(out,"\n/* THEOREM(S)/*\n");

   handle = anchor->succ;
   while(handle!=anchor)
   {
      if(handle->type == tes_final)
      {
         fprintf(out,"theorem(%ld,",handle->ax_lem_no);
         PrintTerm(handle->eqproof->right->term);
         fprintf(out," = ");
         PrintTerm(handle->eqproof->left->term);
         fprintf(out,").\n");
      }
      handle = handle->succ;
   }
   fprintf(out,"\n/* PROOF */\n");
   
   handle = anchor->succ;
   while(handle != anchor)
   {
      if(handle->type == tes_lemma)
      {
         fprintf(out,"lemma(%ld,",handle->ax_lem_no);
         PrintTerm(handle->eqproof->right->term);
         fprintf(out," = ");
         PrintTerm(handle->eqproof->left->term);
         fprintf(out,").\n");
         P_PrintEqChain(handle->eqproof,lemma,handle->ax_lem_no);
         fprintf(out,"\n");
      }
      else if(handle->type == tes_final)
      {
         fprintf(out,"theorem(%ld,",handle->ax_lem_no);
         PrintTerm(handle->eqproof->right->term);
         fprintf(out," = ");
         PrintTerm(handle->eqproof->left->term);
         fprintf(out,").\n");
         P_PrintEqChain(handle->eqproof,theorem,handle->ax_lem_no);
         fprintf(out,"\n");
      }
      handle = handle->succ;
   }
}
      

/*----------------------------------------------------------------------------*/
/*                    Exportierte Funktionen                                  */
/*----------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------

FUNCTION         : void PrintProof(Step_p anchor,OutputType output,
                                   BOOL prsubst, BOOL prplace)   
                   IN     Step_p     anchor
                   IN     OutputType output
                   IN     BOOL       prsubst
                   IN     BOOL       prplace  
		   
Beschreibung     : Gibt den kompletten Beweis aus...  

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 19.2.1992 neu
                   <2> 20.4.1994 Prolog-Teil

-------------------------------------------------------------------------*/

void PrintProof(Step_p anchor, OutputType output, BOOL prsubst,BOOL prplace)
{
   Step_p    handle;
   EqChain_p goal,
             fst,
             rst;

   Goals = 0;
   Lemmas = 0;
   Axioms = 0;
   Theorems = 0;

   handle = anchor->succ;
   while(handle != anchor)
   {
      if(handle->type == tes_lemma)
      {
         handle->ax_lem_no = ++Lemmas;
      }
      else if(((handle->type == tes_goal)||
               (handle->type == tes_intermedgoal)||
               (handle->type == crit_goal)||
               (handle->type == crit_intermedgoal)) 
               && (handle->just->operation == hypothesis)) 
      {
         Goals++;
      }
      else if(handle->type == tes_final)
      {
         handle->ax_lem_no = ++Theorems;
         goal = find_goal(handle->eqproof);
         if(goal)
         {
            CutEqChain(handle->eqproof,goal,&fst,&rst);
            handle->eqproof = ConcatEqChains(SwapEqChain(fst),SwapEqChain(rst));
         }
      }
      else if(handle->just->operation == initial)
      {
         handle->ax_lem_no = ++Axioms;
      }
      CleanEqChain(handle->eqproof); 
      
      handle = handle->succ;
   }

   if(output == ascii)
   {
      p_ascii(anchor,prsubst,prplace);
   }
   else if(output == latex)
   {
      p_latex(anchor,prsubst,prplace);
   }
   else
   {
      p_prolog(anchor,prsubst,prplace);
   }
}




/*----------------------------------------------------------------------------*/
/*                         Ende des Files                                     */
/*----------------------------------------------------------------------------*/


