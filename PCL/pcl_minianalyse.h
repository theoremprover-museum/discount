/*-----------------------------------------------------------------------

File  : pcl_minianalyse.h

Author: Stephan Schulz

Contents
 
  Deklarationen fuer pcl_minianalyse.c

Changes

<1> 17.07.1991 Uebernahme von pcl_analyse.h
<2> 26.09.1996 Neue Kommentarboxen, Erweiterungen fuer Extraktion von
               Schritten mit bestimmtem Mindest-Beweisabstand.

-----------------------------------------------------------------------*/

#ifndef _pcl_minianalyse

#define _pcl_minianalyse

#include "pcl_mininorm.h"
#include "pcl_minidoio.h"


/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


extern BOOL ExtIntermed;
extern BOOL ExtLast;

NumListList_p GetStepParents(miniStep_p step);
NumListTree_p BuildAssocTree(miniStep_p anchor);

void CalculateProofDistances(miniStep_p anchor, NumListTree_p
			     predec_assoc); 
void CalculateCPCost(miniStep_p anchor, NumListTree_p predec_assoc);
void MarkCPGenerators(miniStep_p anchor, NumListTree_p predec_assoc);




void          ExtractProofLevels(miniStep_p anchor, 
				 long max_rw_distance, 
				 long max_cp_distance,
				 long cp_cost_min,
				 BOOL rem_trivials); 
void          ExtractProof(miniStep_p anchor, BOOL calc_pd_only);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

