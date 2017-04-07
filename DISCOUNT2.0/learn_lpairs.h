/*-------------------------------------------------------------------------

File        : learn_lpairs.h

Autor       : Stephan Schulz

Inhalt      : The data structure used to represent all the knowledge
              learned about one single lterm-pair, and functions
	      dealing with most of it's implementation. These are
	      analogous to the NormEqns from lrn_eqn.h.

Aenderungen : <1> 12.12.1994 neu
              <2> 8.3.1998 Beispielnamen in lpair aufgenommen

-------------------------------------------------------------------------*/

#ifndef _learn_lpair

#define _learn_lpair

#include "learn_lterms.h"


/*-----------------------------------------------------------------------*/
/*                   Typdeklarationen                                    */
/*-----------------------------------------------------------------------*/

typedef struct examplecell
{
   char*               name;    /* Example Name */
   double fitness;
   struct examplecell* next;
}ExampleCell, *example_p;

typedef struct lpaircell
{
   Lterm_p             lside;
   Lterm_p             rside;
   long                proofs;  /* Number of proofs it was used in */
   long                negative;/* Number of proofs the fact was
		 		  useless in */
   long                cp_cost; /* Cost this pair caused in useless
				  cp-inferences */
   long                ave_ref; /* Average number of uses per proof */
   long                tot_ref; /* Total number of uses in all proofs */
   long                goal_dist; /* Average goal Distance */
   long                balance;  /* For AVL-Trees */
   struct examplecell* example;  /* For Example Selection */
   struct lpaircell*   left;
   struct lpaircell*   right;
}LpairCell, *Lpair_p;

typedef struct learnsigcell
{
   char     *ident;
   short    arity;
}LearnSigCell, LearnSigCell_p;

typedef struct learnsig
{
   LearnSigCell sig_data[MAXFUNCTION]; /* Cell 0 is generally */
				       /* ignored... */
   short        entrycount;
}LearnSig, *LearnSig_p;

typedef struct
{ 
   long proofs;
   long proofs_accumulated;
   long ave_ref;
   long tot_ref;
   long goal_dist;
   long negative;
   long cp_cost;
   long cp_cost_accumulated;
}LpairTreeParams, *LpairTreeParams_p;


/*-----------------------------------------------------------------------*/
/*                 Deklaration exportierter Funktionen                   */
/*-----------------------------------------------------------------------*/

#define AllocExampleCell() (ExampleCell*)\
        SizeMalloc(sizeof(ExampleCell))
#define FreeExampleCell(junk)\
        SizeFree(junk, sizeof(ExampleCell))

#define AllocLpairCell() (LpairCell*)\
        SizeMalloc(sizeof(LpairCell))
#define FreeLpairCell(junk)\
	SizeFree(junk, sizeof(LpairCell))

#define AllocLearnSig() (LearnSig*)\
        SizeMalloc(sizeof(LearnSig))

#define FreeLearnSig(junk)\
        {short f;\
	    for(f=1;\
		f<=(junk)->entrycount;\
		free((junk)->sig_data[f++].ident));\
	}\
        SizeFree((junk), sizeof(LearnSig))

Lpair_p ParseLpair();
Lpair_p ParseLpairLine();
long    ParseLpairTree(Lpair_p *root, LpairTreeParams_p max_values);
Lpair_p ParseSpecTree(Lpair_p *root);
long    ParseLpair1TET(TET_p *root);
long    ParseLpair2TET(TET_p *rootl, TET_p *rootr);

void    PrintLpair(Lpair_p pair);
void    PrintLpairLine(Lpair_p pair);
void    PrintExampleList(example_p exa_handle);

void    FreeLpair(Lpair_p junk);
void    FreeLpairTree(Lpair_p junk);
long    CmpLpairs(Lpair_p pair1, Lpair_p pair2);
void    LpairTreeInsert(Lpair_p *root, Lpair_p pair);
void    PrintLpairTree(Lpair_p root);

LearnSig_p ParseLearnSig();
void       PrintLearnSig(LearnSig_p sig);
bool       LearnSigCompare(LearnSig_p sig1, LearnSig_p sig2);

extern bool PosAndNeg;

Lpair_p  TermpairToNormLpair(termpair* tp);

#endif


/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/





