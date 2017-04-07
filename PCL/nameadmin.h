

/*************************************************************************/
/*                                                                       */
/*   File:        nameadmin.h                                            */
/*                                                                       */
/*   Autor:       Stephan Schulz                                         */
/*                                                                       */
/*   Inhalt:      Includefile fuer Namensverwaltung                      */
/*                                                                       */
/*   Aenderungen: <1> 19.2.1991 neu                                      */
/*                                                                       */
/*************************************************************************/

#include "scan.h"

#ifndef _nameadmin

#define __nameadmin

/*----------------------------------------------------------------------------*/
/*                   Typdeklarationen                                         */
/*----------------------------------------------------------------------------*/

typedef struct idcell  /* Hier sind tc-Identifier gemeint !! */
{
   long           numval;

   BOOL           swapped;  /* Seiten vertauscht ?? */ 

   StepType       type;   /* pcl-identifier  */
   long           num;

   struct idcell  *pred,  /* Nachfolger   */
                  *succ;
} IdCell, *Id_p;



/*----------------------------------------------------------------------------*/
/*          Exportierte Funktionen und Variable                               */
/*----------------------------------------------------------------------------*/

Id_p AllocIdCell();
void FreeIdCell(Id_p junk);

void InsertIdentifier(Id_p anchor,Id_p id);     /* tc-identifier werden zerlegt in */
BOOL EraseIdentifier(Id_p anchor,long idnum);   /* Typ und Nummer, Schluessel ist  */
Id_p GetIdentifierAddr(Id_p anchor,long idnum); /* die Nummer. Die verschiedenen   */
Id_p RemoveIdentifier(Id_p anchor,long idnum);
BOOL SetPclIdentifier(Id_p anchor,long idnum,StepType type,long num);   /* Typen   */
BOOL GetPclIdentifier(Id_p anchor,long idnum,StepType* type,long* num); /* werden  */
                                             /* in verschiedenen Listen verwaltet. */

#endif

/*----------------------------------------------------------------------------*/
/*                         Ende des Files                                     */
/*----------------------------------------------------------------------------*/


