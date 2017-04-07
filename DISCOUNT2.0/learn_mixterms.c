/*-------------------------------------------------------------------------

File        : learn_mixterms.c

Autor       : Stephan Schulz

Inhalt      : Functions dealing with both lterms and nterms (that is:
              DISCOUNT terms, possibly extended by DNormSubsts, and
              the arbitrary arity terms inherited from the pcl
              programs and necessary for the knowledge base
              maintainance. 

Aenderungen : <1>

-------------------------------------------------------------------------*/


#include "learn_mixterms.h"


/*-----------------------------------------------------------------------*/
/*           Forward-Deklaration interner Funktionen                     */
/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/*                     Interne Funktionen                                */
/*-----------------------------------------------------------------------*/



/*-----------------------------------------------------------------------*/
/*                  Exportierte Funktionen                               */
/*-----------------------------------------------------------------------*/

/*-------------------------------------------------------------------------

FUNCTION         : long CmpLtermNterm(Lterm_p lterm, term* nterm,
                                      DNormSubst_p subst)  

Beschreibung     : Compares a DISCOUNT term normalized with a
                   Norm-subst to the (always normalized) terms used
                   for the knowledge representation.

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 16.12.1994 neu

-------------------------------------------------------------------------*/

long CmpLtermNterm(Lterm_p lterm, term* nterm, DNormSubst_p subst) 
{
   long res = 0;
   long i;
   Lterm_p handle;

/*   printf("CmpLtermNterm called with nterm: ");
   PrintDNormTerm(nterm, subst);
   printf("\n                      and lterm: ");
   PrintLterm(lterm);
   printf("\n");*/

   
   if(!(res = (lterm->term_weight - nterm->weight )))
   {   
      if(varp(nterm)) /* Then also lterm->isvar...both terms represent */
	 /* single variables...this is ensured by the */
	 /* equality of the weights! */ 
      {
	 res = lterm->norm_id - VTfind(subst->variables,
				       (nterm->fcode));	 
      }
      else if(!NormTermPreserveArity ||
	      !(res = (lterm->arity - nterm->arity)))
      {
	 if(!(res = lterm->norm_id - subst->norm_id[nterm->fcode]))
	 {
	    i = 0; 
	    handle = lterm->args;
	    
	    while((i< nterm->arity) && handle && !res)
	    {
	       res = CmpLtermNterm(handle, nterm->argument[i], subst);
	       i++;
	       handle=handle->chain;
	    }
	    
	    if(!res)
	    {
	       if(i< nterm->arity)
	       {
		  res = -1;
	       }
	       else if(handle) 
	       {
		  res = 1;
	       }
	    }
	 }
      }
   }
   return res;
}


/*-------------------------------------------------------------------------

FUNCTION         : Lpair_p FindTwoNtermsLtermTree(term* lside, term*
                                                  rside, DNormSubst_p 
                                                  subst, Lpair_p root);

Beschreibung     : The function will take two DISCOUNT terms, a
                   DNorm-Substituion and a pointer to the root of an
		   Lpair-tree. It assumes the two terms to form
		   the normed pair (lterm, rterm) and will try to find
		   it in the tree. Return value will be a pointer to
		   the tree entry or NULL if none is found.

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 17.12.1994 neu

-------------------------------------------------------------------------*/

Lpair_p FindTwoNtermsLtermTree(term* lside, term* rside, DNormSubst_p
                               subst, Lpair_p root)
{
   long    cmp_res;
   Lpair_p handle = NULL;

   if(root)
   {
      if(!(cmp_res = CmpLtermNterm(root->lside, lside, subst)))
      {
	 cmp_res = CmpLtermNterm(root->rside, rside, subst);
	    
      }    
      if(cmp_res > 0)
      {
	 handle = FindTwoNtermsLtermTree(lside, rside, subst,
					 root->left);  
      }
      else if(cmp_res < 0)
      {
	 handle = FindTwoNtermsLtermTree(lside, rside, subst,
					 root->right); 
      }
      else
      {
	 handle = root;
      }
   }
   return handle;
} 


/*-------------------------------------------------------------------------

FUNCTION         : term* LtermToDomTerm(Lterm_p term, LearnSig_p sig)

Beschreibung     : This function translates an lterm to a DISCOUNT
                   term as used by the domain manager (using functions
		   from parsedom.[ch]. All norm_id's in term have to
		   be mentioned in sig!

Globale Variable : -

Seiteneffekte    : Memory operations (by the memory manager in
                   parsedom.c!

Aenderungen      : <1> 11.1.1994 neu

-------------------------------------------------------------------------*/

term* LtermToDomTerm(Lterm_p t, LearnSig_p sig)
{
   term*   res_term;
   Lterm_p handle; 
   short   f;

   if(t->isvar)
   {
      res_term = newterm_dom(-t->norm_id, 0);
      res_term->weight = 1;
   }
   else
   {
      if((t->norm_id > sig->entrycount) || (t->norm_id <= 0))
      {
	 Error ( __FILE__ ": "  "LtermToDomTerm", 
		"Function symbol not descibed in signature!");
      }
      res_term = newterm_dom(t->norm_id ,
			     sig->sig_data[t->norm_id ].arity); 
      res_term->weight = t->term_weight;

      handle = t->args;

      for(f=0; f<res_term->arity; f++)
      {
	 if(!handle)
	 {
	    Error ( __FILE__ ": "  "LtermToDomTerm", 
		   "Function symbol arity descibed incorrectly!");
	 }
	 res_term->argument[f] = LtermToDomTerm(handle,sig);
	 handle = handle->chain;
      }
      if(handle)
      {
	 Error ( __FILE__ ": "  "LtermToDomTerm", 
		"Function symbol arity descibed incorrectly!");
      }
   }
   return res_term;
}
      
/*-------------------------------------------------------------------------

FUNCTION         : Lterm_p NtermToLterm(term* term, DNormSubst_p subst)

Beschreibung     : This function translates a DISCOUNT Nterm normed
                   by the Subsitution subst to an Lterm as used by
                   the learning algorithms.

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 16.4.1998 neu

-------------------------------------------------------------------------*/

Lterm_p NtermToLterm(term* t, DNormSubst_p subst)
{
   Lterm_p res_term, handle;
   short i;

   res_term = AllocLtermCell();

   if(t->fcode < 0) /* Variables */
   {
      res_term->norm_id = VTfind(subst->variables, t->fcode);
      res_term->arity = 0;
      res_term->isvar = true;
      res_term->term_weight = 1;
      res_term->args = NULL;
   }
   else /* Functions */
   {
      res_term->norm_id = subst->norm_id[t->fcode];
      res_term->arity = t->arity;
      res_term->isvar = false;
      res_term->term_weight = t->weight;
      res_term->args = NULL;
      handle = res_term;

      for(i = 0; i < t->arity; i++)
      {
         if (!i) 
         {
            handle->args = NtermToLterm(t->argument[0], subst);
            handle = res_term->args;
	 }
         else 
         {
            handle->chain = NtermToLterm(t->argument[i], subst);
            handle = handle->chain;
         }
      }
      handle->chain = NULL;
   }
   return res_term;
}
      
/*-------------------------------------------------------------------------

FUNCTION         : termpair* LpairToDomPair(Lpair_p pair, LearnSig_p sig)

Beschreibung     : Creates a DISCOUNT termpair from the information
                   supplied in the lpair and the sig.

Globale Variable : -

Seiteneffekte    : Memory Operations

Aenderungen      : <1> 11.1.1994 neu

-------------------------------------------------------------------------*/

termpair* LpairToDomPair(Lpair_p pair, LearnSig_p sig)
{
   termpair* res_pair;

   res_pair = newpair(LtermToDomTerm(pair->lside ,sig),
		      LtermToDomTerm(pair->rside ,sig), NULL, NULL);
   return res_pair;
}


/*-------------------------------------------------------------------------

FUNCTION         : void LpairTreeAdd(Lpair_p tree, LearnSig_p sig,
                                     pairset *set) 

Beschreibung     : Translates all lpairs in the tree into DISCOUNT
                   termpairs (with dom-terms...) and inserts them into
		   the pairset.

Globale Variable : -

Seiteneffekte    : Chages in the pairset, Memory Operations

Aenderungen      : <1> 11.1.1994 neu

-------------------------------------------------------------------------*/

void LpairTreeAdd(Lpair_p tree, LearnSig_p sig, pairset *set)
{
   if(tree)
   {
      LpairTreeAdd(tree->left, sig, set);
      Add(set, LpairToDomPair(tree, sig));
      LpairTreeAdd(tree->right, sig, set);
   }
}


/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/


