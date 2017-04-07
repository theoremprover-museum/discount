


/*************************************************************************/
/*                                                                       */
/*   File:        nameadmin.c                                            */
/*                                                                       */
/*   Autor:       Stephan Schulz                                         */
/*                                                                       */
/*   Inhalt:      Funktionen zur Namensverwaltung fuer tc-pcl            */
/*                                                                       */
/*   Aenderungen: <1> 19.2.1991 neu                                      */
/*                                                                       */
/*************************************************************************/

#include "nameadmin.h"



/*----------------------------------------------------------------------------*/
/*                    Funktionen                                              */
/*----------------------------------------------------------------------------*/

MakeAlloc(IdCell);

MakeFree(IdCell);


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void InsertIdentifier(Id_p anchor,Id_p id)              */
/*                    IN    Id_p anchor                                       */
/*                    In    Id_p id                                           */
/*                                                                            */
/* Beschreibung     : Fuegt die Identifier-Zelle, auf die id zeigt, in die    */
/*                    Identifier-Liste ein. Es wird keine Kopie angefertigt!  */ 
/*                    Existiert schon ein Eintrag mit gleicher Nummer, so     */
/*                    wird die alte Zelle freigegeben, die neue eingebunden.  */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Die Liste, auf die anchor zeigt, wird geandert.         */
/*                                                                            */
/* Aenderungen      : <1> 19.2.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

void InsertIdentifier(Id_p anchor,Id_p id)  
{
   Id_p handle;

   handle = anchor->succ;
   while(id->numval < handle->numval)  /* Abbruch, da anchor->numval = -1 !  */
   {
      handle = handle->succ;
   }
   if(id->numval != handle->numval)
   {
      id->succ = handle;
      id->pred = handle->pred;
      (handle->pred)->succ = id;
      handle->pred = id;
   }
   else
   {
      id->succ = handle->succ;
      id->pred = handle->pred;
      (handle->pred)->succ = id;
      (handle->succ)->pred = id;
      FreeIdCell(handle);
   }
}



/******************************************************************************/
/*                                                                            */
/* FUNCTION         : BOOL EraseIdentifier(Id_p anchor,long idnum)            */
/*                    IN    Id_p anchor                                       */
/*                    IN    long idnum                                        */
/*                                                                            */
/* Beschreibung     : Loescht einen Identifer aus der Liste. Der Speicher-    */
/*                    Platz wird freigegeben. Erfolg wird zurueckgegeben      */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Die Liste wird veraendert, FreeIdCell aufgerufen.       */
/*                                                                            */
/* Aenderungen      : <1> 19.2.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

BOOL EraseIdentifier(Id_p anchor,long idnum)  
{
   Id_p handle;

   handle = GetIdentifierAddr(anchor,idnum);
   if(handle)
   {
      (handle->pred)->succ = handle->succ;
      (handle->succ)->pred = handle->pred;
      FreeIdCell(handle);
      return TRUE;
   }
   else
   {
      return FALSE;
   }
}



/******************************************************************************/
/*                                                                            */
/* FUNCTION         : Id_p GetIdentifierAddr(Id_p anchor,long idnum)          */
/*                    IN    Id_p anchor                                       */
/*                    IN    long idnum                                        */
/*                                                                            */
/* Beschreibung     : Gibt Adresse des Identifiers in der Identifier-Liste    */
/*                    zurueck, NULL, falls unbekannt.                         */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : -                                                       */
/*                                                                            */
/* Aenderungen      : <1> 19.2.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

Id_p GetIdentifierAddr(Id_p anchor,long idnum)
{
   Id_p handle;

   handle = anchor->succ;
   while((handle->numval != idnum)&&(handle != anchor))
   {
      handle = handle->succ;
   }
   if(handle->numval != idnum)
   {
     handle = NULL;
   }
   return handle;
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : Id_p RemoveIdentifier(Id_p anchor,long idnum)           */
/*                    IN    Id_p anchor                                       */
/*                    IN    long idnum                                        */
/*                                                                            */
/* Beschreibung     : Gibt Adresse des Identifiers in der Identifier-Liste    */
/*                    zurueck, NULL, falls unbekannt. Der Identifier wird aus */
/*                    der Liste entfernt.                                     */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Liste wird wird veraendert                              */
/*                                                                            */
/* Aenderungen      : <1> 26.2.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

Id_p RemoveIdentifier(Id_p anchor,long idnum)
{
   Id_p handle;

   handle = GetIdentifierAddr(anchor,idnum);
   if(handle)
   {
      (handle->pred)->succ = handle->succ;
      (handle->succ)->pred = handle->pred;
   }
   return handle;
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : BOOL SetPclIdentifier(Id_p anchor,long idnum,           */
/*                                          StepType type,long num)           */
/*                    IN    Id_p anchor                                       */
/*                    IN    long idnum                                        */
/*                    IN    StepType type                                     */
/*                    IN    long num                                          */
/*                                                                            */
/* Beschreibung     : Setzt zu vorhandenen tc-Identifier den neuen pcl-       */
/*                    Identifier. Rueckgabewert ist Erfolg.                   */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Durch Aufruf von GeIdentifierAddr                       */
/*                                                                            */
/* Aenderungen      : <1> 19.2.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

BOOL SetPclIdentifier(Id_p anchor,long idnum,StepType type,long num)
{
   Id_p handle;

   handle = GetIdentifierAddr(anchor,idnum);
   if(handle)
   {
      handle->type = type;
      handle->num = num;
      return TRUE;
   }
   else
   {
      return FALSE;
   }

}




/******************************************************************************/
/*                                                                            */
/* FUNCTION         : BOOL GetPclIdentifier(Id_p anchor,long idnum,           */
/*                                          StepType* type,long* num)         */
/*                    IN    Id_p anchor                                       */
/*                    IN    long idnum                                        */
/*                    OUT   StepType* type                                    */
/*                    OUT   long* num                                         */
/*                                                                            */
/* Beschreibung     : Gibt zu einem bereits in der Liste vorhandenen tc-      */
/*                    Identifier den pcl-Identifier zurueck. Rueckgabewert    */
/*                    ist Erfolg.                                             */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Durch Aufruf von GeIdentifierAddr                       */
/*                                                                            */
/* Aenderungen      : <1> 19.2.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

BOOL GetPclIdentifier(Id_p anchor,long idnum,StepType* type,long* num)
{
   Id_p handle;

   handle = GetIdentifierAddr(anchor,idnum);
   if(handle)
   {
      *type = handle->type;
      *num = handle->num;
      return TRUE;
   }
   else
   {
      return FALSE;
   }
}



/*----------------------------------------------------------------------------*/
/*                         Ende des Files                                     */
/*----------------------------------------------------------------------------*/


