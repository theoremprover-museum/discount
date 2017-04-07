/*-------------------------------------------------------------------------

File        : lrn_pcltrans.c  

Autor       : Stephan Schulz

Inhalt      : Funktionen, die PCL-Listings projizieren, umwandeln und
              Informationen extrahieren.

Aenderungen : <1> 28.6.1994 neu

-------------------------------------------------------------------------*/

#include "lrn_pcltrans.h"


/*-----------------------------------------------------------------------*/
/*                       Globale Variable                                */
/*-----------------------------------------------------------------------*/

/* Name des Beispiels fuer die Einordnung in die Dom"ane */

char* examplename = NULL;


/*-----------------------------------------------------------------------*/
/*                  Exportierte Funktionen                               */
/*-----------------------------------------------------------------------*/



/*-------------------------------------------------------------------------

FUNCTION         : void CalcGoalDistAndWeights(Step_p anchor)

Beschreibung     : Traegt im bei anchor haengenden PCL-Listing in
                   jedem Schritt den laengsten Abstand zum
		   entferntesten Ziel (eigentlich Schritt...) ein.
		   Geht davon aus, da"s das Listing mit BuildTree()
		   vorbehandelt wurde. Ausserdem werden alle Terme
		   gewichtet (dies geschieht hier, weil es nur einmal
		   notwendig ist und das PCL-Listing hier sowieso
		   traversiert wird.

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 14.7.1994 neu

-------------------------------------------------------------------------*/

void CalcGoalDistAndWeights(Step_p anchor)
{
   Step_p handle;
   Step_pList_p iterate;
   
   for(handle = anchor->pred; handle !=anchor; handle = handle->pred)
   {
      WeightTerm(handle->pair->lside);
      WeightTerm(handle->pair->rside);
      
      for(iterate = (handle->children)->succ; 
	  iterate != handle->children; iterate = iterate->succ)
      {
	 handle->goal_dist = 0;
	 if((iterate->this)->goal_dist+1 > handle->goal_dist)
	 {
	    handle->goal_dist = (iterate->this)->goal_dist+1;
	 }	 
      }
   }
}


/*-------------------------------------------------------------------------

FUNCTION         : void SetExampleName(char* name)

Beschreibung     : Setzt den Names, unter dem das Beispiel eingeordnet
                   wird. 

Globale Variable : examplename

Seiteneffekte    : Variable wird gesetzt

Aenderungen      : <1> .1994 neu

-------------------------------------------------------------------------*/
	 
void SetExampleName(char* name)
{
   FREE(examplename);
   examplename = secure_strdup(name);
}

/*-------------------------------------------------------------------------

FUNCTION         : NormEqn_p GetStep(Step_p step)

Beschreibung     : Erzeugt aus einem PCL-Step eine Norm-Eqn mit allen
                   Daten. Geht davon aus, das das Listing, aus dem der
		   Schritt stammt, mit CalcGoalDistAndWeight() und
		   BuildTree() vobehandelt ist. 

Globale Variable : examplename

Seiteneffekte    : Speicheroperationen 

Aenderungen      : <1> 14.7.1994 neu

-------------------------------------------------------------------------*/

NormEqn_p GetStep(Step_p step)
{
   NormEqn_p  handle;
   EqnOccur_p occur;
   
   handle = AllocNormEqnCell();
   handle->lside = CopyTerm((step->pair)->lside);
   handle->rside = CopyTerm((step->pair)->rside);
   handle->tot_ref = handle->ave_ref = step->children_no;
   handle->goal_dist = step->goal_dist;

   occur = AllocEqnOccurCell();
   occur->next = NULL;
   occur->example = secure_strdup(examplename);
   occur->references = step->children_no;
   occur->goal_dist = step->goal_dist;

   /* Do not count this twice! */
   occur->cp_cost = step->type == tes_rule ? 0 : step->cp_cost;

   occur->used = step->used;
   
   handle->occur = occur;

   return handle;
}

      
/*-------------------------------------------------------------------------

FUNCTION         : NormEqn_p GetAxioms(Step_p anchor)

Beschreibung     : Erzeugt eine (Norm-Eqn-) Liste der Axiome in der
                   bei anchor verankerten Step-Liste.

Globale Variable : -

Seiteneffekte    : Speicheroperationen

Aenderungen      : <1> 15.7.1994 neu

-------------------------------------------------------------------------*/

NormEqn_p GetAxioms(Step_p anchor)
{
   Step_p    handle;
   NormEqn_p axioms;

   axioms = AllocNormEqnCell();
   axioms->left = axioms->right = axioms;

   for(handle = anchor->succ; handle !=anchor; handle = handle->succ)
   {
      if(handle->just->operation == initial)
      {
	 InsertLastNormEqnList(axioms, GetStep(handle));
      }
   }
   return axioms;
}



/*-------------------------------------------------------------------------

FUNCTION         : NormEqn_p GetGoals(Step_p anchor)

Beschreibung     : Erzeugt eine (Norm-Eqn-) Liste der
                   (urspr"unglichen) Beweisziele in der bei anchor
		   verankerten Step-Liste. 

Globale Variable : -

Seiteneffekte    : Speicheroperationen

Aenderungen      : <1> 15.7.1994 neu

-------------------------------------------------------------------------*/

NormEqn_p GetGoals(Step_p anchor)
{
   Step_p    handle;
   NormEqn_p goals;

   goals = AllocNormEqnCell();
   goals->left = goals->right = goals;

   for(handle = anchor->succ; handle !=anchor; handle = handle->succ)
   {
      if(handle->just->operation == hypothesis)
      {
	 InsertLastNormEqnList(goals, GetStep(handle));
      }
   }
   return goals;
}


/*-------------------------------------------------------------------------

FUNCTION         : NormEqn_p GetLemmas(Step_p anchor)

Beschreibung     : Erzeugt eine (Norm-Eqn-) Liste der Lemmata in der
                   bei anchor verankerten Step-Liste.

Globale Variable : -

Seiteneffekte    : Speicheroperationen

Aenderungen      : <1> 16.8.1994 neu

-------------------------------------------------------------------------*/

NormEqn_p GetLemmas(Step_p anchor)
{
   Step_p    handle;
   NormEqn_p lemmas;

   lemmas = AllocNormEqnCell();
   lemmas->left = lemmas->right = lemmas;

   for(handle = anchor->succ; handle !=anchor; handle = handle->succ)
   {
      if(handle->type == tes_lemma)
      {
	 InsertLastNormEqnList(lemmas, GetStep(handle));
      }
   }
   return lemmas;
}


/*-------------------------------------------------------------------------

FUNCTION         : NormEqn_p GetFacts(Step_p anchor)

Beschreibung     : Erzeugt eine (Norm-Eqn-) Liste der Fakten in der
                   bei anchor verankerten Step-Liste.

Globale Variable : -

Seiteneffekte    : Speicheroperationen

Aenderungen      : <1> 15.7.1994 neu

-------------------------------------------------------------------------*/

NormEqn_p GetFacts(Step_p anchor)
{
   Step_p    handle;
   NormEqn_p facts;

   facts = AllocNormEqnCell();
   facts->left = facts->right = facts;

   for(handle = anchor->succ; handle !=anchor; handle = handle->succ)
   {
      if((handle->type != tes_goal) && 
	 (handle->type != tes_intermedgoal) && 
	 (handle->type != crit_goal) &&
	 (handle->type != crit_intermedgoal))
      {
	 InsertLastNormEqnList(facts, GetStep(handle));
      }
   }
   return facts;
}


/*-------------------------------------------------------------------------

FUNCTION         : Dom_p BuildSpecDom(Step_p anchor, char* example,
                                      char* dom )

Beschreibung     : Erzeuge aus einem PCL-Listing eine Dom"ane, die
                   durch die Spezifikation beschrieben wird.

Globale Variable : -

Seiteneffekte    : Speicheroperationen

Aenderungen      : <1> 31.7.1994 neu

-------------------------------------------------------------------------*/

Dom_p BuildSpecDom(Step_p anchor, char* example, char* dom)
{
   Dom_p handle;
   NormSubst_p subst;
   NormEqn_p tree;

   handle = AllocDomCell();
   
   handle->name = secure_strdup(dom);
   handle->type = SpecDom;
   handle->examples = AllocEqnOccurCell();
   handle->examples->next = NULL;
   handle->examples->example = secure_strdup(example);

   SetExampleName(example);

   handle->axioms = GetAxioms(anchor);
   handle->theorems = NULL;
   handle->lemmas = GetLemmas(anchor);
   handle->facts = GetFacts(anchor);

   subst = NormalizeEqnList(handle->axioms);

   OrientNormEqns(handle->lemmas);
   OrientNormEqns(handle->facts);

   subst = NormEqnList(handle->lemmas,subst);
   subst = NormEqnList(handle->facts,subst);

   FreeNormSubst(subst);

   tree = NULL;
   handle->lemmas = EqnListToTree(&tree, handle->lemmas);
   tree = NULL;
   handle->facts = EqnListToTree(&tree, handle->facts);

   handle->sig = GetSig(handle->axioms,NULL);

   return handle;
}


/*-------------------------------------------------------------------------

FUNCTION         : Dom_p BuildGoalDom(Step_p anchor, char* example,
                                      char* dom) 

Beschreibung     : Baut eine Ziel-Dom"ane aus einem PCL-Listing.

Globale Variable : -

Seiteneffekte    : Speicheroperationen

Aenderungen      : <1> 13.8.1994 neu

-------------------------------------------------------------------------*/

Dom_p BuildGoalDom(Step_p anchor, char* example, char* dom)
{
   Dom_p handle;
   NormSubst_p subst;
   NormEqn_p tree;
   
   handle = AllocDomCell();
   
   handle->name = secure_strdup(dom);
   handle->type = GoalDom;
   handle->examples = AllocEqnOccurCell();
   handle->examples->next = NULL;
   handle->examples->example = secure_strdup(example);

   SetExampleName(example);

   handle->axioms = NULL;
   handle->theorems = GetGoals(anchor);
   handle->lemmas = GetLemmas(anchor);
   handle->facts = GetFacts(anchor);
   
   subst = NormalizeEqnList(handle->theorems);

   OrientNormEqns(handle->lemmas);
   OrientNormEqns(handle->facts);

   PartNormEqnList(handle->lemmas,subst);
   PartNormEqnList(handle->facts,subst);

   handle->sig = GetSig(handle->theorems,NULL);
  
   FreeNormSubst(subst);
   FreeNormEqnList(handle->axioms);
   handle->axioms = NULL;

   tree = NULL;
   handle->lemmas = EqnListToTree(&tree, handle->lemmas);
   tree = NULL;
   handle->facts = EqnListToTree(&tree, handle->facts);

   return handle;
}
   

/*-------------------------------------------------------------------------

FUNCTION         : Brain_p BuildBrain(Step_p anchor, char* example,
                           char* dom, BrainType type)

Beschreibung     : Erzeugt aus einem PCL-Listing eine Brain, d.h. eine
                   Menge von nicht an Dom"anen gebundenen
                   Termpaarmustern (normierten Termpaaren).

Globale Variable : -

Seiteneffekte    : Speicheroperationen

Aenderungen      : <1> 19.8.1994 neu

-------------------------------------------------------------------------*/
 
Brain_p BuildBrain(Step_p anchor, char* example, char* dom, BrainType type)
{
   Brain_p handle;
   NormEqn_p tree;
   
   handle = AllocBrainCell();
   
   handle->name = secure_strdup(dom);
   handle->type = type;
   handle->examples = AllocEqnOccurCell();
   handle->examples->next = NULL;
   handle->examples->example = secure_strdup(example);

   SetExampleName(example);

   if(type == Pruned)
   {
      handle->facts = GetLemmas(anchor);
   }
   else
   {
      handle->facts = GetFacts(anchor);
   }
   OrientNormEqns(handle->facts);
   
   tree = NULL;
   handle->facts = EqnListToTree(&tree, handle->facts);

   return handle;
}

/*-------------------------------------------------------------------------

FUNCTION         : BOOL IsProofProtocol(Step_p anchor)

Beschreibung     : Gibt TRUE zureuck, wenn das Listsing einen Schritt
                   vom Typ tes-goal enthaelt.

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 14.9.1994 neu

-------------------------------------------------------------------------*/

BOOL IsProofProtocol(Step_p anchor)
{
   Step_p handle;

   for(handle = anchor->succ; handle!=anchor; handle = handle->succ)
   {
      if(handle->type == tes_goal)
      {
	 return TRUE;
      }
   }
   return FALSE;
}



/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/


