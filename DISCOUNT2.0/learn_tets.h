/*-------------------------------------------------------------------------

File        : learn_tets.h

Autor       : Stephan Schulz

Inhalt      : Deklarations for and of Functions to evaluate critical
              pairs based on the concept of TETS and TATS ("learning
	      by term space mapping") 

Aenderungen : <1> 5.1.1996 new (extracted from learn_cpweight.h)

-------------------------------------------------------------------------*/

#ifndef _learn_tets

#define _learn_tets

#include <math.h>
#include "learn_cpweight.h"

/*-----------------------------------------------------------------------*/
/*                   Typdeklarationen                                    */
/*-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------*/
/*       Deklaration exportierter Funktionen und Variablen               */
/*-----------------------------------------------------------------------*/


long FindMaxProofs(TET_p tet);
long FindMaxTotRef(TET_p tet);
long FindMaxCount(TET_p tet);

void NormTETAnno(TET_p tet, long proofs, long count, long tot_ref);

long   SinkNTermTET(long *found, long* ref_sum, term *t, TET_p tet);
long   SinkNTermTET2(term* t, long norm, long unk_penalty, TET_p tet);
double SinkNTermTET3(term* t, TET_p tet);

double C1TETLearnWeight( termpair * tp);
double C2TETLearnWeight( termpair * tp);
double C3TETLearnWeight( termpair * tp);

/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/

#endif





