/*-----------------------------------------------------------------------

File  : learn_tsm.h

Author: Stephan Schulz

Contents
 
  Definition of the general Term Space Map (TSM)  datatype and the
  corresponding subtypes and functions.

Changes

<1> 26.3.1997 new

-----------------------------------------------------------------------*/

#ifndef learn_tsm

#define learn_tsm

#include "learn_tets.h"

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/* The following datatype contains the data that is inserted with each
   term cell into the TSM. It is also used in the TSM to accumulate
   the information. */

typedef struct tsmdata
{
   long pos_terms; 
   long neg_terms;
   long pos_proofs;
   long neg_proofs;
   long pos_refs;
   long cost;
}TSMData;

/* This structure is used to store distribution data about the
   different alternatives in a single tsm. */

typedef struct tsmdist
{
   double terms_eval; 
   double proofs_eval;
   double neg_eval;
   double refs_eval;
   double cost_eval;
}TSMDist;


/* Now for the general TSM datatype. A TSM maps a term (currently only
   its top symbol) to one of many Term Space Alternatives (TSAs). It
   is realized as an array of these alternatives (defined below) with
   corresponding data. The number of alternatives is fixed by the
   constants TSM_MAX_VARS and TSM_MAX_FUNS. A usable TSM consists of a
   descriptor and a pointer to the recursive map.
   
   TSMs are described by a descriptor containing a pointer to the
   recursive TSM and pointers to the functions realizing the mapping
   of normed discount terms (nterms) and the terms used in the
   learning routines (lterms) to alternatives.  */

/* Function pointer: Given an Lterm, return an alternative. */
typedef int (*tsm_lterm_map)(Lterm_p);
/* Ditto for Nterms */
typedef int (*tsm_nterm_map)(term*,DNormSubst_p);

typedef struct tsmdesccell
{
   tsm_lterm_map   MapLterm;
   tsm_nterm_map   MapNterm;
   struct tsmcell  *tsm;  /* Basic tsm, usued by tsmstandard/tsmabsolute */
   struct tsmcell  *atsm; /* For second terms of ordered tsms, used by
			     otsmstandard */
}TSMDescCell,*TSMDesc_p;

#define TSM_MAX_VARS 20
#define TSM_MAX_FUNS 20
#define TSM_MAX_ALTS (TSM_MAX_VARS+TSM_MAX_FUNS)

typedef struct tsmcell
{
   TSMDesc_p        desc;
   TSMData          accu[TSM_MAX_ALTS];
   TSMDist          eval[TSM_MAX_ALTS];
   int              used;
   struct tsa_cell* alts[TSM_MAX_ALTS];
}TSMCell,*TSM_p;


/* This is the datatype containing the information about individual
   alternatives, including the TSMs representing the subterms. */

typedef struct tsa_cell
{
   TSMDesc_p       desc;
   long            accu_w[MAXARITY];
   long            accu_n[MAXARITY];
   double          eval_w[MAXARITY];
   double          eval_n[MAXARITY];
   TSM_p           args[MAXARITY];
}TSACell,*TSA_p;

/* The type of a generic function that can be applied to a valid TSM
   in a consistent state (i.e. at least an initalized TSMCell */ 
typedef void (*process_tsm_cell)(TSM_p);
typedef void (*process_tsa_cell)(TSA_p);


/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

#define AllocTSMDescCell() (TSMDescCell*)SizeMalloc(sizeof(TSMDescCell))
#define FreeTSMDescCell(junk)            SizeFree(junk, sizeof(TSMDescCell))

#define AllocTSMCell() (TSMCell*)SizeMalloc(sizeof(TSMCell))
#define FreeTSMCell(junk)        SizeFree(junk, sizeof(TSMCell))

#define AllocTSACell() (TSACell*)SizeMalloc(sizeof(TSACell))
#define FreeTSACell(junk)        SizeFree(junk, sizeof(TSACell))

TSMDesc_p AllocEmptyTSMDesc(tsm_lterm_map MapLterm, 
			    tsm_nterm_map MapNterm);
void FreeTSMDesc(TSMDesc_p junk);

TSM_p AllocEmptyTSM(TSMDesc_p desc);
void  FreeTSM(TSM_p junk);

TSA_p AllocEmptyTSA(TSMDesc_p desc);
void  FreeTSA(TSA_p junk);

int  StandardTSMLtermMap(Lterm_p term);
int  StandardTSMNtermMap(term* term, DNormSubst_p subst);

void     PrintTSM(TSM_p tsm);
TSA_p    TSAInsertLterm(TSMDesc_p desc, TSA_p *tsa, Lterm_p t, TSMData* data); 
TSM_p    TSMInsertLterm(TSMDesc_p desc, TSM_p *tsm, Lterm_p t, TSMData* data); 

long     ParseLpairTSM(TSMDesc_p desc, example_p goodexamples);
long     ParsePlainLpairTSM(TSMDesc_p desc);
long     ParseLpairOTSM(TSMDesc_p desc, example_p goodexamples);

long     TraverseTSM(TSM_p tsm, process_tsm_cell proc, 
		     process_tsa_cell procalt);
double   DeltaTSM(TSM_p tsm1, TSM_p tsm2);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

