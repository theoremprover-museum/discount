/*-------------------------------------------------------------------------

File        : learn_lterms.c

Autor       : Stephan Schulz

Inhalt      : This module implements norm-terms for the representation
              of the learned knowledge. These terms differ from the
	      DISCOUNT terms - each function symbol is represented by
	      a long integer, and may have arities that change between
	      terms (theoretically even within terms...). They are
	      derived from the pcl-terms and do not require any
	      signature information to decode (however, (partial)
	      signature may be useful for the later case-matching
	      algorithms). 

Aenderungen : <1> 8.12.1994 neu
              <2> 16.4.1998 PrintLterm modifiziert

-------------------------------------------------------------------------*/

#include "learn_lterms.h"


/*-----------------------------------------------------------------------*/
/*           Forward-Deklaration interner Funktionen                     */
/*-----------------------------------------------------------------------*/

Lterm_p   parse_lterm_args();
long      cmp_term_cell(Lterm_p tc1, Lterm_p tc2);


/*-----------------------------------------------------------------------*/
/*                     Interne Funktionen                                */
/*-----------------------------------------------------------------------*/

/*-------------------------------------------------------------------------

FUNCTION         : Lterm_p parse_lterm_args()

Beschreibung     : Parses an argument list "(t1,....,tn)" and returns
                   a pointer to it.

Globale Variable : -

Seiteneffekte    : By calling the scanner

Aenderungen      : <1> 8.12.1994 neu

-------------------------------------------------------------------------*/

Lterm_p parse_lterm_args()
{
   Lterm_p handle = NULL,
           *help;

   AcceptToken(openbracket);

   if(!TestToken(closebracket))
   {
      help = &handle; 
      *help = ParseLterm();
      help = &((*help)->chain);
      
      while(TestToken(comma))
      {
	 AcceptToken(comma);
	 *help = ParseLterm();
	 help = &((*help)->chain);
      }

      *help = NULL;
   }
   AcceptToken(closebracket);
   
   return handle;
}


/*-------------------------------------------------------------------------

FUNCTION         : long cmp_term_cell(Lterm_p tc1, Lterm_p tc2)

Beschreibung     : Compares to Lterm-Cells, using first the existance
                   of the pointer (NULL is smaller than any other
		   one), then the term_weight (making Variables small)
		   and finally the norm_ids.

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 8.12.1994 neu

-------------------------------------------------------------------------*/

long cmp_term_cell(Lterm_p tc1, Lterm_p tc2)
{
   long res;
   
   if(tc1 && !tc2)
   {
      res = 1;
   }
   else if (!tc1 && tc2)
   {
      res = -1;
   }
   else if(!(res = (tc1->term_weight - tc2->term_weight)))
   {
      if(!NormTermPreserveArity || !(res = (tc1->arity - tc2->arity)))
      {
         res = (tc1->norm_id - tc2->norm_id);
      }
   }
   return res;
}




/*-----------------------------------------------------------------------*/
/*                  Exportierte Funktionen                               */
/*-----------------------------------------------------------------------*/

/* Cell allocation and deallocation provided by macros in */
/* learn_lterm.h ! */


/*-------------------------------------------------------------------------

FUNCTION         : void FreeLterm(Lterm_p junk)

Beschreibung     : Returns all the memory occupied by a term to the
                   memory managment.

Globale Variable : -

Seiteneffekte    : Memory operations

Aenderungen      : <1> 8.12.1994 neu

-------------------------------------------------------------------------*/

void FreeLterm(Lterm_p junk)
{
   Lterm_p point, help;
   
   point = junk->args; 

   while(point) 
   {
      help = point->chain;
      FreeLterm(point);
      point = help;
   }
   FreeLtermCell(junk);
}


/*-------------------------------------------------------------------------

FUNCTION         : Lterm_p ParseLterm()

Beschreibung     : Parses an Lterm and builds the internal structure
                   belonging to it

Globale Variable : -

Seiteneffekte    : By calling the scanner

Aenderungen      : <1> 8.12.1994 neu

-------------------------------------------------------------------------*/

Lterm_p ParseLterm()
{
   Lterm_p handle, point;

   handle = AllocLtermCell();

   handle->norm_id = AktNum;
   handle->arity = 0;
   handle->term_weight = 2; /* Assuming it's a functions...variables */
			    /* get an absolute weight below */
   AcceptToken(idnum);
   
   if(TestToken(openbracket))
   {
     handle->args = parse_lterm_args();
     handle->isvar = false;
     for(point = handle->args; point; point = point->chain)
     {
	handle->arity++;
	handle->term_weight += point->term_weight;
     }
   }
   else
   {
      handle->args = NULL;
      handle->isvar = true;
      handle->term_weight = 1;
   }
   return handle;
}



/*-------------------------------------------------------------------------

FUNCTION         : long CmpLterms(Lterm_p term1, Lterm_p term2)

Beschreibung     : Compares two Lterms, using the lexicographic
                   extension to the natural ordering  on the function
		   symbols.

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 27.6.1994 neu

-------------------------------------------------------------------------*/

long CmpLterms(Lterm_p term1, Lterm_p term2)
{
   long res = 0;
   Lterm_p handle1, handle2;

   res = cmp_term_cell(term1,term2);

   if(!res && term1 && term2)
   {
      handle1 = term1->args;
      handle2 = term2->args;

      while(!res &&(handle1 || handle2))
      {
         res = CmpLterms(handle1,handle2);
         
         if(!res && handle1 && handle2)
         {
            handle1 = handle1->chain;
            handle2 = handle2->chain;
         }
      }
   }      

   return res;
}

/*-------------------------------------------------------------------------

FUNCTION         : short LtermArity(Lterm_p term)

Beschreibung     : Returns the number of arguments of term.

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 7.2.1994 neu

-------------------------------------------------------------------------*/

short LtermArity(Lterm_p term)
{
   short res = 0;
   Lterm_p handle;

   if(!term)
   {
      printf("Warning: LtermArity(NULL) called...\n");
      return 0;
   }
   for(handle = term->args; handle; handle = handle->chain)
   {
      res++;
   }
   if(res>= MAXARITY)
   {
      printf("Warning: LtermArity() found to many arguments...\n");
      res = MAXARITY-1;
   }
   return res;
}


/*-------------------------------------------------------------------------

FUNCTION         : void PrintLterm(Lterm_p term)

Beschreibung     : Prints a Lterm onto stdout...

Globale Variable : -

Seiteneffekte    : Output

Aenderungen      : <1> 9.12.1994 neu
                   <2> 16.4.1998 NormTermPreserveArity beruecksichtigen

-------------------------------------------------------------------------*/

void PrintLterm(Lterm_p term)
{
   Lterm_p handle;

   if(term->isvar)
   {
      printf("x%ld",term->norm_id);
   }
   else
   {
      if(NormTermPreserveArity) printf("f%ld_%ld(", term->arity, term->norm_id);
      else printf("f%ld(",term->norm_id);
      handle = term->args;
      if(handle)
      {
	 PrintLterm(handle);
	 for(handle = handle->chain; handle; handle = handle->chain)
	 {
	    printf(",");
	    PrintLterm(handle);
	 }
      }
      printf(")");
   }
}
      

/*-----------------------------------------------------------------------*/
/*                     Funktionen zu Abstraction-Trees                   */
/*-----------------------------------------------------------------------*/



/*-------------------------------------------------------------------------

FUNCTION         : TET_p AllocEmptyTET()

Beschreibung     : Allocates an empty TET.

Globale Variable : -

Seiteneffekte    : Memory Operations

Aenderungen      : <1> 6.2.1994 neu

-------------------------------------------------------------------------*/

TET_p AllocEmptyTET()
{
   TET_p handle;

   handle = AllocTETCell();
   memset(handle->branches, 0, MAXARITY * sizeof(TAT_p));
/*   handle->variables = false;*/

   return handle;
}


/*-------------------------------------------------------------------------

FUNCTION         : TAT_p AllocEmptyTAT()

Beschreibung     : Allocates an empty TAT-List

Globale Variable : -

Seiteneffekte    : Memory Operations

Aenderungen      : <1> 6.2.1994 neu

-------------------------------------------------------------------------*/

TAT_p AllocEmptyTAT()
{
   TAT_p handle;

   handle = AllocTATCell();
   memset(handle->args, 0, MAXARITY * sizeof(TET_p));
   handle->proofs = handle->count = 0;

   return handle;
}



/*-------------------------------------------------------------------------

FUNCTION         : void FreeTET(TET_p junk)

Beschreibung     : Frees a full Atree(list) and all subtrees.

Globale Variable : -

Seiteneffekte    : Memory Operations

Aenderungen      : <1> 6.2.1994 neu

-------------------------------------------------------------------------*/

void FreeTET(TET_p junk)
{
   short i;

   if(junk)
   {
      for(i=0; i<MAXARITY; i++)
      {
	 FreeTAT(junk->branches[i]);
      }
      FreeTETCell(junk);
   }
}

/*-------------------------------------------------------------------------

FUNCTION         : void FreeTAT(TAT_p junk)

Beschreibung     : Frees a Operator-Atree (list).

Globale Variable : -

Seiteneffekte    : Memory Operations

Aenderungen      : <1> 6.2.1994 neu

-------------------------------------------------------------------------*/

void FreeTAT(TAT_p junk)
{
   short i;

   if(junk)
   {
      for(i=0; i<MAXARITY && junk->args[i]; i++)
      {
	 FreeTET(junk->args[i]);
      }
      FreeTATCell(junk);
   }
}


/*-------------------------------------------------------------------------

FUNCTION         : void PrintTET(TET_p tree)

Beschreibung     : Prints a Full Atree - just for debugging, I suppose.

Globale Variable : -

Seiteneffekte    : Output

Aenderungen      : <1> 6.2.1994 neu

-------------------------------------------------------------------------*/

void PrintTET(TET_p tree)
{
   long i;

   if(tree)
   {
      printf("TET %ld BEGIN\n", (unsigned long)tree);
/*      printf("Variables: %ld\n", tree->variables); */
      for(i=1; i<MAXARITY;i++)
      {
	 if(tree->branches[i])
	 {
	    printf("TET %ld (%ld)\n", (unsigned long)tree, i);
	    PrintTAT(tree->branches[i]);
	 }
      }
      printf("TET %ld END\n", (unsigned long)tree);
   }
   else
   {
      printf("NULL\n");
   }
}


/*-------------------------------------------------------------------------

FUNCTION         : void PrintTAT(TAT_p tree)

Beschreibung     : Prints an Operator Atree - just for debugging, I
                   suppose. 

Globale Variable : -

Seiteneffekte    : Output

Aenderungen      : <1> 6.2.1994 neu

-------------------------------------------------------------------------*/

void PrintTAT(TAT_p tree)
{
   long i;
   
   if(tree)
   {
      printf("TAT %ld BEGIN: proofs = %ld count = %ld\n",
	     (unsigned long)tree, tree->proofs, tree->count);
      for(i=0; i<MAXARITY && tree->args[i];i++)
      {
	 printf("TAT %ld (%ld)\n", (unsigned long)tree, i);
	 PrintTET(tree->args[i]);
      }
      printf("TAT %ld END\n", (unsigned long)tree);
   }
   
   else
   {
      printf("NULL\n");
   }
} 


/*-------------------------------------------------------------------------

FUNCTION         : void LtermTATInsert(TAT_p* tree, Lterm_p t, long
                                       proofs) 

Beschreibung     : Fuegt den Term in den Op-Atree ein.

Globale Variable : -

Seiteneffekte    : Veraenderung des Baumes.

Aenderungen      : <1> 6.2.1995 neu

-------------------------------------------------------------------------*/

void LtermTATInsert(TAT_p *tree, Lterm_p t, long proofs, long tot_ref)
{
   short i = 0;
   Lterm_p handle;

   if(!(*tree))
   {
      *tree = AllocEmptyTAT();
   }
   (*tree)->proofs += proofs;
   (*tree)->tot_ref += tot_ref;
   (*tree)->count ++;
   
   for(handle = t->args; handle; handle = handle->chain)
   {
      LtermTETInsert(&((*tree)->args[i++]), handle, proofs, tot_ref);
   }
}
      

/*-------------------------------------------------------------------------

FUNCTION         : void LtermTETInsert(TET_p* tree, Lterm_p t, long proofs) 

Beschreibung     : Inserts the term into the abstraction tree.

Globale Variable : -

Seiteneffekte    : Veraenderung des Baumes

Aenderungen      : <1> 6.2.1995 neu

-------------------------------------------------------------------------*/

void LtermTETInsert(TET_p* tree, Lterm_p t, long proofs, long tot_ref)
{
   if(!(*tree))
   {
      *tree = AllocEmptyTET();
   }
   LtermTATInsert(&((*tree)->branches[LtermArity(t)]), t, proofs, tot_ref);

   /*   if(t->isvar)
   {
      (*tree)->variables += proofs;
   }
   else
   {
      LtermTATInsert(&((*tree)->branches[LtermArity(t)]), t, proofs, tot_ref);
   }*/
}
   





/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/


