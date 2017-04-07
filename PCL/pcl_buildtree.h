/*-------------------------------------------------------------------------

File        : pcl_buildtree.h 

Autor       : Stephan Schulz

Inhalt      : Deklarationen fuer pcl_buildtree.c  

Aenderungen : <1> 16.10.1991 neu  

-------------------------------------------------------------------------*/


#include "pcl_parse.h"
#include "pcl_analyse.h"


#ifndef _pcl_buildtree

#define _pcl_buildtree




/*-----------------------------------------------------------------------*/
/*                     Typ-Deklarationen                                 */
/*-----------------------------------------------------------------------*/


typedef struct pclidlistcell
{
   NumList_p id;
   Step_p    *arg;   /* Zeigt auf die Position, wo das Argument in der JustCell fehlt. */
   Step_p    source; /* Zeigt auf StepCell, die aus Justifikation entsteht */
   struct pclidlistcell *pred;
   struct pclidlistcell *succ;
}PclIdListCell,*PclIdList_p;



/*-----------------------------------------------------------------------*/
/*                     Exportierte Variable                              */
/*-----------------------------------------------------------------------*/

extern BOOL GatherNodes;

/*-----------------------------------------------------------------------*/
/*                   Forward-Deklarationen                               */
/*-----------------------------------------------------------------------*/


PclIdList_p   AllocPclIdListCell();
void          FreePclIdListCell(PclIdList_p junk);

void          get_parents_real(Just_p just,Step_p step,PclIdList_p ret);

void          insert_identifier(PclIdList_p list,PclIdList_p id);
PclIdList_p   merge_Id_lists(PclIdList_p anchor, PclIdList_p new);


PclIdList_p   GetStepParentsReal(Step_p step);
void          BuildTree(Step_p anchor);

void          TreeOut(Step_p anchor);



#endif


/*-----------------------------------------------------------------------*/
/*                      Ende des Files                                   */
/*-----------------------------------------------------------------------*/


