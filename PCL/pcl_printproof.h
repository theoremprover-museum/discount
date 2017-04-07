


/*************************************************************************/
/*                                                                       */
/*   File:        pcl_printproof.h                                       */
/*                                                                       */
/*   Autor:       Stephan Schulz                                         */
/*                                                                       */
/*   Inhalt:      Deklarationen zur Beweisausgabe                        */
/*                                                                       */
/*   Aenderungen: <1> 19.2.1992 neu                                      */
/*                                                                       */
/*************************************************************************/

#include "pcl_fproof.h"
#include "pcl_latexout.h"
#include "pcl_prologout.h"


#ifndef _pcl_printproof

#define _pcl_printproof



/*-----------------------------------------------------------------------*/
/*                          Typdefinitionen                              */
/*-----------------------------------------------------------------------*/


typedef enum
{
   ascii,
   latex,
   prolog
}OutputType; /* Beschreibt die moeglichen Ausgabeformate */
   



/*----------------------------------------------------------------------------*/
/*                    Deklaration exportierter Funktionen                     */
/*----------------------------------------------------------------------------*/

void PrintProof(Step_p anchor,OutputType output,BOOL prsubst,BOOL prplace);


#endif

/*----------------------------------------------------------------------------*/
/*                         Ende des Files                                     */
/*----------------------------------------------------------------------------*/


