
/*************************************************************************/
/*                                                                       */
/*   File:        pcl_analyse.c                                          */
/*                                                                       */
/*   Autor:       Stephan Schulz                                         */
/*                                                                       */
/*   Inhalt:      Prozeduren zur Analyse von PCL-Beweisen                */
/*                                                                       */
/*   Aenderungen: <1> 02.4.1991 neu                                      */
/*                <2> 17.4.1991 Voelliger Umbau, Hauptprogramm ist jetzt */
/*                              extract, Ein- und Ausgabe in pcl_doio    */ 
/*                <3> 19.3.1998 Auch unbenutzte Axiome beim Extrahieren  */
/*                              erhalten (für Beispielauswahl)           */
/*                                                                       */
/*************************************************************************/

#include "pcl_analyse.h"




/*----------------------------------------------------------------------------*/
/*                      Globale Variable                                      */
/*----------------------------------------------------------------------------*/

BOOL ExtIntermed = FALSE;
BOOL ExtLast = FALSE;



/*----------------------------------------------------------------------------*/
/*                 Exportierte Funktionen                                     */
/*----------------------------------------------------------------------------*/


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void get_parents(Just_p just,NumListList_p ret)         */
/*                    IN     Just_p just                                      */
/*                    INOUT  NumListList_p ret                                */
/*                                                                            */
/* Beschreibung     : Fuegt alle Identifier, die im PCL-Ausdruck, auf den     */
/*                    just zeigt, auftauchen, in die Liste ein, auf die ret   */
/*                    zeigt.                                                  */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Allokierung des Speichers fuer die Liste                */
/*                                                                            */
/* Aenderungen      : <1> 30.4.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

void get_parents(Just_p just,NumListList_p ret)
{
   switch(just->operation)
   {
   case initial:
   case hypothesis:
	   break;
   case orientx:
   case orientu:    
	   get_parents(just->arg1.rarg,ret);
	   break;
   case cp:
   case tes_red:
   case instance:
	   get_parents(just->arg1.rarg,ret);
	   get_parents(just->arg2.rarg,ret);
	   break;
   case quotation:  
	   InsertNumList(ret,CopyNumList(just->arg1.Targ.sarg));
	   break;
   default:
	   Error("Illegal Operation in PCL-Justification");
	   break;
   }
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : NumListList_p GetStepParents(Step_p step)               */
/*                    IN    Step_p step                                       */
/*                                                                            */
/* Beschreibung     : Generiert eine Liste der PCL-Schritte, von denen der    */
/*                    mit step referenzierte Schritt abhaengt.                */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Allokierung des notwendigen Speicherplatzes.            */
/*                                                                            */
/* Aenderungen      : <1> 18.4.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

NumListList_p GetStepParents(Step_p step)
{
   NumListList_p handle;

   handle = AllocNumListListCell();
   handle->succ = handle;
   handle->pred = handle;
   handle->this = NULL;

   get_parents(step->just,handle);

   return handle;
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void ExtractProof(Step_p anchor)                        */
/*                    INOUT  Step_p anchor                                    */
/*                                                                            */
/* Beschreibung     : Loescht alle nicht verwendeten Beweisschritte in dem    */
/*                    Beweis, auf den anchor zeigt.                           */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Die Liste mit den Beweisschritten wird veraendert.      */
/*                                                                            */
/* Aenderungen      : <1> 15.6.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

void ExtractProof(Step_p anchor)
{
   Step_p help,next;
   NumListList_p UsedList,handle;
   BOOL InList, ExtStep;

   help = anchor->pred;
   UsedList = AllocNumListListCell();
   UsedList->pred = UsedList;
   UsedList->succ = UsedList;
   UsedList->this = NULL;

   while(help != anchor)
   {
      next = help->pred;
      InList = !CmpNumList(help->id,(UsedList->succ)->this);

      ExtStep= (help->type == tes_final)
               ||(ExtIntermed &&
                       ((help->type == tes_intermed)||
	               (help->type == tes_intermedgoal)||
	               (help->type == crit_intermedgoal)))
               ||(ExtLast)
               ||(help->just->operation==initial); /* Extraxt all axioms */

      ExtLast = FALSE; /* This is a hack...ExtLast implies an          */
		       /* unconditional extraction of the first        */
		       /* (respectively last) step of the PCL-Listing, */
		       /* thereafter it does not influence the         */
		       /* process. Therefore it serves two purposes,   */
		       /* its BOOLEAN Value is something like          */   
		       /* (Is_Last_Step && -l Flag was set)            */

      if(InList||ExtStep)
      {   /* Schritt wird benoetigt */
         MergeNumListLists(UsedList,GetStepParents(help)); /* Notwendige Schritte merken */

         if(InList)
         {   /* Abgearbeiten Schritt aus UsedList loeschen */
            handle = UsedList->succ;
            UsedList->succ = handle->succ;
            (handle->succ)->pred = UsedList;
            FreeNumList(handle->this);
            FreeNumListListCell(handle);
         }
      }
      else
      {
         (help->pred)->succ = help->succ;  /* Schritt vergessen */
         (help->succ)->pred = help->pred;
         FreeStep(help);
      }
      help = next;
   }
}
      

/*----------------------------------------------------------------------------*/
/*                         Ende des Files                                     */
/*----------------------------------------------------------------------------*/


