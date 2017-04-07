/*-------------------------------------------------------------------------

File        : learn_tets.c

Autor       : Stephan Schulz

Inhalt      : Functions for the evaluation of critical pairs based on
              the concept of TETS and TATS ("learning by term space
	      mapping")  

Aenderungen : <1> 5.1.1996 new (extracted from learn_cpweight.c)

-------------------------------------------------------------------------*/

#include "learn_tets.h"



/*-----------------------------------------------------------------------*/
/*                      Globale Variable                                 */
/*-----------------------------------------------------------------------*/

double CountLimit = 0.2;

/*-----------------------------------------------------------------------*/
/*           Forward-Deklaration interner Funktionen                     */
/*-----------------------------------------------------------------------*/


long read_global_tet(TET_p *tet, bool complete);
long read_global_2tet(TET_p *tetl, TET_p *tetr, 
				 bool complete);
long sink_args_TAT(long *found, long *ref_sum, term *t, 
			      TAT_p tat);
long sink_args_TAT2(term *t, long norm, long unk_penalty, 
			       TAT_p tat);
void norm_TAT_anno(TAT_p tat, short arity, long proofs, long count,
		   long tot_ref); 

/*-----------------------------------------------------------------------*/
/*                     Interne Funktionen                                */
/*-----------------------------------------------------------------------*/


/*-------------------------------------------------------------------------
//
// Function: read_global_tet()
//
//    Reads the file realpath(KnowledgeBase/Xdata) if KnowledgeBase !=
//    NULL, realpath("KNOWLEDGE/Xdata") otherwise. X is either c or p,
//    depending on the value of complete (complet == true -> cdata
//    (complete data) is read). The fuction will skip the syntactic
//    sugar and go directly for the facts. These facts will be
//    collected into a TET, return value is the number of equations in
//    the tet.  
//
// Global Variables: KnowledgeBase, TmpPathName
//
// Side Effect     : Memory, IO, building the TET
//
//-----------------------------------------------------------------------*/

long read_global_tet(TET_p *tet, bool complete)
{
   long kb_size = 0;

   if(KnowledgeBase[0])
   {
      strcpy(PathNameHelp, KnowledgeBase);
   }
   else
   {
      strcpy(PathNameHelp, "KNOWLEDGE");
   }
   strcat(PathNameHelp, complete ? "/cdata":"/pdata");
   
   if(!realpath(PathNameHelp, TmpPathName))
   {
      printf("Filename '%s' could not be resolved...\n", PathNameHelp);
      perror("ERROR: realpath");
      exit(-1);
   }
   InitScanner(TmpPathName);
   printf("Reading global knowledge from %s into TET.\n", TmpPathName);
   
   while(!(TestIdent(ident, "facts") && ColonFollows)) /* Skip junk */
   {
      NextRealToken();
      if(TestToken(NoToken))
      {
         ScannerError("Unexpected end of file");
      }
   }
   
   AcceptIdent(ident, "facts");
   AcceptToken(colon);

   kb_size = ParseLpair1TET(tet);

   EndScanner();
   printf("Knowledge read.\n");
   
   return kb_size;
} 

/*-------------------------------------------------------------------------

FUNCTION         : long read_global_2tet(TET_p *tetl, TET_p *tetr,
                                         bool complete) 

Beschreibung     : Reads the file realpath(KnowledgeBase/Xdata) if
                   KnowledgeBase != NULL, realpath("KNOWLEDGE/Xdata")
                   otherwise. X is either c or p, depending on the
                   value of complete (complet == true -> cdata
                   (complete data) is read). The fuction will skip the
                   syntactic sugar and go directly for the facts.
                   These facts will be collected into two TETs. Return
                   value is the numbers of equations read.

Globale Variable : KnowledgeBase, TmpPathName

Seiteneffekte    : Memory, IO, building the TET

Aenderungen      : <1> 7.2.1995 neu

-------------------------------------------------------------------------*/

long read_global_2tet(TET_p *tetl, TET_p *tetr, bool complete)
{
   long kb_size;
   
   if(KnowledgeBase[0])
   {
      strcpy(PathNameHelp, KnowledgeBase);
   }
   else
   {
      strcpy(PathNameHelp, "KNOWLEDGE");
   }
   strcat(PathNameHelp, complete ? "/cdata":"/pdata");
   
   if(!realpath(PathNameHelp, TmpPathName))
   {
      printf("Filename '%s' could not be resolved...\n", PathNameHelp);
      perror("ERROR: realpath");
      exit(-1);
   }
   InitScanner(TmpPathName);
   printf("Reading global knowledge from %s into TET.\n", TmpPathName);
   
   while(!(TestIdent(ident, "facts") && ColonFollows)) /* Skip junk */
   {
      NextRealToken();
      if(TestToken(NoToken))
      {
         ScannerError("Unexpected end of file");
      }
   }
   
   AcceptIdent(ident, "facts");
   AcceptToken(colon);

   kb_size = ParseLpair2TET(tetl,tetr);
   
   EndScanner();
   printf("Knowledge read.\n");

   return kb_size;
} 



/*-------------------------------------------------------------------------

FUNCTION         : long sink_args_TAT(long *found, long *ref_sum, term *t,
                                      TAT_p tat)
Beschreibung     : Sinks the arguments of t into the TAT's argument
                   list.

Globale Variable : -

Seiteneffekte    : I/O-Variables are changed.

Aenderungen      : <1> 7.2.1995 neu

-------------------------------------------------------------------------*/

long sink_args_TAT(long *found, long *ref_sum, term *t, TAT_p tat)
{
   long  ret_weight = 0;
   short i;

   if(t->fcode >0)  /* arity of variables is not guaranteed to be 0! */
   {
      for(i=0; i<t->arity; i++)
      {
	 ret_weight += SinkNTermTET(found, ref_sum, t->argument[i],
				    tat->args[i]);
      }
   }
   return ret_weight;
}

/*-------------------------------------------------------------------------

FUNCTION         : long  sink_args_TAT2(term *t, long norm, long
                                        unk_penalty, TAT_p tat)


Beschreibung     : Sinks the arguments of t into the TAT's argument
                   list, using SinkNTermTET2 as a base.

Globale Variable : -

Seiteneffekte    : I/O-Variables are changed.

Aenderungen      : <1> 7.2.1995 neu

-------------------------------------------------------------------------*/

long  sink_args_TAT2(term *t, long norm, long unk_penalty, TAT_p tat)
{
   long  ret_weight = 0;
   short i;

   if(t->fcode >0)  /* arity of variables is not guaranteed to be 0! */
   {
      for(i=0; i<t->arity; i++)
      {
	 ret_weight += SinkNTermTET2(t->argument[i], norm, unk_penalty,
				     tat->args[i]);
      }
   }
   return ret_weight;
}

/*-----------------------------------------------------------------------
//
// Function: sink_args_TAT3()
//
//   Sinks the arguments of t into the normalized TAT's argument list,
//   sums and returns the weight.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double sink_args_TAT3(term *t, TAT_p tat)
{
   double ret_weight = 0;
   short i;

   if(t->fcode >0)  /* arity of variables is not guaranteed to be 0! */
   {
      for(i=0; i<t->arity; i++)
      {
	 ret_weight += SinkNTermTET3(t->argument[i], tat->args[i]);
      }
   }
   return ret_weight;
}

/*-----------------------------------------------------------------------
//
// Function: norm_TAT_anno()
//
//   Put normal values into tat, assuming it describes a branch of
//   arity arity.
//
// Global Variables: 
//
// Side Effects    : 
//
/----------------------------------------------------------------------*/

void norm_TAT_anno(TAT_p tat, short arity, long proofs, long count,
		   long tot_ref)
{
   long i;

   if(!tat)
   {
      return;
   }
   tat->proofs_r  = 1-(tat->proofs -tat->proofs  * ProofsLimit)/proofs;
   tat->tot_ref_r = 1-(tat->tot_ref-tat->tot_ref * TotalLimit )/tot_ref;
   tat->count_r   = 1-(tat->count  -tat->count   * CountLimit )/count;
   tat->cell_weight = tat->proofs_r * tat->tot_ref_r;
/*   printf("(%ld, %ld, %ld)->(%f, %f, %f)\n",
	  tat->proofs, tat->tot_ref, tat->count,
	  tat->proofs_r, tat->tot_ref_r, tat->count_r); */

   for(i=0; i<arity; i++)
   {
      NormTETAnno(tat->args[i], proofs, count, tot_ref);
   }
}



/*-----------------------------------------------------------------------*/
/*                  Exportierte Funktionen                               */
/*-----------------------------------------------------------------------*/

#define SCAN_TET_BRANCHES(variable)                           \
        {                                                     \
           short i;                                           \
           long res = 0;                                      \
                                                              \
           for(i=0; i<MAXARITY; i++)                          \
           {                                                  \
              if(tet->branches[i])                            \
              {                                               \
	         res = max(res, tet->branches[i]->##variable);\
              }                                               \
           }                                                  \
           printf( #variable ": %ld\n",res);       \
           return res;                                        \
        }                                                     \

/*-----------------------------------------------------------------------
//
// Function: FindMaxProofs()
//
//   Calculate the maximum value of the variable "proofs" in the
//   complete TET. This routine makes use of the knowledge that this
//   value is monotonically falling with the depth of the TET, thus it
//   needs to check only the top branches.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long FindMaxProofs(TET_p tet)
SCAN_TET_BRANCHES(proofs);

/*-----------------------------------------------------------------------
//
// Function: FindMaxTotRef()
//
//   Calculate the maximum value of the variable "tot_ref" in the
//   complete TET. This routine makes use of the knowledge that this
//   value is monotonically falling with the depth of the TET, thus it
//   needs to check only the top branches.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long FindMaxTotRef(TET_p tet)
SCAN_TET_BRANCHES(tot_ref);

/*-----------------------------------------------------------------------
//
// Function: FindMaxCount()
//
//   Calculate the maximum value of the variable "count" in the
//   complete TET. This routine makes use of the knowledge that this
//   value is monotonically falling with the depth of the TET, thus it
//   needs to check only the top branches.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

long FindMaxCount(TET_p tet)
SCAN_TET_BRANCHES(count);


/*-----------------------------------------------------------------------
//
// Function: NormTETAnno()
//
//   
//
// Global Variables: -
//
// Side Effects    : Changes the values in the TET
//
/----------------------------------------------------------------------*/

void NormTETAnno(TET_p tet, long proofs, long count, long tot_ref)
{
   long i;
   
   if(!tet)
   {
      return;
   }
   
   for(i=0; i< MAXARITY; i++)
   {
      norm_TAT_anno(tet->branches[i], i, proofs, count, tot_ref);
   }
}


/*-------------------------------------------------------------------------

FUNCTION         : long SinkNTermTET(long *found, long* ref_sum, 
                                     term *t, TET_p tet) 

Beschreibung     : Lets the (normed) term sink into the TET.
                   *found is inremented by the number of matching
		   cells in the TET, ref_sum is incremented by the sum
		   of the "proofs" of these cells, the return value is
		   weight of sub-terms not matching. 

Globale Variable : -

Seiteneffekte    : *found and *ref_sum are modified

Aenderungen      : <1> 7.2.1995 neu

-------------------------------------------------------------------------*/

long SinkNTermTET(long *found, long* ref_sum, term *t, TET_p tet)
{
   long arity = t->fcode < 0 ? 0 : t->arity;

   if(!tet)
   {
      return t->weight;
   }
   
   if(tet->branches[arity])
   {
      (*found)++;
      (*ref_sum) += tet->branches[arity]->proofs;
      return sink_args_TAT(found, ref_sum, t,
			   tet->branches[arity]);
   }
   return t->weight;
}


/*-------------------------------------------------------------------------

FUNCTION         : long SinkNTermTET2(term* t, long norm, long
                                      unk_penalty, TET_p tet) 

Beschreibung     : Calculates the weight of a term by sinking it into
                   the tet and weigthing the termcells accordingly.

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 8.2.1994 neu

-------------------------------------------------------------------------*/

long SinkNTermTET2(term* t, long norm, long unk_penalty, TET_p tet)
{
   long symbol_w = t->fcode < 0? 1:2;
   long arity = t->fcode < 0 ? 0 : t->arity;

   if(!tet)
   {
      return t->weight*unk_penalty*norm;
   }
   if(tet->branches[arity])
   {
      return symbol_w+ (LearnInfluence*norm/(double)tet->branches[arity]->proofs)+
	 sink_args_TAT2(t, norm, unk_penalty, tet->branches[arity]);
   }
   return /*symbol_w+*/LearnInfluence*unk_penalty*norm*t->weight;
}

/*-----------------------------------------------------------------------
//
// Function: SinkNTermTET3()
//
//   Sink a term through a normalized TET and return the weight.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

double SinkNTermTET3(term* t, TET_p tet)
{
   long symbol_w = t->fcode < 0? 1:2;
   long arity = t->fcode < 0 ? 0 : t->arity;

   if(!tet)
   {
      return t->weight;
   }
   if(tet->branches[arity])
   {
      return symbol_w
	 * tet->branches[arity]->cell_weight
	 + sink_args_TAT3(t, tet->branches[arity]);
   }
   return t->weight;
}


/*-------------------------------------------------------------------------

FUNCTION         : double C1TETLearnWeight( termpair * tp)

Beschreibung     : Evaluates a critical pair by evaluating the two
                   terms using a TET consisting of all equations in
		   the cdata file.

Globale Variable : -

Seiteneffekte    : Might load the TET!

Aenderungen      : <1> 7.2.1995 neu

-------------------------------------------------------------------------*/

double C1TETLearnWeight( termpair * tp)
{
   static TET_p knowledge_tet = NULL;
   term           *left  = tp->left;
   term           *right = tp->right;
   long           kb_size = 1,
                  foundl = 1, foundr = 1, 
                  ref_suml = 0, ref_sumr = 0,
                  restl, restr, prev_exp;


   static bool    load_tried = false;


   if(!load_tried)
   {
      kb_size = read_global_tet(&knowledge_tet, true); 
      load_tried = true;
   }  
   
   restl = SinkNTermTET(&foundl, &ref_suml, left, knowledge_tet);
   restr = SinkNTermTET(&foundr, &ref_sumr, right, knowledge_tet);
   
   prev_exp = -LearnInfluence *
      (long)sqrt((double)(ProofsWeight*(ref_suml + 
			       ref_sumr)))/(kb_size+foundl+foundr);   

   return prev_exp + UnknownWeight*(restl +
				 restr)+left->weight+right->weight; 
}


/*-------------------------------------------------------------------------

FUNCTION         : double C2TETLearnWeight( termpair * tp)

Beschreibung     : Evaluates a critical pair by evaluating the two
                   terms using a tet and the "used is bad, but new is
		   worse"-Strategie.

Globale Variable : -

Seiteneffekte    : Might load the TET!

Aenderungen      : <1> 8.2.1995 neu

-------------------------------------------------------------------------*/

double C2TETLearnWeight( termpair * tp)
{
   static TET_p knowledge_tet = NULL;
   term           *left  = tp->left;
   term           *right = tp->right;
   long           kb_size = 1,
                  rweight,
                  lweight;
   static bool    load_tried = false;


   if(!load_tried)
   {
      kb_size = read_global_tet(&knowledge_tet, true); 
      printf("kb_size: %ld\n", kb_size);
      load_tried = true;
   }
   rweight = SinkNTermTET2(left,  kb_size, UnknownWeight, knowledge_tet);
   lweight = SinkNTermTET2(right, kb_size, UnknownWeight, knowledge_tet);

/*   printtpair(tp);
   printf("restl: %ld, ref_suml: %ld, foundl: %ld\n", restl, ref_suml,
   foundl); 
   printf("restr: %ld, ref_sumr: %ld, foundr: %ld\n", restr, ref_sumr,
   foundr); */

   return rweight + lweight;
}

/*-----------------------------------------------------------------------
//
// Function: C3TETLearnWeight
//
//   Evaluates a critical pair with a TET and relative values.
//
// Global Variables: -
//
// Side Effects    : Might load the TET
//
/----------------------------------------------------------------------*/

double C3TETLearnWeight( termpair * tp)
{
   static TET_p knowledge_tet = NULL;
   term           *left  = tp->left;
   term           *right = tp->right;
   long           kb_size = 1;
   double         rweight = 0,
                  lweight = 0;
   static bool    load_tried = false;

   if(!load_tried)
   {
      kb_size = read_global_tet(&knowledge_tet, true); 
      printf("kb_size: %ld\n", kb_size);
      load_tried = true;
      NormTETAnno(knowledge_tet,
		  FindMaxProofs(knowledge_tet),
		  FindMaxCount(knowledge_tet),
		  FindMaxTotRef(knowledge_tet));
   }
   rweight = SinkNTermTET3(left, knowledge_tet);
   lweight = SinkNTermTET3(right,knowledge_tet);

/*   printf("CPW: %f\n", rweight + lweight); */
   return rweight + lweight;
}


/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/


