/*************************************************************************/
/*                                                                       */
/*   File:        pcl_fproof.c                                           */
/*                                                                       */
/*   Autor:       Stephan Schulz                                         */
/*                                                                       */
/*   Inhalt:      Funktionen, die konstruktive PCL_Beweise in            */
/*                Gleichheitsketten umwandeln.                           */
/*                                                                       */
/*   Aenderungen: <1> 7.2.1992 neu                                       */
/*                                                                       */
/*************************************************************************/

#include "pcl_fproof.h"


/*----------------------------------------------------------------------------*/
/*              Forward-Deklaration interner Funktionen                       */
/*----------------------------------------------------------------------------*/

EqChain_p proof_self(Step_p step);

EqChain_p proof_tes_red(Just_p just);

EqChain_p proof_cp(Just_p just);

EqChain_p proof_orient(Just_p just);

EqChain_p proof_instance(Just_p just);




/*----------------------------------------------------------------------------*/
/*                      Globale Variable                                      */
/*----------------------------------------------------------------------------*/



long lemmacount = 0;
long axiomcount = 0;
long theoremcount =0;


/*----------------------------------------------------------------------------*/
/*                       Interne Funktionen                                   */
/*----------------------------------------------------------------------------*/


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : EqChain_p proof_self(Step_p step)                       */
/*                    IN     Step_p step                                      */
/*                                                                            */
/* Beschreibung     : Liefert die triviale Begruendung einer Gleichung        */
/*                    durch sich selbst.                                      */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Speicheroperationen                                     */
/*                                                                            */
/* Aenderungen      : <1> 7.2.1992 neu                                        */
/*                                                                            */
/******************************************************************************/

EqChain_p proof_self(Step_p step)
{
   EqChain_p handle,
             chainhandle;
   EqTerm_p  termhandle;

   handle = AllocEqChainCell();

   termhandle = AllocEqTermCell();
   handle->right = termhandle;
   termhandle->left = handle;
   if(!step->eqproof)
   {
      termhandle->term = CopyTerm(step->pair->lside);
   }
   else
   {
      termhandle->term = CopyTerm(step->eqproof->right->term);
   }
   chainhandle = AllocEqChainCell();
   chainhandle->sigma = VarList(NULL,termhandle->term);

   termhandle->right = chainhandle;
   chainhandle->left = termhandle;
   chainhandle->swapped = 0;
   chainhandle->place = NULL;
   chainhandle->tp = step;

   termhandle = AllocEqTermCell();
   chainhandle->right = termhandle;
   termhandle->left = chainhandle;
   if(!step->eqproof)
   {
      termhandle->term = CopyTerm(step->pair->rside);
   }
   else
   {
      termhandle->term = CopyTerm(step->eqproof->left->term);
   }
   chainhandle->sigma = VarList(chainhandle->sigma,termhandle->term);

   termhandle->right = handle;
   handle->left = termhandle;
 
   return handle;
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : EqChain_p proof_tes_red(Just_p just)                    */
/*                    IN    Just_p just                                       */
/*                                                                            */
/* Beschreibung     : Transformiert einen Reduktionsschritt in eine Gleich-   */
/*                    heitskette.                                             */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Berechnet eventuell die Gleichheitsketten fuer die      */
/*                    Vorfahren des Schrittes.                                */
/*                                                                            */
/* Aenderungen      : <1> 7.2.1992 neu                                        */
/*                                                                            */
/******************************************************************************/

EqChain_p proof_tes_red(Just_p just)
{
   EqChain_p parent1,
             parent2,
             handle;
   Term_p    l,
             t;
   Subst_p   match;

   parent1 = proof_f_step(just->arg1.rarg);
   parent2= proof_f_step(just->arg2.rarg);

   if(just->place1->side == 'L')
   {
      t = CopyTerm(parent1->right->term);  /* LINKER term der Kette ! */
   }
   else
   {
      t = CopyTerm(parent1->left->term);
   }
   if(just->place2->side == 'L')
   {
      l = CopyTerm(parent2->right->term);  /* LINKER term der Kette ! */
   }
   else
   {
      l = CopyTerm(parent2->left->term);
   }

   if(!Match(l,CopyTerm(Subterm(t,just->place1->rest)),&match)) /* This frees l */
   {
      Error("tes-red-expression does not describe reduction in proof_tes_red (pcl_fproof.c)");
   }
   
   if(just->place1->side == 'L')
   {
      if(just->place2->side == 'L')
      {
         handle = ConcatEqChains(S(t,just->place1->rest,match,SwapEqChain(parent2)),
                                 parent1);
      }
      else
      {
         handle = ConcatEqChains(S(t,just->place1->rest,match,parent2),
                                 parent1);
      }
   }
   else
   {
      if(just->place2->side == 'L')
      {
         handle = ConcatEqChains(parent1,
                                 S(t,just->place1->rest,match,parent2));
      }
      else
      {
         handle = ConcatEqChains(parent1,
                                 S(t,just->place1->rest,match,SwapEqChain(parent2)));
      }
   }
   FreeSubst(match);
   FreeTerm(t);
   
   return handle;   
}

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : EqChain_p proof_cp(Just_p just)                         */
/*                    IN    Just_p just                                       */
/*                                                                            */
/* Beschreibung     : Transformiert eine Kritische-Paar-Bildung in eine       */
/*                    Gleichheitskette.                                       */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Berechnet eventuell die Gleichheitsketten fuer die      */
/*                    Vorfahren des Schrittes.                                */
/*                                                                            */
/* Aenderungen      : <1> 7.2.1992 neu                                        */
/*                                                                            */
/******************************************************************************/

EqChain_p proof_cp(Just_p just)
{
   EqChain_p parent1,
             parent2,
             handle;
   Term_p    l1,
             l2,
             r2,
             overlap;
   Subst_p   unif,
             pretty,
             used,
             usedim,
             used_r2,
             match1,
             match2;

   parent1 = proof_f_step(just->arg1.rarg);
   parent2= proof_f_step(just->arg2.rarg);

   if(just->place1->side == 'L')
   {
      l1 = CopyTerm(parent1->right->term);  /* LINKER term der Kette ! */
   }
   else
   {
      l1 = CopyTerm(parent1->left->term);  
   }
   if(just->place2->side == 'L')  
   {
      l2 = CopyTerm(parent2->right->term);  /* LINKER term der Kette ! */
      r2 = CopyTerm(parent2->left->term);
   } 
   else
   {
      l2 = CopyTerm(parent2->left->term);  
      r2 = CopyTerm(parent2->right->term);
   }

   if(!Mgu(CopyTerm(Subterm(l1,just->place1->rest)),MakeUniq(CopyTerm(l2)),&unif))
   {
      Error("cp-expression does not describe critical pair in proof_cp (pcl_fproof.c)");
   }

   used = EqChainVars(NULL,parent1);/* Variablen des "linken" Elters  aufsammeln */
   usedim = UniqSubst(NULL,r2);     /* rechte Seite braucht neue Variablen...    */
   r2 = ApplySubst(r2,usedim);      /* ... diese Einsetzen                       */
   used_r2 = EqChainVars(CopySubst(used),parent2); /* Variablen des "rechten" Elters finden */
   pretty = PrettySubst(CopySubst(used_r2),r2);    /* "Schoene" neue Variablen fuer r2      */
   ApplySubstSubst(pretty,usedim);  /* Jetzt Variablen-Permutation: "alte->haesslich->neu"  */
   parent2 = S(r2,NULL,usedim,parent2); /* Der ganze Elter soll es wissen...                */
   ApplySubst(l2,usedim);               /* sogar die billige Kopie der Ueberlappung         */
   FreeSubst(pretty);                   /* Muellabfuhr                           */
   FreeTerm(r2);
   FreeSubst(used_r2);
   FreeSubst(usedim);
   used = EqChainVars(used,parent2);    /* used enthaelt jetzt alle Variablen aus beiden    */
                                        /* Eltern - dabei die NEUEN von parent2             */
   
   overlap = ApplySubst(CopyTerm(l1),unif);
   pretty = PrettySubst(CopySubst(used),overlap);
   overlap = ApplySubst(overlap,pretty);
   FreeSubst(used);
   FreeSubst(pretty); 

   if(!Match(l1,CopyTerm(overlap),&match1))  /* This frees l1 !  */
   { 
      Error("Something funny (foo) happend in proof_cp (pcl_fproof.c)");
   }
   if(!Match(l2,CopyTerm(Subterm(overlap,just->place1->rest)),&match2))  /* This frees l2 ! */
   { 
      Error("Something funny (bar) happend in proof_cp (pcl_fproof.c)");
   }
   if(just->place1->side == 'L')  
   {
      if(just->place2->side == 'L')
      {
         handle = ConcatEqChains(S(overlap,NULL,match1,SwapEqChain(parent1)),
                                 S(overlap,just->place1->rest,match2,parent2));
      }
      else
      {
         handle = ConcatEqChains(S(overlap,NULL,match1,SwapEqChain(parent1)),
                                 S(overlap,just->place1->rest,match2,SwapEqChain(parent2)));
      }
   }
   else
   {
      if(just->place2->side == 'L')
      {
         handle = ConcatEqChains(S(overlap,NULL,match1,parent1),
                                 S(overlap,just->place1->rest,match2,parent2));
      }
      else
      {
         handle = ConcatEqChains(S(overlap,NULL,match1,parent1),
                                 S(overlap,just->place1->rest,match2,SwapEqChain(parent2)));
      }
   }
   
   FreeTerm(overlap);
   FreeSubst(unif);
   FreeSubst(match1);
   FreeSubst(match2);

   return handle; 
}

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : EqChain_p proof_orient(Just_p just)                     */
/*                    IN    Just_p just                                       */
/*                                                                            */
/* Beschreibung     : Transformiert eine Orientierung in eine                 */
/*                    Gleichheitskette.                                       */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Berechnet eventuell die Gleichheitsketten fuer die      */
/*                    Vorfahren des Schrittes.                                */
/*                                                                            */
/* Aenderungen      : <1> 7.2.1992 neu                                        */
/*                                                                            */
/******************************************************************************/

EqChain_p proof_orient(Just_p just)
{
   EqChain_p parent;

   parent = proof_f_step(just->arg1.rarg);

   if(just->operation == orientx)
   {
      return SwapEqChain(parent);
   }
   else
   {
      return parent;
   }
}

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : EqChain_p proof_instance(Just_p just)                   */
/*                    IN    Just_p just                                       */
/*                                                                            */
/* Beschreibung     : Transformiert eine Instanziierung in eine               */
/*                    Gleichheitskette.                                       */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Berechnet eventuell die Gleichheitsketten fuer die      */
/*                    Vorfahren des Schrittes.                                */
/*                                                                            */
/* Aenderungen      : <1> 19.2.1992 neu                                       */
/*                                                                            */
/******************************************************************************/

EqChain_p proof_instance(Just_p just)
{
   EqChain_p parent1,
             parent2,
             fst,
             rst,
             handle;
   Term_p    instance,
             instancing,
             dummy;
   Subst_p match;

   parent1 = proof_f_step(just->arg1.rarg); /* instanziierte Gleichung   */
   parent2 = proof_f_step(just->arg2.rarg); /* instanziierende Gleichung */

   instance = CopyTerm(parent1->right->term);
   dummy = CopyTerm(instance);
   instance->chain = CopyTerm(parent1->left->term);
   instancing = CopyTerm(parent2->right->term);
   instancing->chain = CopyTerm(parent2->left->term);

   if(!Match(instancing,instance,&match))
   {
      Error("Instance-expression does not describe instanciation in proof_instance (pcl_fproof.c)");
   }

   handle = parent1->right->right;

   while(handle->tp->just->operation != hypothesis)
   {
      handle = handle->right->right;
      if(handle==parent1)
      {
         Error("Instanciated eqution does not describe a goal (?!?) in proof_instance (pcl_fproof.c)");
      }
   }
   CutEqChain(parent1,handle,&fst,&rst);
   parent2 = S(dummy,NULL,match,parent2);
   handle = ConcatEqChains(SwapEqChain(fst),parent2);
   handle = ConcatEqChains(handle,SwapEqChain(rst));

   FreeSubst(match);
   FreeTerm(dummy);

   return handle;
}



/******************************************************************************/
/*                                                                            */
/* FUNCTION         : EqChain_p proof_f_step(Just_p just)                     */
/*                    IN    Just_p just                                       */
/*                                                                            */
/* Beschreibung     : Generiert zu einer konstruktiven Begruendung fuer       */
/*                    einen PLC_Schritt die Gleichheitskette.                 */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Berechnet eventuell die Gleichheitsketten fuer die      */
/*                    Vorfahren des Schrittes.                                */
/*                                                                            */
/* Aenderungen      : <1> 7.2.1992 neu                                        */
/*                                                                            */
/******************************************************************************/

EqChain_p proof_f_step(Just_p just)
{
   EqChain_p handle;

   switch(just->operation)
   {
      case quotation: handle = ProofFStep(just->arg1.Targ.parg);
                      break;
      case tes_red  : handle = proof_tes_red(just);
                      break;
      case cp       : handle = proof_cp(just);
                      break;
      case orientu: 
      case orientx:   handle = proof_orient(just);
                      break;
      case instance:  handle = proof_instance(just);
                      break;
      default:        handle = 0;
                      Error("PCL-Expression does not describe a constructive step in proof_f_step (pcl_fproof.c)");
                      break;
   }
   return handle;
}
       



/*----------------------------------------------------------------------------*/
/*                    Exportierte Funktionen                                  */
/*----------------------------------------------------------------------------*/


/*-------------------------------------------------------------------------

FUNCTION         : void AllStepsNewVars(Step_p anchor)

Beschreibung     : Geht das gesamte PCL-listing bei anchor durch und
                   gibt allen Termen neue Variablen (wichtig fuer die
		   Prolog-Ausgabe). Steht nur hier, weil mir kein
		   besserer Platz einfiel.

Globale Variable : -

Seiteneffekte    : Terme im Listing werden veraendert

Aenderungen      : <1> 22.4.1994 neu

-------------------------------------------------------------------------*/

void AllStepsNewVars(Step_p anchor)
{
   Step_p help;

   for(help = anchor->succ; help!=anchor; help = help->succ)
   {
      help->pair = MakePrettyPair(help->pair);
   }
}



/******************************************************************************/
/*                                                                            */
/* FUNCTION         : EqChain_p ProofFStep(Step_p step)                       */
/*                    IN    Step_p step                                       */
/*                                                                            */
/* Beschreibung     : Berechnet zu einem konstruktiven PCL-Schritt den        */
/*                    Gleichheitsbeweis                                       */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Berechnet eventuell die Gleichheitsketten fuer die      */
/*                    Vorfahren des Schrittes.                                */
/*                                                                            */
/* Aenderungen      : <1> 7.2.1992 neu                                        */
/*                                                                            */
/******************************************************************************/

EqChain_p ProofFStep(Step_p step)
{
   DEBUG(DPROOF,fprintf(out,"Prooving ");PrintNumList(step->id);fprintf(out,"...\n"););
   if(!(step->eqproof))
   {
      if((step->just->operation == initial) || (step->just->operation == hypothesis))
      {
         step->eqproof = proof_self(step);
      }
      else
      {
         step->eqproof = proof_f_step(step->just);
      }
   }
   DEBUG(DPROOF,fprintf(out,"...prooved ");PrintNumList(step->id);fprintf(out," :"););
   DEBUG(DPROOF,PrintTermPair(step->pair);fprintf(out," :\n"));
   DEBUG(DPROOF,PrintEqChain(step->eqproof,TRUE,TRUE,TRUE);fprintf(out,"\n"));

   if(step->ax_lem_no)
   {
      return proof_self(step);
   }
   else
   {
      return CopyEqChain(step->eqproof);
   }
}

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : long ProofForward(Step_p anchor)                        */
/*                    IN     Step_p anchor                                    */
/*                                                                            */
/* Beschreibung     : Berechnet zu jedem konstruktiven PCL-Schritt in der     */
/*                    bei anchor verankerten Liste den Gleichheitsbeweis.     */
/*                    Rueckgabewert ist Anzahl der zu beweisenden Gleichungen */
/*                    (Fuer Vervollstaendigungen also 0)                      */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Die Gleichheitsketten werden in das Listing eingefuegt. */
/*                                                                            */
/* Aenderungen      : <1> 7.2.1992 neu                                        */
/*                                                                            */
/******************************************************************************/

long ProofForward(Step_p anchor)
{
   Step_p handle;
   long   existsgoal = 0;

   handle = anchor->succ;
   
   while(handle != anchor)
   {
      if(handle->just->operation == initial)
      {
         handle->ax_lem_no = ++axiomcount;
      }
      else if(handle->type == tes_final)
      {
         handle->ax_lem_no = ++theoremcount;
      }
      else if(handle->type == tes_lemma)
      {
         handle->ax_lem_no = ++lemmacount;
      }
      
      switch(handle->type)
      {
         case tes_eqn:
         case tes_rule:
         case tes_lemma:
         case tes_final:   
         case tes_goal: 
	 case tes_intermed: 
         case tes_intermedgoal:  if(handle->just->operation == hypothesis)
	                         { 
				    existsgoal++;
				 }
	                         ProofFStep(handle);
	                         break;
	 case crit_goal:
	 case crit_intermedgoal: Error(
              "Support for critical goals not yet implemented in ProofForward (pcl_fproof.c)");
	                         break;
         default:                Error("Undefined steptype in ProofForward (pcl_fproof.c)");
	                         break;
      }
      handle = handle->succ;
   }
   return existsgoal;
} 




/*----------------------------------------------------------------------------*/
/*                         Ende des Files                                     */
/*----------------------------------------------------------------------------*/


