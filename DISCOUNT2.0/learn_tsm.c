/*-----------------------------------------------------------------------

File  : learn_tsm.c

Author: Stephan Schulz

Contents
 
  Implementation of the general Term Space Map (TSM)  datatype and the
  corresponding subtypes and functions.

Changes

<1> 27.3.1997 new
<2> 13.4.1998 DeltaTSM und DeltaTSA eingefügt

-----------------------------------------------------------------------*/

#include "learn_tsm.h"

/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/

void print_tsa(TSA_p alt);
int  standard_tsm_map(short id, short arity, bool isvar);
long traverse_tsm_alts(TSA_p alt, 
		       process_tsm_cell procm,
		       process_tsa_cell proca);

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/


/*-------------------------------------------------------------------------
//
// Function: print_tsa()
//
//   Print a single TSA (with its arguments)
//
// Global Variables: -
//
// Side Effect     : Output
//
//-----------------------------------------------------------------------*/

void print_tsa(TSA_p alt)
{
   int i, count = 0;
   TSM_p tsm;
   
   if(!alt)
   {
      printf("WARNING: NULL-Pointer in print_tsa()");
   }
   else
   {
      printf("{TSA\n");

      for(i=0; i<MAXARITY; i++)
      {
	 if((tsm = alt->args[i]))
	 {
	    printf("accu_w[%2d] = %6ld eval_w[%d] = %1.5f\n",
		   i, alt->accu_w[i], i, alt->eval_w[i]);
	    count++;
	 }
      }
      printf("%d Arguments follow:\n", count);
      for(i=0; i<MAXARITY; i++)
      {
	 if((tsm = alt->args[i]))
	 {
	    printf("Argument %d\n", i);
	    PrintTSM(tsm);
	 }
      }
      printf("%d arguments found\n}\n", count);
   }
}

/*-------------------------------------------------------------------------
//
// Function:  standard_tsm_map()
//
//   Maps a norm-id, consisting of a (small) integer and an arity (if
//   not a variable) to a position in a TSM.
//
// Global Variables: -
//
// Side Effect     : The map is generated dynamically and maintained
//                   in static variables.
//
//-----------------------------------------------------------------------*/

int standard_tsm_map(short id, short arity, bool isvar)
{
   /* TSM_MAX_ALTS is an upper bound! */ 
   static int* idmap[MAXARITY] = {NULL}; 
   static int  map_size[MAXARITY] = {0};
   static int  nextpos = 0;
   short i;
   
   /* Variables are simply mapped to their id. Variables >
      TSM_MAX_VARS are all mapped to 0 (which is never used
      otherwise). */
   if(isvar)
   {
      if(id>=TSM_MAX_VARS)
      {
	 return 0;
      }
      return id;
   }
   else
   {
      if(map_size[arity]<=id)
      {
	 idmap[arity] = SecureRealloc(idmap[arity],(id+1)*sizeof(int));
	 for(i=map_size[arity]; i<=id; i++)
	 {
	    idmap[arity][i] = -1;
	 }
	 map_size[arity] = id+1;
      }
      if((idmap[arity])[id] == -1)
      {
	 (idmap[arity])[id] = TSM_MAX_VARS+nextpos++;
      }
   } 
/*   printf("f%d_%d = %d\n", arity,id, (idmap[arity])[id]);*/
   return (idmap[arity])[id];
}

/*-------------------------------------------------------------------------
//
// Function: traverse_tsa()
//
//   Traverse the TSA and apply proca() and procm() to the encountered
//   TSM- and TSACells (see TraverseTSM). Returns number of TSACells
//   encountered (including alt). 
//
// Global Variables: -
//
// Side Effect     : Those of proca() and procm()
//
//-----------------------------------------------------------------------*/

long traverse_tsa(TSA_p alt, process_tsm_cell procm,
		  process_tsa_cell proca)
{
   long nodes = 1,
        i;

   if(alt)
   {
      if(proca)
      {
	 proca(alt);
      }
      for(i=0; i<MAXARITY;i++)
      {
	 if(alt->args[i])
	 {
	    nodes += TraverseTSM(alt->args[i], procm, proca);
	 }
      }
   }
   return nodes;
}

/*-------------------------------------------------------------------------
//
// Function: DeltaTSA(TSA_p tsa1, TSA_p tsa2)
//
//   Calculates the difference between two TSAs.
//
// Global Variables: -
//
// Side Effect     : -
//
//-----------------------------------------------------------------------*/

double DeltaTSA(TSA_p tsa1, TSA_p tsa2)
{
   long i, n = 0;
   double res = 0;
  
   for (i = 0; i < MAXARITY; i++)
   {
      if(tsa1->args[i]) 
      {
         res += DeltaTSM(tsa1->args[i], tsa2->args[i]);
         n++;
      }
   }
   if (!n) return 0;
   return res/n;
}

/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
//
// Function:  AllocEmptyTSMDesc()
//
//   Allocate and initialize a TSMDescCell and return a pointer to
//   it. 
//
// Global Variables: -
//
// Side Effect     : Memory management
//
//-----------------------------------------------------------------------*/

TSMDesc_p AllocEmptyTSMDesc(tsm_lterm_map MapLterm, 
			    tsm_nterm_map MapNterm)
{
   TSMDesc_p handle;

   handle = AllocTSMDescCell();
   handle->MapLterm = MapLterm;
   handle->MapNterm = MapNterm;
   handle->tsm = NULL;
   handle->atsm = NULL;
   
   return handle;
}

/*-------------------------------------------------------------------------
//
// Function: FreeTSMDesc()
//
//   Free a TSMDescCell and the associated TSM.
//
// Global Variables: -
//
// Side Effect     : Memory management
//
//-----------------------------------------------------------------------*/

void FreeTSMDesc(TSMDesc_p junk)
{
   if(junk)
   {
      FreeTSM(junk->tsm);
      FreeTSM(junk->atsm);
      FreeTSMDescCell(junk);
   }
}


/*-------------------------------------------------------------------------
//
// Function: AllocEmptyTSM()
//
//   Return a pointer to a fresh, initialized TSM-Cell.
//
// Global Variables: -
//
// Side Effect     : Memory Management
//
//-----------------------------------------------------------------------*/

TSM_p AllocEmptyTSM(TSMDesc_p desc)
{
   TSM_p handle;
   int   i;
   
   handle = AllocTSMCell();
   handle->desc = desc;
   handle->used = 0;

   for(i=0;i<TSM_MAX_ALTS;i++)
   {
      handle->accu[i].pos_terms  = 0;
      handle->accu[i].neg_terms  = 0;
      handle->accu[i].pos_proofs = 0;
      handle->accu[i].neg_proofs = 0;
      handle->accu[i].pos_refs   = 0;
      handle->accu[i].cost       = 0;

      handle->eval[i].terms_eval  = 0;
      handle->eval[i].proofs_eval = 0;
      handle->eval[i].refs_eval   = 0;
      handle->eval[i].cost_eval   = 0;

      handle->alts[i] = NULL;
   }
   return handle;
}

/*-------------------------------------------------------------------------
//
// Function: FreeTSM()
//
//   Return a complete TMS to the internal memory management.
//
// Global Variables: -
//
// Side Effect     : Memory management
//
//-----------------------------------------------------------------------*/

void FreeTSM(TSM_p junk)
{
   int i;

   if(junk)
   {
      for(i=0;i<TSM_MAX_ALTS;i++)
      {
	 FreeTSA(junk->alts[i]);
      }
      FreeTSMCell(junk);
   }
}

/*-------------------------------------------------------------------------
//
// Function: AllocEmptyTSA()
//
//   Return a pointer to  fresh, initializes TSA.
//
// Global Variables: -
//
// Side Effect     : Memory management
//
//-----------------------------------------------------------------------*/

TSA_p AllocEmptyTSA(TSMDesc_p desc)
{
   TSA_p handle;
   int      i;
   
   handle = AllocTSACell();
   handle->desc = desc;
   
   
   for(i=0;i<MAXARITY;i++)
   {
      handle->accu_w[i] = 0;
      handle->eval_w[i] = 0;
      handle->args[i] = NULL;
   }
   return handle;
}

/*-------------------------------------------------------------------------
//
// Function: FreeTSA()
//
//   Return a complete TSA to the internal memory
//   management. 
//
// Global Variables: -
//
// Side Effect     : Memory management
//
//-----------------------------------------------------------------------*/

void FreeTSA(TSA_p junk)
{
   int i;
   
   if(junk)
   {
      for(i=0;i<MAXARITY;i++)
      {
	 FreeTSM(junk->args[i]);
      }
      FreeTSACell(junk);
   }
}

/*-------------------------------------------------------------------------
//
// Function: StandardTSMLtermMap()
//
//   Maps a Lterm onto a TSM alternative.
//
// Global Variables: -
//
// Side Effect     : By calling standard_tsm_map()
//
//-----------------------------------------------------------------------*/

int StandardTSMLtermMap(Lterm_p term)
{
   return standard_tsm_map(term->norm_id, term->arity, term->isvar);
}

/*-------------------------------------------------------------------------
//
// Function: StandardTSMNtermMap()
//
//   Maps a normalized DISCOUNT tern onto a TSM alternative.
//
// Global Variables: -
//
// Side Effect     : By calling standard_tsm_map()
//
//-----------------------------------------------------------------------*/

int StandardTSMNtermMap(term* term, DNormSubst_p subst)
{
   if(term->fcode < 0)
   {
      return standard_tsm_map(VTfind(subst->variables,term->fcode),
			      0, true);
   }
   else
   {
      return standard_tsm_map(subst->norm_id[term->fcode],term->arity,false);
   }
}

/*-------------------------------------------------------------------------
//
// Function: PrintTSM()
//
//   Print a TSM (mainly for debugging and studying)
//
// Global Variables: -
//
// Side Effect     : Output
//
//-----------------------------------------------------------------------*/

void PrintTSM(TSM_p tsm)
{
   int i, count =0;
   TSA_p alt;

   if(!tsm)
   {
      printf("WARNING: NULL-Pointer in PrintTSM()");
   }
   else
   {
      printf("{ New TSM\n");
      
      printf("Used: %d Alternatives\n", tsm->used);
      
      for(i=0; i<TSM_MAX_ALTS; i++)
      {
	 if((alt = tsm->alts[i]))
	 {
	    printf("Alt %2d: pos_terms=%5ld neg_terms=%5ld "
		   "pos_proofs=%5ld neg_proof=%5ld pos_refs=%5ld "
		   "cost=%5ld\n", i,
		   tsm->accu[i].pos_terms, tsm->accu[i].neg_terms,
		   tsm->accu[i].pos_proofs, tsm->accu[i].neg_proofs,
		   tsm->accu[i].pos_refs, tsm->accu[i].cost);
	    printf("        terms_eval=%1.5f proofs_eval=%1.5f"
		   "        refs_eval=%1.5f cost_eval=%1.5f\n",
		   tsm->eval[i].terms_eval, tsm->eval[i].proofs_eval,
		   tsm->eval[i].refs_eval, tsm->eval[i].cost_eval);
		   
	    count++;
	 }
      }
      for(i=0; i<TSM_MAX_ALTS; i++)
      {
	 if((alt = tsm->alts[i]))
	 {
	    print_tsa(alt);
	 }
      }
      printf("Found: %d Alternatives\n}\n", count);
   }
}



/*-------------------------------------------------------------------------
//
// Function: TSAInsertLterm()
//
//   Insert an Lterm and the associated data into a given TSA.
//   Returns a pointer to the alternative.
//
// Global Variables: -
//
// Side Effect     : Changes the TSM
//
//-----------------------------------------------------------------------*/

TSA_p TSAInsertLterm(TSMDesc_p desc, TSA_p *tsa, Lterm_p t, 
		     TSMData* data)
{
   short   i = 0;
   Lterm_p handle;

   if(!(*tsa))
   {
      *tsa = AllocEmptyTSA(desc);
   }   
    
   for(handle=t->args; handle; handle = handle->chain)
   {   
      (*tsa)->accu_w[i] += handle->term_weight * data->pos_proofs;
      (*tsa)->accu_n[i] += handle->term_weight * data->neg_proofs;
      
      TSMInsertLterm(desc, &((*tsa)->args[i++]), handle, data);
   }
   return *tsa;
}

/*-------------------------------------------------------------------------
//
// Function: TSMInsertLterm()
//
//   Insert a term (and associated data) into a TSM. Returns pointer
//   to TSM.
//
// Global Variables: -
//
// Side Effect     : Changes TSM
//
//-----------------------------------------------------------------------*/

TSM_p TSMInsertLterm(TSMDesc_p desc, TSM_p *tsm, Lterm_p term,
		     TSMData* data)
{
   int index;

   if(!(*tsm))
   {
      *tsm = AllocEmptyTSM(desc);
   }
   index = desc->MapLterm(term);
   
   (*tsm)->accu[index].pos_terms  += data->pos_terms;
   (*tsm)->accu[index].neg_terms  += data->neg_terms;
   (*tsm)->accu[index].pos_proofs += data->pos_proofs;
   (*tsm)->accu[index].neg_proofs += data->neg_proofs;
   (*tsm)->accu[index].pos_refs   += data->pos_refs;
   (*tsm)->accu[index].cost       += data->cost;
   TSAInsertLterm(desc, &((*tsm)->alts[index]), term, data);

   return *tsm;
}


/*-------------------------------------------------------------------------
//
// Function: ParseLpairTSM()
//
//   Reads a list of equations (Lpairs with data) from the current
//   input string and builds a TSM. Returns the number of Lterms
//   parsed. 
//
// Global Variables: -
//
// Side Effect     : Input, changes the tree, memory operations by
//                   calling subroutines.
//
//-----------------------------------------------------------------------*/
 
long ParseLpairTSM(TSMDesc_p desc, example_p goodexamples)
{
   Lpair_p handle;
   long    count = 0;
   TSMData data;

   while(TestToken(identifier) && !ColonFollows)
   {
      handle = ParseLpairLine();

      if(!IsEmptyIntersection(handle->example, goodexamples))
      {
         data.pos_terms  = handle->proofs? 1:0;
         data.neg_terms  = handle->negative? 1:0;
         data.pos_proofs = handle->proofs;
         data.pos_refs   = handle->tot_ref;
         data.neg_proofs = handle->negative;
         data.cost       = handle->cp_cost;
         count++;
         TSMInsertLterm(desc, &(desc->tsm), handle->lside, &data);
         count++;
         TSMInsertLterm(desc, &(desc->tsm), handle->rside, &data);
      }
      FreeLpair(handle);
   }
   return count;
}

/*-------------------------------------------------------------------------
//
// Function: ParsePlainLpairTSM()
//
//   Reads a list of equations (Lpairs withOUT data) from the current
//   input string and builds a TSM. Returns the number of Lterms
//   parsed. 
//
// Global Variables: -
//
// Side Effect     : Input, changes the tree, memory operations by
//                   calling subroutines.
//
//-----------------------------------------------------------------------*/
 
long ParsePlainLpairTSM(TSMDesc_p desc)
{
   Lpair_p handle;
   long    count = 0;
   TSMData data;

   while(TestToken(identifier))
   {
      handle = ParseLpair();
      count++;
      TSMInsertLterm(desc, &(desc->tsm), handle->lside, &data); 
      count++;
      TSMInsertLterm(desc, &(desc->tsm), handle->rside, &data); 
      /*      FreeLpair(handle);  Muss raus, sonst Segmentation Fault */
   }
   return count;
}

/*-------------------------------------------------------------------------
//
// Function: ParseLpairOTSM()
//
//   Reads a list of equations (Lpairs with data) from the current
//   input string and builds an ordered TSM (i.e. putting left terms
//   into desc->tsm, right terms into desc->atsm). Returns the number
/    of Lterms parsed. 
//
// Global Variables: -
//
// Side Effect     : Input, changes the tree, memory operations by
//                   calling subroutines.
//
//-----------------------------------------------------------------------*/
 
long ParseLpairOTSM(TSMDesc_p desc, example_p goodexamples)
{
   Lpair_p handle;
   long    count = 0;
   TSMData data;
                     
   while(TestToken(identifier) && !ColonFollows)
   {
      handle = ParseLpairLine();
      
      if(!IsEmptyIntersection(handle->example, goodexamples))
      {
         data.pos_terms  = handle->proofs? 1:0;
         data.neg_terms  = handle->negative? 1:0;
         data.pos_proofs = handle->proofs;
         data.pos_refs   = handle->tot_ref;
         data.neg_proofs = handle->negative;
         data.cost       = handle->cp_cost;
         count++;
         TSMInsertLterm(desc, &(desc->tsm), handle->lside, &data);
         count++;
         TSMInsertLterm(desc, &(desc->atsm), handle->rside, &data);
      }
      FreeLpair(handle);
   }
   return count;
}


/*-------------------------------------------------------------------------
//
// Function: TraverseTSM()
//
//   Traverse the TSM in pre-order, apply the function procm() to all
//   TSMCells encountered and proca() to all TSACells. Return value is
//   the total number of valid TSAs in the TSM.
//
// Global Variables: -
//
// Side Effect     : Those of proca() and procm() (usually
//                   manipulating of the internal state of the Cells)
//
//-----------------------------------------------------------------------*/

long TraverseTSM(TSM_p tsm, process_tsm_cell procm, 
		 process_tsa_cell proca)
{
   long nodes = 0,
        i;
   
   if(tsm)
   {
      if(procm)
      {
	 procm(tsm);
      }
      for(i=0; i<TSM_MAX_ALTS;i++)
      {
	 if(tsm->alts[i])
	 {
	    nodes += traverse_tsa(tsm->alts[i],
				       procm, proca);
	 }
      }
   }
   return nodes;
}

/*-------------------------------------------------------------------------
//
// Function: DeltaTSM(TSM_p tsm1, TSM_p tsm2)
//
//   Calculates the difference between two TSMs.
//
// Global Variables: -
//
// Side Effect     : -
//
//-----------------------------------------------------------------------*/

double DeltaTSM(TSM_p tsm1, TSM_p tsm2)
{
   TSM_p t;
   long i, n1 = 0, n2 = 0;
   double res = 0;

   for(i = 0; i < TSM_MAX_ALTS; i++) 
   { 
      if(tsm1->alts[i]) n1++;
      if(tsm2->alts[i]) n2++;
   }
   if(n1<n2) {t = tsm1; tsm1 = tsm2; tsm2 = t; n1 = n2;}

   if(!n1) return 0;

   for (i = 0; i < TSM_MAX_ALTS; i++)
   {
      if (tsm1->alts[i])
      {
         if(tsm2->alts[i]) res+= DeltaTSA(tsm1->alts[i], tsm2->alts[i]);
         else res+=1;
      }
   }
   return res/n1;
}

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


