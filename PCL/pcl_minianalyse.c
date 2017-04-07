/*-----------------------------------------------------------------------

File  : pcl_minianalyse.c    

Author: Stephan Schulz

Contents
 
  Prozeduren zur Analyse von PCL-Beweise

Changes

<1> 02.04.1991 neu (als pcl_analyse.c!)
<2> 17.04.1991 Voelliger Umbau, Hauptprogramm ist jetzt extract, Ein-
               und Ausgabe in pcl_doio
<3> 17.07.1991 Uernahme fuer mextract, Variante pcl_minianalyse.c
<4> ??.??.1995 Ende der Entwicklung fuer extract, Konzentration auf
               mextract. Die Library-Pakete fuer extract werden
	       weiter gewartet, da sie auch fuer lemma, proof,
	       etc. verwendet werden. 
<5> 30.09.1996 Neue Kommentarboxen, Erweiterungen fuer Extraktion von
               Schritten mit bestimmtem Mindest-Beweisabstand.
<6> 01.12.1996 Aufraeumen
<7> 17.04.1998 Auch unbenutzte Axiome beim Extrahieren erhalten
-----------------------------------------------------------------------*/

#include "pcl_minianalyse.h"


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

BOOL ExtIntermed = FALSE;
BOOL ExtLast = FALSE;


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/




/*-----------------------------------------------------------------------
//
// Function: get_parents()
//
//   Fuegt alle Identifier, die im PCL-Ausdruck, auf den just zeigt,
//   auftauchen, in die Liste ein, auf die ret zeigt.
//
// Global Variables: -
//
// Side Effects    : Allokierung des Speichers fuer die Liste
//
/----------------------------------------------------------------------*/

void get_parents(miniJust_p just,NumListList_p ret)
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
      case quotation:  InsertNumList(ret,CopyNumList(just->arg1.targ));
                       break;
      default:         Error("Illegal Operation in PCL-Justification");
                       break;
   }
}


/*-----------------------------------------------------------------------
//
// Function: calc_rw_distance()
//
//   Berechnet zu einem PCL-Schritt den Abstand (in Rewrite-Schritten)
//   zum Beweis.
//
// Global Variables: -
//
// Side Effects    : Fehler, falls der Abstand in den
//                   Vorgaengerschritten noch nicht gesetzt ist.
//
/----------------------------------------------------------------------*/

long calc_rw_distance(miniJust_p just, NumListTree_p predec_assoc)
{
   long ret = 0;
   NumListTree_p help;

   switch(just->operation)
   {
   case initial:
	 Error("calc_rw_distance called for initial step!\n");
	 break;
   case hypothesis:
	 return 0;
	 break;
   case orientx:
   case orientu:    
	 ret = calc_rw_distance(just->arg1.rarg,predec_assoc);
	 break;
   case cp:
   case tes_red:
	 ret = max(calc_rw_distance(just->arg1.rarg,predec_assoc),
		   calc_rw_distance(just->arg2.rarg,predec_assoc))+1;
	 break;
   case instance:
	 ret = max(calc_rw_distance(just->arg1.rarg,predec_assoc),
		   calc_rw_distance(just->arg2.rarg,predec_assoc));
	 break;
   case quotation:              
	 help = NumListTreeFind(predec_assoc,just->arg1.targ);
	 if(help)
	 {
	    if((ret = help->value->rw_distance) == -1)
	    {
	       Error("Rewrite-proof-distance for predecessor-step not"
		     "calculated"); 
	    }
	 }
	 else
	 {
	    Error("Required parent step not found");
	 }
	 break;
   default:
         Error("Illegal Operation in PCL-Justification");
	 break;
   }
   return ret;
}

/*-----------------------------------------------------------------------
//
// Function: calc_cp_distance()
//
//   Berechnet zu einem PCL-Schritt den Abstand (in CP-Inferenzen)
//   zum Beweis. 
//
// Global Variables: -
//
// Side Effects    : Fehler, falls der Abstand in den
//                   Vorgaengerschritten noch nicht gesetzt ist.
//
/----------------------------------------------------------------------*/

long calc_cp_distance(miniJust_p just, NumListTree_p predec_assoc)
{
   long ret = 0;
   NumListTree_p help;

   switch(just->operation)
   {
   case initial:
	 Error("calc_cp_distance called for initial step!\n");
	 break;
   case hypothesis:
	 return 0;
	 break;
   case orientx:
   case orientu:
	 ret = calc_cp_distance(just->arg1.rarg,predec_assoc);
	 break;
   case cp:
	 ret = max(calc_cp_distance(just->arg1.rarg,predec_assoc),
		   calc_cp_distance(just->arg2.rarg,predec_assoc))+1;
	 break;
   case tes_red:
   case instance:
	 ret = max(calc_cp_distance(just->arg1.rarg,predec_assoc),
		    calc_cp_distance(just->arg2.rarg,predec_assoc));
	 break;
   case quotation:  
	 help = NumListTreeFind(predec_assoc,just->arg1.targ);
	 if(help)
	 {
	    if((ret = help->value->cp_distance) == -1)
	    {
	       Error("CP-proof-distance for predecessor-step not"
		     "calculated"); 
	    }
	 }
	 else
	 {
	    Error("Required parent step not found");
	 }
	 break;
   default:
         Error("Illegal Operation in PCL-Justification");
	 break;
   }
   return ret;
}

/*-------------------------------------------------------------------------
//
// Function: propagate_cp_cost()
//
//   Propagate (and modify) the cost (in direct unnecessary
//   cp-inferences) a given fact causes. For tes-red inferences, only
//   the reduced  parent inherit the cost. For all other
//   inferences, the cost is passed on to the (real) parent.
//
// Global Variables: -
//
// Side Effect     : Modifies the cp_cost variable in the parents of a
//                   PCL step.
//
//-----------------------------------------------------------------------*/

void propagate_cp_cost(miniJust_p just, NumListTree_p predec_assoc,
		       long cp_cost)
{
   NumListTree_p help;

   switch(just->operation)
   {
   case initial:
   case hypothesis:
	 break;
   case orientx:
   case orientu:
	 propagate_cp_cost(just->arg1.rarg,predec_assoc,cp_cost);
	 break;
   case cp:
	 propagate_cp_cost(just->arg1.rarg,predec_assoc,1);
	 propagate_cp_cost(just->arg2.rarg,predec_assoc,1);
	 break;
   case tes_red:
	 propagate_cp_cost(just->arg1.rarg,predec_assoc,cp_cost);
	 break;
   case instance:
	 propagate_cp_cost(just->arg2.rarg,predec_assoc,cp_cost);
	 break;
   case quotation:  
	 help = NumListTreeFind(predec_assoc,just->arg1.targ);
	 if(help)
	 {
/*	    printf("Quote: ");
	    PrintStep(help->value);
	    printf("Old: %ld New: %ld\n", help->value->cp_cost, cp_cost);*/
	    help->value->cp_cost += cp_cost;
	 }
	 else
	 {
	    Error("Required parent step not found");
	 }
	 break;
   default:
         Error("Illegal Operation in PCL-Justification");
	 break;
   }
}

/*-----------------------------------------------------------------------
//
// Function: mark_nontrivial_pairs()
//
//   Markiert alle Schritte, die nicht nur triviale Nachkommen haben
//   bzw. trivial sind. Voraussetzung: Das Listing ist mit
//   ExtractProof() vorbehandelt, predec_assoc ist ein Baum, der alle
//   Identifier mit ihren Schritten assoziiert.
//
// Global Variables: -
//
// Side Effects    : Setzen des Flags <trivial> in den Schritten
//
/----------------------------------------------------------------------*/

void mark_nontrivial_steps(miniStep_p anchor, NumListTree_p predec_assoc)
{
   NumListList_p parents,
                 step;
   NumListTree_p help;
   miniStep_p    handle;
   
   for(handle = anchor->pred; handle != anchor; handle = handle->pred)
   {
      if(handle->trivial == -1)
      {
	 if(IsTrivial(handle->pair) || IsRedundant(handle))
	 {
	    handle->trivial = 1;
	 }
	 else
	 {
	   handle->trivial = 0;
	   parents = GetStepParents(handle);
	   for(step = parents->succ; step!=parents;
	       step = step->succ)
	   {
	      help = NumListTreeFind(predec_assoc,step->this);
	      if(!help)
	      {
		 out = stderr; /* Hack! I will always hand the stream to
				  an output function from now on! I swear!
				  Really! */
		 fprintf(stderr, 
			 "Association tree for identifiers and steps "
			 "incomplete: Cannot find information on step ");
		 PrintNumList(step->this);
		 fprintf(stderr, 
			 "(Function mark_nontrivial_steps() in pcl_minianalyse.c)");
	      }
	      help->value->trivial = 0;
	   }
	   FreeNumListList(parents);
	 }
      }
   }
}



/*-----------------------------------------------------------------------
//
// Function: mark_cp_parents()
//
//   Markiert alle Eltern, die an dieser Justifikation an einer
//   CP-Inferenz irgendwie beteiligt sind.
//
// Global Variables: -
//
// Side Effects    : Fehler, falls predec_assoc nicht komplett.
//
/----------------------------------------------------------------------*/

void mark_cp_parents(miniJust_p just, NumListTree_p predec_assoc,
		      BOOL generated_cps)
{
   NumListTree_p help;

   switch(just->operation)
   {
   case initial:
   case hypothesis:
	 break;
   case orientx:
   case orientu:
	 mark_cp_parents(just->arg1.rarg,predec_assoc,generated_cps);
	 break;
   case cp:
	 mark_cp_parents(just->arg1.rarg,predec_assoc,TRUE);
	 mark_cp_parents(just->arg2.rarg,predec_assoc,TRUE);
	 break;
   case tes_red:
   case instance:
	 mark_cp_parents(just->arg1.rarg,predec_assoc,generated_cps);
	 mark_cp_parents(just->arg2.rarg,predec_assoc,generated_cps);
	 break;
   case quotation:  
	 help = NumListTreeFind(predec_assoc,just->arg1.targ);
	 if(help)
	 {
	    /* printf("Quote: ");
	    PrintStep(help->value); */
	    help->value->generated_cps = help->value->generated_cps
	       || generated_cps;
	 }
	 else
	 {
	    Error("Required parent step not found");
	 }
	 break;
   default:
         Error("Illegal Operation in PCL-Justification");
	 break;
   }
}


/*---------------------------------------------------------------------*/
/*                         Exported Functions                          */
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
//
// Function: GetStepParents()
//
//   Generiert eine Liste der PCL-Schritte, von denen der mit step
//   referenzierte Schritt abhaengt.
//
// Global Variables: -
//
// Side Effects    : Allokierung des notwendigen Speicherplatzes.
//
/----------------------------------------------------------------------*/

NumListList_p GetStepParents(miniStep_p step)
{
   NumListList_p handle;

   handle = AllocNumListListCell();
   handle->succ = handle;
   handle->pred = handle;
   handle->this = NULL;

   get_parents(step->just,handle);

   return handle;
}


/*-----------------------------------------------------------------------
//
// Function: BuildAssocTree()
//
// Berechnet zue einem PCL-Listing einen NumListTree, der als
// Schluessel die Pointer auf PCL-Identifier und als Wert Pointer auf
// entsprechende miniSteps enthaelt. Rueckgabewert ist der Pointer auf
// den Wurzelknoten.  
//
// Global Variables: -
//
// Side Effects    : Speicheroperationen
//
/----------------------------------------------------------------------*/

NumListTree_p BuildAssocTree(miniStep_p anchor)
{
   NumListTree_p assoc_tree = NULL,
                 help;
   miniStep_p handle;

   for(handle = anchor->succ; handle!= anchor; handle = handle->succ)
   {
      help = AllocNumListTreeCell();
      help->key = handle->id;
      help->value = handle;
      NumListTreeInsert(&assoc_tree, help);
   }
   return assoc_tree;
}


/*-----------------------------------------------------------------------
//
// Function: CalculateProofDistances();
//
//   Berechnet die "Abstaende" eines PCL-Schrittes vom
//   Beweis. Schritte, die zum Beweis gehoeren, haben Abstand 0,
//   unbenutzte Axiome haben Abstand 1, bei allen anderen Schritten
//   ergeben sich die Abstaende aus denen der Eltern und der
//   PCL-Justification. 
//
// Global Variables: -
//
// Side Effects    : Traegt die Abstaende ein.
//
/----------------------------------------------------------------------*/

void CalculateProofDistances(miniStep_p anchor, 
                             NumListTree_p predec_assoc) 
{
   miniStep_p    handle;

   VERBOUT("Calculating Proof Distances...\n");
   ExtractProof(anchor,TRUE);

   for(handle = anchor->succ; handle != anchor; handle = handle->succ)
   {
      if(handle->rw_distance == -1)
      {
	 if(handle->just->operation == initial)
	 {
	    handle->rw_distance = 1;
	 }
	 else
	 {
	    handle->rw_distance = calc_rw_distance(handle->just,
						   predec_assoc);
	 }
      }
      if(handle->cp_distance == -1)
      {
	 if(handle->just->operation == initial)
	 {
	    handle->cp_distance = 1;
	 }
	 else
	 {
	    handle->cp_distance = calc_cp_distance(handle->just,
						   predec_assoc);
	 }
      }
   }
}

/*-------------------------------------------------------------------------
//
// Function: CalculateCPCost()
//
//   Calculate the cost (in unneccessary cp-inferences between the
//   successors) for all PCL-steps in the listing. Requires
//   (e.g. by calling ExtractProof(x,TRUE)
//
// Global Variables: -
//
// Side Effect     : Sets the values
//
//-----------------------------------------------------------------------*/

void CalculateCPCost(miniStep_p anchor, NumListTree_p predec_assoc)
{
   miniStep_p handle;

   for(handle = anchor->pred; handle != anchor; handle = handle->pred)
   {
      if(handle->used)
      {
	 /* handle->cp_cost = 0; */
      }
      else
      {
	 propagate_cp_cost(handle->just, predec_assoc,
			   handle->cp_cost);
      }
   }      
}




/*-----------------------------------------------------------------------
//
// Function: MarkCPGenerators()
//
//   Markiert die Schritte, mit denen Kritische Paare gebildet wurden.
//
// Global Variables: -
//
// Side Effects    : -
//
/----------------------------------------------------------------------*/

void MarkCPGenerators(miniStep_p anchor, NumListTree_p predec_assoc)
{
   miniStep_p    handle;

   VERBOUT("Marking CP-generating steps...\n");

   for(handle = anchor->pred; handle != anchor; handle = handle->pred)
   {
      mark_cp_parents(handle->just,predec_assoc,handle->generated_cps);
   }
}


/*-----------------------------------------------------------------------
//
// Function:  ExtractProofLevels()
//
//   Entfernt alle Schritte mit einer rw-distanz > max_rw_distance
//   oder einer cp-distance > max_cp_distance oder mit cp_cost <
//   cp_cost_min (und !used) oder flag trivial (falls gewuenscht) aus
//   dem Beweisprotokoll.  
//
// Global Variables: -
//
// Side Effects    : Die Liste mit den Beweisschritten kann veraendert
//                   werden, die Distanzen werden ueberall
//                   eingetragen.
//
/----------------------------------------------------------------------*/

void ExtractProofLevels(miniStep_p anchor, long max_rw_distance, 
			long max_cp_distance, long cp_cost_min, BOOL
			rem_trivials)
{
   miniStep_p handle, 
              help;
   NumListTree_p predec_assoc;

   predec_assoc = BuildAssocTree(anchor);
   CalculateProofDistances(anchor, predec_assoc);
   CalculateCPCost(anchor, predec_assoc);
   if(rem_trivials)
   {
      mark_nontrivial_steps(anchor, predec_assoc);
   }
   MarkCPGenerators(anchor, predec_assoc);

   FreeNumListTree(predec_assoc);

   VERBOUT("Level-Extraction of steps...\n");
   for(handle = anchor->succ; handle!=anchor; handle = help)
   {
      help = handle->succ;

      if((handle->rw_distance >  max_rw_distance) || 
	 (handle->cp_distance >  max_cp_distance) ||
	 ((handle->cp_cost < cp_cost_min) && !handle->used) ||
	 (handle->trivial && rem_trivials))
      {
	 handle->pred->succ = handle->succ;
	 handle->succ->pred = handle->pred;
	 FreeminiStep(handle);
      }
   }
}


/*-----------------------------------------------------------------------
//
// Function: ExtractProof()
//
//   Originalversion: Loescht alle nicht verwendeten Beweisschritte in
//   dem Beweis, auf den anchor zeigt. Die neue Version nimmt einen
//   boolschen Parameter, falls dieser gesetzt ist, so wird nur die
//   proof-distance der benoetigten Schritte auf 0 gesetzt und kein
//   Schritt geloescht
//
// Global Variables: -
//
// Side Effects    : Die Liste mit den Beweisschritten kann veraendert
//                   werden, benoetigte Schritte werden mit
//                   proof_distance 0 markiert, alle anderem mit -1
//
/----------------------------------------------------------------------*/

void ExtractProof(miniStep_p anchor, BOOL calc_pd_only)
{
   miniStep_p help,next;
   NumListList_p UsedList,handle;
   BOOL InList,ExtStep;

   VERBOUT("Designating or extracting necessary steps\n");
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
               ||(help->just->operation==initial); /* Extract all axioms */

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
	 help->used = TRUE;
	 help->rw_distance = 0;
	 help->cp_distance = 0;
	 help->trivial = 0;

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
	 if(calc_pd_only)
	 {
	    help->used = FALSE;
	    help->rw_distance = -1;
	    help->cp_distance = -1;
	 }
	 else
	 {
	    (help->pred)->succ = help->succ;  /* Schritt vergessen */
	    (help->succ)->pred = help->pred;
	    FreeminiStep(help);
	 }
      }
      help = next;
   }
}
      

/*----------------------------------------------------------------------------*/
/*                         Ende des Files                                     */
/*----------------------------------------------------------------------------*/


