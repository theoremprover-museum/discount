/*-----------------------------------------------------------------------

File  : learn_tsmexperts.c

Author: Stephan Schulz

Contents
 
  Implementation of experts using the general TSM data structure. 

Changes

<1> 31.3.1997 new
<2> 9.3.1998 Example Selection

-----------------------------------------------------------------------*/

#include "learn_tsmexperts.h"


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

TSMData max_values = {0,0,0,0,0,0};
TSMData max_avalues = {0,0,0,0,0,0};

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/

void   tsm_calc_altdistrib(TSM_p tsm);
void   tsa_calc_argdistrib(TSA_p alt);

double tsm_distrib_eval(TSM_p tsm, term* t, DNormSubst_p subst);
double tsa_distrib_eval(TSA_p alt, term* t, DNormSubst_p subst);

/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/

#define calc_prop(sample,sum) ((sum)?((double)(sample)/(double)(sum)):0)

/*-------------------------------------------------------------------------
//
// Function: tsm_calc_altdistrib()
//
//   Compute and set the eval-values for the alternatives anchored at
//   tsm. 
//
// Global Variables: -
//
// Side Effect     : Sets the values.
//
//-----------------------------------------------------------------------*/

void tsm_calc_altdistrib(TSM_p tsm)
{
   int     i;
   TSMData maximum = {0,0,0,0,0,0};

   tsm->used = 0;

   if(!TSMAbsolutes)
   {
      for(i=0; i<TSM_MAX_ALTS;i++)
      {
	 if(tsm->alts[i])
	 {
	    tsm->used++;
	    maximum.pos_terms  = max(maximum.pos_terms,  tsm->accu[i].pos_terms);
	    maximum.neg_terms  = max(maximum.neg_terms,  tsm->accu[i].neg_terms);
	    maximum.pos_proofs = max(maximum.pos_proofs, tsm->accu[i].pos_proofs);
	    maximum.neg_proofs = max(maximum.neg_proofs, tsm->accu[i].neg_proofs);
	    maximum.pos_refs   = max(maximum.pos_refs,   tsm->accu[i].pos_refs);
	    maximum.cost       = max(maximum.cost,       tsm->accu[i].cost); 
	 }
      }
   }
   else
   {
      maximum = max_values;
   }
   for(i=0; i<TSM_MAX_ALTS;i++)
   {
      if(tsm->alts[i])
      {
	 tsm->eval[i].terms_eval  = calc_prop(tsm->accu[i].pos_terms,
					      maximum.pos_terms);
/*	 printf("terms_eval[%d] = %f\n", i, alt->store.terms_eval);*/
	 tsm->eval[i].proofs_eval = calc_prop(tsm->accu[i].pos_proofs,
					      maximum.pos_proofs);
	 tsm->eval[i].neg_eval    = calc_prop(tsm->accu[i].neg_proofs,
					      maximum.neg_proofs);
	 tsm->eval[i].refs_eval   = calc_prop(tsm->accu[i].pos_refs,
					      maximum.pos_refs);
	 tsm->eval[i].cost_eval = calc_prop(tsm->accu[i].cost,
					     maximum.cost);

      }
   }  
}


/*-------------------------------------------------------------------------
//
// Function: tsa_calc_argdistrib()
//
//   Calculate the term weight distribution of the arguments of the
//   TSA.
//
// Global Variables: -
//
// Side Effect     : Sets the values
//
//-----------------------------------------------------------------------*/

void tsa_calc_argdistrib(TSA_p tsa)
{
   int i;
   long weight = 0,
        neg_weight = 0;
   
   for(i=0; i<MAXARITY;i++)
   {
      if(tsa->args[i])
      {
	 weight     = max(weight, tsa->accu_w[i]);
	 neg_weight = max(neg_weight, tsa->accu_n[i]);
      }
   }
   for(i=0; i<MAXARITY;i++)
   {
      if(tsa->args[i])
      {
	 tsa->eval_w[i] = calc_prop(tsa->accu_w[i], weight);
	 tsa->eval_n[i] = calc_prop(tsa->accu_n[i], neg_weight);
      }
   }     
}


/*-------------------------------------------------------------------------
//
// Function: tsm_distrib_eval()
//
//   Evaluate a term by mapping it to a tsm and modifying the standard
//   node weight according to the weights corresponding to the
//   tsm-alternative. 
//
// Global Variables: -
//
// Side Effect     : -
//
//-----------------------------------------------------------------------*/

double tsm_distrib_eval(TSM_p tsm, term* t, DNormSubst_p subst)
{
   double ret, fak;
   TSA_p  alt;
   int    index;

   ret = t->weight; /* May be overwritten if appropriate TSM exists */

   /* PrintDNormTerm(t, subst);*/
   if(tsm)
   {
      index =(tsm->desc)->MapNterm(t,subst);
      alt = tsm->alts[index];
      if(alt)
      {
	 fak = (1-(ProofsLimit*tsm->eval[index].proofs_eval))
	    * (1+(NegativeLimit*tsm->eval[index].neg_eval));

	 if(PosLimitsNeg && tsm->accu[index].pos_proofs)
	 {
	    fak = min(fak,1);
	 }
	 ret = (varp(t)? 1:2 ) * fak;

	 ret = ret + tsa_distrib_eval(alt, t, subst);
      }
      else 
      {
	 /*printf("Unknown subterm with weight %f\n",ret);*/
      }
   }
   return ret;
}

/*-------------------------------------------------------------------------
//
// Function: tsa_distrib_eval()
//
//   Evaluate the arguments of a term by mapping them to the arguments
//   of the given alternative.
//
// Global Variables: -
//
// Side Effect     : -
//
//-----------------------------------------------------------------------*/

double tsa_distrib_eval(TSA_p alt, term* t, DNormSubst_p subst)
{
   int    i;
   double ret = 0,
          fak;
   
   for(i=0; i<t->arity;i++)
   {
      if(!alt->args[i])
      {
	 Error(__FILE__ ": tsmalt_distrib_eval()", 
	       "TSM for the argument in an alternative is missing!");
      }
      /* fak = (1-0.1*alt->eval_w[i])*(1+0.0*alt->eval_n[i]);*/
      fak = 1;
      ret = ret + fak * 
	 tsm_distrib_eval(alt->args[i], t->argument[i], subst);
   }

   return ret;
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
//
// Function: CreateTSMFromKB()
//
//   Read the global data from the general knowledge base and compile
//   it into the (usually empty) TSM handed to it. Return value is the
//   number of terms parsed.
//
// Global Variables: 
//
// Side Effect     : I/O, by calling various functions.
//
//-----------------------------------------------------------------------*/

long CreateTSMFromKB(TSMDesc_p desc, bool complete)
{
   long terms_parsed,
        nodes;
   int i;
   example_p goodexamples;
   bool HasSelData;

   NormTermPreserveArity = GetBoolKBVariable("PreserveArity");
   if(!NormTermPreserveArity)
   {
      Error(__FILE__ ": CreateTSMFromKB()", 
	    "TSMs currently require a knowledge base where each"
	    " identifier has a unique and permanent arity");
   }
   PosAndNeg = GetBoolKBVariable("PosAndNeg");
   strcpy(PathNameHelp, KBFullName());
   strcat(PathNameHelp, complete ? "/cdata":"/pdata");
   
   HasSelData = GetBoolKBVariable("HasSelData");
   goodexamples = GetExaList(PathNameHelp,HasSelData && !no_exasel);

   InitScanner(PathNameHelp);
   printf("Reading global knowledge from %s into TSM.\n",
	  PathNameHelp);

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
   terms_parsed = ParseLpairTSM(desc, goodexamples);
      
   EndScanner();
   
   if(TSMAbsolutes && desc->tsm)
   {
      for(i=0;i<TSM_MAX_ALTS;i++)
      {
	 if(desc->tsm->alts[i])
	 {
	    max_values.pos_terms = 
	       max(max_values.pos_terms,desc->tsm->accu[i].pos_terms);
	    max_values.neg_terms = 
	       max(max_values.neg_terms,desc->tsm->accu[i].neg_terms);
	    max_values.pos_proofs = 
	       max(max_values.pos_proofs,desc->tsm->accu[i].pos_proofs);
	    max_values.neg_proofs = 
	       max(max_values.neg_proofs,desc->tsm->accu[i].neg_proofs);
	    max_values.pos_refs = 
	       max(max_values.pos_refs,desc->tsm->accu[i].pos_refs);
	    max_values.cost = 
	       max(max_values.cost,desc->tsm->accu[i].cost);
	 }
      }
   }
   nodes = TraverseTSM(desc->tsm, tsm_calc_altdistrib,
		       tsa_calc_argdistrib);
   printf("Knowledge (%ld term patterns) compiled into TSM with a"
	  " total of %ld alternatives.\n", terms_parsed, nodes);
   return terms_parsed;
}


/*-------------------------------------------------------------------------
//
// Function: CreateOTSMFromKB()
//
//   Read the global data from the general knowledge base and compile
//   it into the (usually empty) ordered TSM handed to it. Return
//   value is the number of terms parsed.
//
// Global Variables: 
//
// Side Effect     : I/O, by calling various functions.
//
//-----------------------------------------------------------------------*/

long CreateOTSMFromKB(TSMDesc_p desc, bool complete)
{
   long terms_parsed,
        nodes;
   int i;
   example_p goodexamples;
   bool HasSelData;   

   NormTermPreserveArity = GetBoolKBVariable("PreserveArity");
   if(!NormTermPreserveArity)
   {
      Error(__FILE__ ": CreateTSMFromKB()", 
            "TSMs currently require a knowledge base where each"
            " identifier has a unique and permanent arity");
   }
   PosAndNeg = GetBoolKBVariable("PosAndNeg");
   strcpy(PathNameHelp, KBFullName());
   strcat(PathNameHelp, complete ? "/cdata":"/pdata");

   HasSelData = GetBoolKBVariable("HasSelData");
   goodexamples = GetExaList(PathNameHelp,HasSelData);
   
   InitScanner(PathNameHelp);
   printf("Reading global knowledge from %s into TSM.\n",
          PathNameHelp);
   
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
   terms_parsed = ParseLpairOTSM(desc, goodexamples);
      
   EndScanner();
   
   if(TSMAbsolutes && desc->tsm)
   {
      for(i=0;i<TSM_MAX_ALTS;i++)
      {
         if(desc->tsm->alts[i])
         {
            max_values.pos_terms = 
               max(max_values.pos_terms,desc->tsm->accu[i].pos_terms);
            max_values.neg_terms = 
               max(max_values.neg_terms,desc->tsm->accu[i].neg_terms);
            max_values.pos_proofs = 
               max(max_values.pos_proofs,desc->tsm->accu[i].pos_proofs);
            max_values.neg_proofs = 
               max(max_values.neg_proofs,desc->tsm->accu[i].neg_proofs);
            max_values.pos_refs = 
               max(max_values.pos_refs,desc->tsm->accu[i].pos_refs);
            max_values.cost = 
               max(max_values.cost,desc->tsm->accu[i].cost);
         }
	 if(desc->atsm->alts[i])
         {
            max_avalues.pos_terms = 
               max(max_avalues.pos_terms,desc->atsm->accu[i].pos_terms);
            max_avalues.neg_terms = 
               max(max_avalues.neg_terms,desc->atsm->accu[i].neg_terms);
            max_avalues.pos_proofs = 
               max(max_avalues.pos_proofs,desc->atsm->accu[i].pos_proofs);
            max_avalues.neg_proofs = 
               max(max_avalues.neg_proofs,desc->atsm->accu[i].neg_proofs);
            max_avalues.pos_refs = 
               max(max_avalues.pos_refs,desc->atsm->accu[i].pos_refs);
            max_avalues.cost = 
               max(max_avalues.cost,desc->atsm->accu[i].cost);
         }      
      }
   }
   nodes = TraverseTSM(desc->tsm, tsm_calc_altdistrib,
                       tsa_calc_argdistrib);
   max_values = max_avalues; /* Ugly hack! I want partially evaluated
				functions! */
   nodes += TraverseTSM(desc->atsm, tsm_calc_altdistrib,
                       tsa_calc_argdistrib);
    
   printf("Knowledge (%ld term patterns) compiled into ordered TSM with a"
          " total of %ld alternatives.\n", terms_parsed, nodes);
   return terms_parsed;
}


/*-------------------------------------------------------------------------
//
// Function: StandardTSMWeight()
//
//   TSM-based evaluation function.
//
// Global Variables: CompleteKB (from learn_cpweight.h)
//
// Side Effect     : May load knowledge base (stored in heap via
//                   static variables).
//
//-----------------------------------------------------------------------*/

double StandardTSMWeight(termpair *tp)
{
   static       TSMDesc_p desc = NULL;
   double       eval = 0;
   DNormSubst_p subst;
   term         *left  = tp->left;
   term         *right = tp->right;
   
   if(!desc)
   {
      desc = AllocEmptyTSMDesc(StandardTSMLtermMap,
			       StandardTSMNtermMap);
      CreateTSMFromKB(desc, CompleteKB);
   }
   subst = OrderNormTerms(&left, &right);

   eval += tsm_distrib_eval(desc->tsm, left,  subst);
   eval += tsm_distrib_eval(desc->tsm, right, subst);
   FreeDNormSubst(subst);

   return eval;
}

/*-------------------------------------------------------------------------
//
// Function: StandardOTSMWeight()
//
//   OTSM-based evaluation function.
//
// Global Variables: CompleteKB (from learn_cpweight.h)
//
// Side Effect     : May load knowledge base (stored in heap via
//                   static variables).
//
//-----------------------------------------------------------------------*/

double StandardOTSMWeight(termpair *tp)
{
   static       TSMDesc_p desc = NULL;
   double       eval = 0;
   DNormSubst_p subst;
   term         *left  = tp->left;
   term         *right = tp->right;
   
   if(!desc)
   {
      desc = AllocEmptyTSMDesc(StandardTSMLtermMap,
                               StandardTSMNtermMap);
      CreateOTSMFromKB(desc, CompleteKB);
   }
   subst = OrderNormTerms(&left, &right);

   eval += tsm_distrib_eval(desc->tsm, left,  subst);
   eval += tsm_distrib_eval(desc->atsm, right, subst);
   FreeDNormSubst(subst);

   return eval;
}


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/


