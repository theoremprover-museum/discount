/*-------------------------------------------------------------------------

File        : learn_lpairs.c

Autor       : Stephan Schulz

Inhalt      : The data structure used to represent all the knowledge
              learned about one single lterm-pair, and functions
	      dealing with most of it's implementation. These are
	      analogous to the NormEqns from lrn_eqn.h.

Aenderungen : <1> 13.12.1994 neu
              <2> 8.3.1998   Beispielnamen in Lpair aufgenommen
              <3> 21.4.1998  TermpairToNormLpair

-------------------------------------------------------------------------*/


#include "learn_lpairs.h"


bool PosAndNeg = false;


/*-------------------------------------------------------------------------
//
// Function: parse_annotation(Lapir_p handle, example_p exa_handle)
//
//   Parses a single annotation and updates the data in the provided
//   Lpair accordingly.
//
// Global Variables:
//
// Side Effect     : As above
//
//-----------------------------------------------------------------------*/

void parse_annotation(Lpair_p handle, example_p exa_handle)
{
   bool posp = true;

   if(TestToken(identifier) || TestToken(string))
   {      
      exa_handle->name = SizeMalloc(strlen(AktLiteral)+1); 
      strcpy(exa_handle->name, AktLiteral);
      NextRealToken();
   }
   else
   {
      ScannerError("Identifier or \"\"-delimited string expected");
   }
   AcceptToken(openbracket);
   if(PosAndNeg)
   {
      posp = ParseBool();
      AcceptToken(comma);
      handle->cp_cost += AktNum;
      AcceptToken(number);
      AcceptToken(comma);
   }
   if(posp)
   {
      handle->proofs ++;
      handle->tot_ref += AktNum;
      AcceptToken(positive);
      AcceptToken(comma);
      handle->goal_dist += AktNum;
      AcceptToken(positive);
   }
   else
   {  /* Skip the useless data */
      handle->negative ++;
      AcceptToken(positive);
      AcceptToken(comma);
      AcceptToken(positive);
   }      
   AcceptToken(closebracket);
}


/*-----------------------------------------------------------------------*/
/*                  Exportierte Funktionen                               */
/*-----------------------------------------------------------------------*/

/*-------------------------------------------------------------------------

FUNCTION         : Lpair_p ParseLpair()

Beschreibung     : Parses an Lpair, consisting of two Lterms and the
                   equation sign between them, but not more
		   Information (see ParseLpairLine() for this). 

Globale Variable : -

Seiteneffekte    : By calling the scanner, Memory allocation

Aenderungen      : <1> 13.12.1994 neu

-------------------------------------------------------------------------*/

Lpair_p ParseLpair()
{
   Lpair_p handle;

   handle = AllocLpairCell();
   
   handle->lside = ParseLterm();
   AcceptToken(equal_sign);
   handle->rside = ParseLterm();

   handle->left = handle->right = NULL;

   return handle;
}


/*-------------------------------------------------------------------------

FUNCTION         : Lpair_p ParseLpairLine()

Beschreibung     : Parses the complete entry pertaining to one
                   equation in the knowledge base: The equation and
		   the list of occurences with the accompanying data.

Globale Variable : -

Seiteneffekte    : Scanning, Memory operations

Aenderungen      : <1> 13.12.1994 neu
                   <2> 9.3.1998 Beispielnamen

-------------------------------------------------------------------------*/

Lpair_p ParseLpairLine()
{
   Lpair_p handle;
   example_p exa_handle;   

   handle = ParseLpair();
   
   handle->proofs = 0;
   handle->negative = 0;
   handle->cp_cost = 0;
   handle->tot_ref = 0;
   handle->goal_dist = 0;
   
   AcceptToken(colon);

   handle->example = AllocExampleCell();
   handle->example->next = NULL;
   exa_handle = handle->example;

   parse_annotation(handle, exa_handle);

   while(TestToken(comma))
   {
      AcceptToken(comma);
      exa_handle->next = AllocExampleCell();
      exa_handle = exa_handle->next;
      parse_annotation(handle, exa_handle);
   }
   
   exa_handle->next = NULL;

   if(handle->proofs)
   {
      handle->ave_ref = handle->tot_ref/handle->proofs;
      handle->goal_dist = handle->goal_dist/handle->proofs;
   }
   else
   {
      handle->ave_ref = 0;
      handle->goal_dist = 0;
   }
   
   handle->balance =0;
   
   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: ParseLpairTree()
//
//   Parse a list of Lpairs and enters them into the binary search
//   tree rooted in *root. Returns the number of pairs parsed, updates
//   the maximal encountered values for goal-distance, total and
//   average references and proofs in max_values. 
//
// Global Variables: -
//
// Side Effects    : Scanning, Memory operations. The tree is, of
//                   course, changed, as is the record of maximums.
//
/----------------------------------------------------------------------*/

long    ParseLpairTree(Lpair_p *root, LpairTreeParams_p max_values)
{
   Lpair_p handle;
   long    count = 0;

   while(TestToken(identifier) && !ColonFollows)
   {
      handle = ParseLpairLine();
      max_values->proofs    = max(max_values->proofs,    handle->proofs);
      max_values->proofs_accumulated+=handle->proofs;
      max_values->ave_ref   = max(max_values->ave_ref,   handle->ave_ref);
      max_values->tot_ref   = max(max_values->tot_ref,   handle->tot_ref);
      max_values->goal_dist = max(max_values->goal_dist, handle->goal_dist);
      max_values->cp_cost   = max(max_values->cp_cost,   handle->cp_cost);
      max_values->negative  = max(max_values->negative,  handle->negative);
      max_values->cp_cost_accumulated+=handle->cp_cost;
      LpairTreeInsert(root, handle);
      count++;
   }
   return count;
}


/*-------------------------------------------------------------------------

FUNCTION         : Lpair_p ParseSpecTree(Lpair_p *root)

Beschreibung     : Parse a list of Lpairs and enters them into the
                   binary search tree rooted in *root. Returns the
		   tree. It does _not_ expect LpairLines (that's the
		   difference to ParseLpairTree().

Globale Variable : -

Seiteneffekte    : Scanning, Memory operations. The tree is, of
                   course, changed.

Aenderungen      : <1> 13.12.1994 neu

-------------------------------------------------------------------------*/

Lpair_p ParseSpecTree(Lpair_p *root)
{
   Lpair_p handle;

   while(TestToken(identifier) && !ColonFollows)
   {
      handle = ParseLpair();
      LpairTreeInsert(root, handle);
   }
   return *root;
}


/*-------------------------------------------------------------------------

FUNCTION         : long ParseLpair1TET(TET_p *root)

Beschreibung     : Parse a list of Lpairs and enters them into the
                   TET at *root. Returns number of Lpairs parsed.

Globale Variable : -

Seiteneffekte    : Scanning, Memory operations. The tree is, of
                   course, changed.

Aenderungen      : <1> 7.2.1995 neu

-------------------------------------------------------------------------*/

long ParseLpair1TET(TET_p* root)
{
   Lpair_p handle;
   long    count =0;

   while(TestToken(identifier) && !ColonFollows)
   {
      handle = ParseLpairLine();
      count++;
      LtermTETInsert(root, handle->lside, handle->proofs, handle->tot_ref);
      LtermTETInsert(root, handle->rside, handle->proofs, handle->tot_ref);
      FreeLpair(handle);
   }
   return count*2;
}


/*-------------------------------------------------------------------------

FUNCTION         : long ParseLpair2TET(TET_p *rootl, TET_p *rootr)

Beschreibung     : Parse a list of Lpairs and enters them into the
                   TETs. Returns number of Lpairs parsed.

Globale Variable : -

Seiteneffekte    : Scanning, Memory operations. The tree is, of
                   course, changed.

Aenderungen      : <1> 7.2.1995 neu

-------------------------------------------------------------------------*/

long ParseLpair2TET(TET_p *rootl, TET_p *rootr)
{
   Lpair_p handle;
   long    count =0;

   *rootl = *rootr = NULL;

   while(TestToken(identifier) && !ColonFollows)
   {
      handle = ParseLpairLine();
      count++;
      LtermTETInsert(rootl, handle->lside, handle->proofs, handle->tot_ref);
      LtermTETInsert(rootr, handle->rside, handle->proofs, handle->tot_ref);
      FreeLpair(handle);
   }
   return count;
}



/*-------------------------------------------------------------------------

FUNCTION         : void PrintLpair(Lpair_p pair)

Beschreibung     : Prints the equation represented by an Lpair. 

Globale Variable : -

Seiteneffekte    : Output

Aenderungen      : <1> 13.12.1994 neu

-------------------------------------------------------------------------*/

void PrintLpair(Lpair_p pair)
{
   PrintLterm(pair->lside);
   printf(" = ");
   PrintLterm(pair->rside);
}
   

/*-------------------------------------------------------------------------

FUNCTION         : void PrintLpairLine(Lpair_p pair)

Beschreibung     : Prints an Lpair with all the information as
                   available to DISCOUNT. Probably just for testing
		   and debugging purposes..

Globale Variable : -

Seiteneffekte    : Output

Aenderungen      : <1> 13.12.1994 neu
                   <2> 9.3.1998 Beispielnamen

-------------------------------------------------------------------------*/

void PrintLpairLine(Lpair_p pair)
{
   example_p exa_handle;   

   PrintLpair(pair);
   printf(" Proofs: %ld  Av.Ref: %ld  Tot.Ref: %ld Av.GoalDist: %ld ",
	  pair->proofs, pair->ave_ref, pair->tot_ref,
	  pair->goal_dist);
   printf("Examples: ");

   exa_handle = pair->example;

   while(exa_handle)
   {
     printf("%s ",exa_handle->name);
     exa_handle = exa_handle->next;
   }
   printf("\n");
}

/*-------------------------------------------------------------------------

FUNCTION         : void PrintExampleList(example_p exa_handle)

Beschreibung     : Prints an example list. Probably just for testing
		   and debugging purposes..

Globale Variable : -

Seiteneffekte    : Output

Aenderungen      : <1> 13.3.1998 new

-------------------------------------------------------------------------*/

void PrintExampleList(example_p exa_handle)
{
   while(exa_handle)
   {
     printf("%s ",exa_handle->name);
     exa_handle = exa_handle->next;
   }
   printf("\n");
}

/*-------------------------------------------------------------------------

FUNCTION         : void FreeExampleList(example_p junk)

Beschreibung     : Returns the memory used by an Example List

Globale Variable : -

Seiteneffekte    : Memory operations

Aenderungen      : <1> 8.3.1998 neu

-------------------------------------------------------------------------*/

void FreeExampleList(example_p junk)
{
   example_p help;

   while(junk)
   {
      help = junk->next;
      free(junk->name);
      junk->name=NULL;
      FreeExampleCell(junk);
      junk = help;
   }
}
      


/*-------------------------------------------------------------------------

FUNCTION         : void FreeLpair(Lpair_p junk)

Beschreibung     : Returns the memory used by an Lpair. Frees the
                   terms, but not lside and rside!

Globale Variable : -

Seiteneffekte    : Memory operations

Aenderungen      : <1> 13.12.1994 neu

-------------------------------------------------------------------------*/

void FreeLpair(Lpair_p junk)
{
   if(junk)
   {
      FreeLterm(junk->lside);
      FreeLterm(junk->rside);
      FreeExampleList(junk->example);
      FreeLpairCell(junk);
   }
   else
   {
      printf("Warning: Tried to free NULL in FreeLpair...\n");
   }
}
      

/*-------------------------------------------------------------------------

FUNCTION         : void FreeLpairTree(Lpair_p junk)

Beschreibung     : Frees a whole tree of Lpairs.

Globale Variable : -

Seiteneffekte    : Memory operations

Aenderungen      : <1> 13.12.1994 neu

-------------------------------------------------------------------------*/
  
void FreeLpairTree(Lpair_p junk)
{
   if(junk)
   {
      FreeLpairTree(junk->left);
      FreeLpairTree(junk->right);
      FreeLpair(junk);
   }
}
      
      

/*-------------------------------------------------------------------------

FUNCTION         : long CmpLpairs(Lpair_p eqn1, Lpair_p eqn2)

Beschreibung     : Compares two LpairTerms by lexicographically
                   comparing the two terms using CmpLTerms().

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 13.12.1994 neu

-------------------------------------------------------------------------*/

long CmpLpairs(Lpair_p pair1, Lpair_p pair2)
{
   long res;
   
   if(!(res =  CmpLterms(pair1->lside, pair2->lside)))
   {
      res = CmpLterms(pair1->rside, pair2->rside);
   }

/*
   printf("Vergleich von: ");
   PrintLpair(pair1);
   printf("\n          und: ");
   PrintLpair(pair2);
   printf("\nergibt %ld\n",res);
*/

   return res;
}


/*-------------------------------------------------------------------------

FUNCTION         : void LpairTreeInsert(Lpair_p *root, Lpair_p pair)

Beschreibung     : Inserts a Lterm into the binary Lterm-tree anchored
                   in *root.

Globale Variable : -

Seiteneffekte    : Changes in the tree structure

Aenderungen      : <1> 13.12.1994 neu

-------------------------------------------------------------------------*/

void LpairTreeInsert(Lpair_p *root, Lpair_p pair)
{
   long cmp_res;

   if(!(*root))
   {
      *root = pair;
      pair->left = pair->right = NULL;
   }
   else
   {
      cmp_res = CmpLpairs(*root, pair);

      if(cmp_res > 0)
      {
	 LpairTreeInsert(&((*root)->left), pair);
      }
      else if(cmp_res < 0)
      {
	 LpairTreeInsert(&((*root)->right), pair);
      }
      else
      {
	 /* Merge the two entries. The new goal_dist is the weighted */
	 /* average of the two old goal_dists, everything else is */
	 /* straigtforward. Both tot_refs should be greater than 0, so */
	 /* a division by zero cannot occur! */
	 (*root)->goal_dist = ((*root)->tot_ref * (*root)->goal_dist +
			    pair->tot_ref * pair->goal_dist) /
			       ((*root)->tot_ref+pair->tot_ref);
	 (*root)->tot_ref += pair->tot_ref;
	 (*root)->proofs  += pair->proofs;
	 (*root)->ave_ref = (*root)->tot_ref/(*root)->proofs;
	 FreeLpair(pair);
	 /* printf("Warning: Tried to insert an already known equation in\
LpairTreeInsert()\n"); */
      }
   }
}

/*-------------------------------------------------------------------------

FUNCTION         : void PrintLpairTree(Lpair_p root)

Beschreibung     : Prints the Tree given as an argument - probably
                   only for debugging purposes.

Globale Variable : -

Seiteneffekte    : Output

Aenderungen      : <1> 13.12.1994 neu

-------------------------------------------------------------------------*/

void PrintLpairTree(Lpair_p root)
{
   if(root)
   {
      PrintLpairTree(root->left);
      PrintLpairLine(root);
      PrintLpairTree(root->right);
   }
}


/*-------------------------------------------------------------------------

FUNCTION         : LearnSig_p ParseLearnSig()

Beschreibung     : Parses a complete signature from the knowledge base
                   and returns it. The signature should contain only
		   identifiers from f1...fn, with n<MAXFUNCTION. This
		   is only partially checked...

Globale Variable : AktToken, AktLiteral, Aktnum from s_scanner.c

Seiteneffekte    : I/O, memory organization

Aenderungen      : <1> 10.1.1994 neu

-------------------------------------------------------------------------*/

LearnSig_p ParseLearnSig()
{
   LearnSig_p handle = AllocLearnSig();
   long       max_id = 0,
              akt_id,
              ids_read = 0;

   AcceptIdent(ident, "signature");
   AcceptToken(colon);
   
   while(AktToken == idnum)
   {
      ids_read++;
      if(AktNum >= MAXFUNCTION)
      {
	 ScannerError("Function symbol has a to big norm-ident!");
      }
      akt_id = AktNum;
      max_id = max(akt_id, max_id);
      handle->sig_data[AktNum].ident = SecureStrdup(AktLiteral);
      AcceptToken(idnum);
      AcceptToken(colon);
      if(AktNum >= MAXARITY)
      {
	 ScannerError("Function symbol has a to big arity!");
      } 
      handle->sig_data[akt_id].arity = AktNum;
      AcceptToken(number);
   }
   if(ids_read != max_id)
   {
      ScannerError("Signature does not match required norm conditions!");
   }
   handle->entrycount = ids_read;

   return handle;
}
              

/*-------------------------------------------------------------------------

FUNCTION         : void PrintLearnSig(LearnSig_p sig) 

Beschreibung     : Prints a learn-signature (for debugging purposes
                   only...) 

Globale Variable : -

Seiteneffekte    : Output 

Aenderungen      : <1> 10.1.1994 neu

-------------------------------------------------------------------------*/

void PrintLearnSig(LearnSig_p sig) 
{
   short f;
   
   printf("PrintLearnSig (Size is %d):\n", sig->entrycount);
   printf("================================\n");

   for(f=1; f<=sig->entrycount;f++)
   {
      printf("%s (f%d) : %d\n", sig->sig_data[f].ident,
	     f, sig->sig_data[f].arity);
   }
}



/*-------------------------------------------------------------------------

FUNCTION         : bool LearnSigCompare(LearnSig_p sig1, LearnSig_p sig2)

Beschreibung     : Tests 2 signatures for identity (but ignores the
                   external idents)

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 10.1.1994 neu

-------------------------------------------------------------------------*/

bool LearnSigCompare(LearnSig_p sig1, LearnSig_p sig2)
{
   short f;

   if(sig1->entrycount != sig2->entrycount)
   {
      return false;
   }
   for(f=1; f<=sig1->entrycount; f++)
   {
      if(sig1->sig_data[f].arity !=
	 sig2->sig_data[f].arity)
      {
	 return false;
      }
   }
   return true;
}

/*-------------------------------------------------------------------------

FUNCTION         : Lpair_p TermpairToNormLpair(termpair* t)

Beschreibung     : Norms a discount termpair (after sorting the terms
                   by term-weight) and returns it as an Lpair.

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 21.4.1998 neu

-------------------------------------------------------------------------*/

Lpair_p TermpairToNormLpair(termpair* tp)
{
   Lpair_p res;
   DNormSubst_p subst;
   res = AllocLpairCell();
   subst = AllocEmptyDNormSubst();

   res->left = res->right = NULL;
   res->example = NULL;

   if(tp->left->weight < tp->right->weight)
   {
      subst = DNormTerm(tp->left, subst);
      res->lside = (Lterm_p)NtermToLterm(tp->left, subst);
      subst = DNormTerm(tp->right, subst);
      res->rside = (Lterm_p)NtermToLterm(tp->right, subst);
   }
   else
   {
      subst = DNormTerm(tp->right, subst);
      res->lside = (Lterm_p)NtermToLterm(tp->right, subst);
      subst = DNormTerm(tp->left, subst);
      res->rside = (Lterm_p)NtermToLterm(tp->left, subst);
   }

   FreeDNormSubst(subst);

   return res;
}

/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/


