/*-------------------------------------------------------------------------

File        : lrn_pcltrans.h

Autor       : Stephan Schulz

Inhalt      : Funktionsdeklarationen zu Funktionen, die (im
              wesentlichen) die Teile des PCL-Listings projezieren und
	      daraus notwendige Informationen ableiten.

Aenderungen : <1> 28.6.1994 neu
              <2> 30.8.1994 Uebernhme der Build*()-Funktionen aus
	      lrn_makedom (nun obsolet).

-------------------------------------------------------------------------*/

#ifndef _lrn_pcltrans

#define _lrn_pcltrans


#include "lrn_domains.h"


/*-----------------------------------------------------------------------*/
/*                   Typdeklarationen                                    */
/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/*                 Deklaration exportierter Funktionen                   */
/*-----------------------------------------------------------------------*/


void      CalcGoalDistAndWeights(Step_p anchor);
void      SetExampleName(char* name);
NormEqn_p GetStep(Step_p step);
NormEqn_p GetAxioms(Step_p anchor);
NormEqn_p GetGoals(Step_p anchor);
NormEqn_p GetLemmas(Step_p anchor);
NormEqn_p GetFacts(Step_p anchor);

Dom_p   BuildSpecDom(Step_p anchor, char* example, char* dom);
Dom_p   BuildGoalDom(Step_p anchor, char* example, char* dom);

Brain_p BuildBrain(Step_p anchor, char* example, char* dom, BrainType
                   type); 

BOOL    IsProofProtocol(Step_p anchor);



#endif


/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/





