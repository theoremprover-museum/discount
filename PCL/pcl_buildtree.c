/*-------------------------------------------------------------------------

File        : pcl_buildtree.c  

Autor       : Stephan Schulz

Inhalt      : Prozeduren zur Analyse von PCL-Beweisen  

Aenderungen : <1> 16.10.1991 Uebernahme von pcl_analyse.c    

-------------------------------------------------------------------------*/


#include "pcl_buildtree.h"



/*-----------------------------------------------------------------------*/
/*                    Globale Variable                                   */
/*-----------------------------------------------------------------------*/

BOOL GatherNodes = TRUE;



/*-----------------------------------------------------------------------*/
/*                Exportierte Funktionen                                 */
/*-----------------------------------------------------------------------*/

MakeAlloc(PclIdListCell);
MakeFree(PclIdListCell);



/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void get_parents_real(Just_p just,Step_p step,          */
/*                                     PclIdList_p ret)                       */
/*                    IN     Just_p just                                      */
/*                    IN     Step_p step                                      */
/*                    INOUT  PclIdList_p ret                                  */
/*                                                                            */
/* Beschreibung     : Fuegt alle Identifier, die im PCL-Ausdruck, auf den     */
/*                    just zeigt, auftauchen, in die Liste ein, auf die ret   */
/*                    zeigt.                                                  */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Allokierung des Speichers fuer die Liste                */
/*                                                                            */
/* Aenderungen      : <1> 30.4.1991 neu                                       */
/*                    <2> 18.10.1991 Umbau fuer BuildTree                     */
/*                                                                            */
/******************************************************************************/

void get_parents_real(Just_p just,Step_p step, PclIdList_p ret)
{
   PclIdList_p handle;

   switch(just->operation)
   {
      case initial:
      case hypothesis:
                       break;
      case orientx:
      case orientu:    
                       get_parents_real(just->arg1.rarg,step,ret);
                       break;
      case cp:
      case tes_red:
      case instance:
                       get_parents_real(just->arg1.rarg,step,ret);
                       get_parents_real(just->arg2.rarg,step,ret);
                       break;
      case quotation:  handle = AllocPclIdListCell();
                       handle->arg = &((just->arg1).Targ.parg);
                       handle->source = step;
                       handle->id = CopyNumList(just->arg1.Targ.sarg);
                       insert_identifier(ret,handle);
                       break;
      default:         Error("Illegal Operation in PCL-Justification");
                       break;
   }
}

                       




/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void insert_identifier(PclIdList_p list,PclIdList_p id) */
/*                                                                            */
/* Beschreibung     : Fuegt Identifier in (sortierte) Liste ein. Der          */
/*                    darf danach nicht mehr extern referenziert werden!      */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Liste wird geaendert                                    */
/*                                                                            */
/* Aenderungen      : <1> 23.4.1991 neu                                       */
/*                    <2> 18.10.1991 Umbau fuer BuildTree                     */
/*                                                                            */
/******************************************************************************/

void insert_identifier(PclIdList_p list,PclIdList_p id)
{
   PclIdList_p help;

   for(help = list->succ; (help != list) && (CmpNumList(id->id,help->id)<0); help = help->succ);

   id->pred = help->pred;
   id->succ = help;
   (help->pred)->succ = id;
   help->pred = id;
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : PclIdList_p merge_Id_lists(PclIdList_p anchor,          */
/*                                                   PclIdList_p new)         */
/*                    IN     PclIdList_p anchor                               */
/*                    IN     PclIdList_p new                                  */
/*                                                                            */
/* Beschreibung     : Fuegt die Elemente der Liste, auf die new zeigt, in die */
/*                    Liste, auf die anchor zeigt, ein. Die Liste, auf die    */
/*                    new zeigt, wird dabei zerlegt, Rueckgabewert ist wieder */
/*                    anchor.                                                 */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Beide Listen werden geaendert                           */
/*                                                                            */
/* Aenderungen      : <1> 30.4.1991 neu                                       */
/*                    <2> 18.10.1991 Umbau fuer BuildTree                     */
/*                                                                            */
/******************************************************************************/

PclIdList_p merge_Id_lists(PclIdList_p anchor, PclIdList_p new)
{
   PclIdList_p handle,help;

   while(new->succ != new)
   {
      handle = new->succ;
      help = handle->succ;
      insert_identifier(anchor,handle);
      new->succ = help;           /* Die Liste ist waehrend der Dekomposition */
                                  /* nicht sauber verkettet !                 */
   }
   FreePclIdListCell(new);

   return anchor;
}
      

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : PclIdList_p GetStepParentsReal(Step_p step)             */
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
/*                    <2> 18.10.1991 Umbau fuer BuildTree                     */
/*                                                                            */
/******************************************************************************/

PclIdList_p GetStepParentsReal(Step_p step)
{
   PclIdList_p handle;

   handle = AllocPclIdListCell();
   handle->succ = handle;
   handle->pred = handle;
   handle->id = NULL;

   get_parents_real(step->just,step,handle);

   return handle;
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void BuildTree(Step_p anchor)                           */
/*                    INOUT  Step_p anchor                                    */
/*                                                                            */
/* Beschreibung     : Loescht alle nicht verwendeten Beweisschritte in dem    */
/*                    Beweis, auf den anchor zeigt, trage Vorwaerts- und      */
/*                    Rueckwaertsverweise ein.                                */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Die Liste mit den Beweisschritten wird veraendert.      */
/*                                                                            */
/* Aenderungen      : <1> 15.6.1991 neu                                       */
/*                    <2> 18.10.1991 Umbau aus ExtractProof                   */
/*                                                                            */
/******************************************************************************/

void BuildTree(Step_p anchor)
{
   Step_p       help,next;
   PclIdList_p  UsedList,handle;
   Step_pList_p child;
   BOOL         InList;

   help = anchor->pred;
   UsedList = AllocPclIdListCell();
   UsedList->pred = UsedList;
   UsedList->succ = UsedList;
   UsedList->id = NULL;

   while(help != anchor) 
   {
      next = help->pred; 
      if((InList=!CmpNumList(help->id,(UsedList->succ)->id)) ||
	 (help->type == tes_final))
      { /* Schritt wird benoetigt */
	 merge_Id_lists(UsedList,GetStepParentsReal(help)); 
	 help->used = TRUE;

	 /* Notwendige Schritte merken */

         if(InList)
         {   /* Alle Abhaengigkeiten eintragen, entsprechende */
             /* Zellen in UsedList loeschen                   */
            while(CmpNumList(help->id,(UsedList->succ)->id)==0)   
            {
               handle = UsedList->succ;
               *(handle->arg) = help;
               child = AllocStep_pListCell();
               child->this = handle->source;
               child->succ = help->children;
               child->pred = help->children->pred;
               (child->succ)->pred = child;
               (child->pred)->succ = child;
               help->children_no++;
               UsedList->succ = handle->succ;
               (handle->succ)->pred = UsedList;
               FreeNumList(handle->id);
               FreePclIdListCell(handle);
            }
         }
      }
      else
      {
	 help->used = FALSE;
	 /* Schritt vergessen - sollte unn"otig sein, da BuildTree bei
  	    den Standard-Programmen immer nach ExtractProof aufgerufen
	    wird - mal sehen, was passiert! StS, 4.6.1997 */
         /* (help->pred)->succ = help->succ;
	    (help->succ)->pred = help->pred;
	    FreeStep(help); */
      }
      help = next;
   }
   if(UsedList->pred != UsedList)
   {
      fprintf(stderr,
	      "Warning: Unresolved references found in BuildTree()\n");
   }
}


/*-------------------------------------------------------------------------

FUNCTION         : void TreeOut(Step_p anchor)

Beschreibung     : Gibt das PCL-Listing in einer Form aus, die tview
                   versteht. 

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 25.4.1994 neu

-------------------------------------------------------------------------*/


void TreeOut(Step_p anchor)
{
   Step_p        handle;
   NumListList_p children,
                 help;

   fprintf(out,"ok(Testing the pipe - it works...)\n");
   fprintf(out,"root(Proof)\n");
   fprintf(out,"set_info(Proof,Proof)\n");
   fprintf(out,"set_contents(Proof, Just the handle of the proof graph)\n\n");
   fprintf(out,"set_shape(Proof,0)\n");
  
   for(handle = anchor->pred; handle!=anchor; handle = handle->pred)
   {
      if(handle->type == tes_final)
      {
	 fprintf(out,"new_edge(Proof,");
	 PrintNumList(handle->id);
	 fprintf(out,")\n\n");
      }

      fprintf(out,"set_info(");
      PrintNumList(handle->id);
      fprintf(out,",");
      PrintIdList(handle->id);
      fprintf(out,")\nset_contents(");
      PrintNumList(handle->id);
      fprintf(out,",");
      PrintStepPure(handle);
      fprintf(out,")\n");
      
      fprintf(out,"set_shape(");
      PrintNumList(handle->id);
      fprintf(out,",");
      if(handle->just->operation == initial)
      {
	 fprintf(out,"1");
      }
      else
      {
	 switch(handle->type)
	 {
	 case tes_lemma:
	    fprintf(out,"2");
	    break;
	 case tes_final:
	    fprintf(out,"3");
	    break;
	 case tes_rule:
	 case tes_eqn:
	    fprintf(out,"4");
	    break;
	 case tes_intermed:
	    fprintf(out,"5");
	    break;
	 case tes_goal:
	    fprintf(out,"6");
	    break;
	 case tes_intermedgoal:
	    fprintf(out,"7");
	    break;
	 case crit_goal:
	    fprintf(out,"8");
	    break;
	 case crit_intermedgoal:
	    fprintf(out,"9");
	    break;
	 default:
	    Error("Unknown StepType in TreeOut");
	    break;
	 }
      }
      fprintf(out,")\n\n");

      children = GetStepParents(handle);
      
      for(help = children->succ;help!=children;help=help->succ)
      {
	 fprintf(out,"new_edge(");
	 PrintNumList(handle->id);
	 fprintf(out,",");
	 PrintNumList(help->this);
	 fprintf(out,")\n\n");
      }
      FreeNumListList(children);
   }
   if(GatherNodes)
   {
      fprintf(out,"call(GatherAll,Proof)\n");
   }
   fprintf(out,"update\n");
   fprintf(out,"ok(Proof tree completed)\n");
}

      

/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/


