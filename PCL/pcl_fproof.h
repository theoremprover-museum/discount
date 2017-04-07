


/*************************************************************************/
/*                                                                       */
/*   File:        pcl_fproof.h                                           */
/*                                                                       */
/*   Autor:       Stephan Schulz                                         */
/*                                                                       */
/*   Inhalt:      Deklarationen zu Funktionen, die konstruktive PCL-     */
/*                Begruendungen in Gleichheitsketten umwandeln.          */
/*                                                                       */
/*   Aenderungen: <1> 7.2.1992                                           */
/*                                                                       */
/*************************************************************************/

#ifndef _pcl_fproof

#define _pcl_fproof

#include "pcl_eqchains.h"



/*----------------------------------------------------------------------------*/
/*                    Deklaration exportierter Funktionen                     */
/*----------------------------------------------------------------------------*/

EqChain_p proof_f_step(Just_p just);

void      AllStepsNewVars(Step_p anchor);

EqChain_p ProofFStep(Step_p step);

long      ProofForward(Step_p anchor);


#endif

/*----------------------------------------------------------------------------*/
/*                         Ende des Files                                     */
/*----------------------------------------------------------------------------*/


