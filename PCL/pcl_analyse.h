/*-----------------------------------------------------------------------

File  : pcl_analyse.h

Author: Stephan Schulz

Contents
 
  Deklarationen fuer pcl_analyse.c

Changes

<1> 17.7.1991 Uebernahme von pcl_analyse.h
<2> 26.9.1996 Neue Kommentarboxen, Kosmetik

-----------------------------------------------------------------------*/

#ifndef _pcl_analyse

#define _pcl_analyse

#include "pcl_parse.h"



/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/




/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/


extern BOOL ExtIntermed;
extern BOOL ExtLast;

NumListList_p GetStepParents(Step_p step);
void          ExtractProof(Step_p anchor);


#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/



