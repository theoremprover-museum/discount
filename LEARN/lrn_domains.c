/*-------------------------------------------------------------------------

File        : lrn_domains.c

Autor       : Stephan Schulz

Inhalt      : Funktionen zum Umgang mit Domaenen und Brains. Hier
              werden nun alle wichtigen Funktionen definiert. Dazu
	      gehoert auch ein Teil der Dinge, die bisher in
	      lrn_makedom.c untergebracht waren. Diese Datei wird
	      obsolet.

Aenderungen : <1> 30.8.1994 neu (mit Teilen von lrn_makedom.c)

-------------------------------------------------------------------------*/

#include "lrn_domains.h"


/*-----------------------------------------------------------------------*/
/*           Forward-Deklaration interner Funktionen                     */
/*-----------------------------------------------------------------------*/


char* dom_type_str(DomType type);
char* brain_type_str(BrainType type);


/*-----------------------------------------------------------------------*/
/*                     Interne Funktionen                                */
/*-----------------------------------------------------------------------*/



/*-------------------------------------------------------------------------

FUNCTION         : char* dom_type_str(DomType type)

Beschreibung     : Gibt zu einem Dom"anentype einen Pointer auf einen
                   String zur"uck, der diesen beschreibt. Dieser
                   String darf nur gelesen werden - es wird keine
                   Kopie angefertigt.

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 17.8.1994 neu

-------------------------------------------------------------------------*/

char* dom_type_str(DomType type)
{
   switch(type)
   {
   case SpecDom: 
      return "Specification";
      break;
   case GoalDom: 
      return "Goal";
      break;
   default:
      Error("Unknown DomType (dom_type_str())");
      FAKE_RETURN;
      break;
   }
}


/*-------------------------------------------------------------------------

FUNCTION         : char* brain_type_str(BrainType type)

Beschreibung     : Gibt zu einem Braintype einen Pointer auf einen
                   String zur"uck, der diesen beschreibt. Dieser
                   String darf nur gelesen werden - es wird keine
                   Kopie angefertigt.

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 19.8.1994 neu

-------------------------------------------------------------------------*/

char* brain_type_str(BrainType type)
{
   switch(type)
   {
   case Complete: 
      return "Complete";
      break;
   case Pruned:
      return "Pruned";
      break;
   default:
      Error("Unknown BrainType (brain_type_str())");
      FAKE_RETURN;
      break;
   }
}
                  
/*-----------------------------------------------------------------------*/
/*                  Exportierte Funktionen                               */
/*-----------------------------------------------------------------------*/

MakeAlloc(DomCell);
MakeFree(DomCell);


/*-------------------------------------------------------------------------

FUNCTION         : void FreeDom(Dom_p junk) 

Beschreibung     : Gib Dom"ane zur"uck. Dabei wird FreeEqnList()
                   verwendet, 

Globale Variable : -

Seiteneffekte    : Speicheroperationen

Aenderungen      : <1> 31.7.1994 neu

-------------------------------------------------------------------------*/

void FreeDom(Dom_p junk)
{
   FREE(junk->name);
   FreeEqnOccurList(junk->examples);
   FreeSymbolList(junk->sig); 
   FreeNormEqnList(junk->axioms);
   FreeEqnTree(junk->lemmas);
   FreeEqnTree(junk->facts);
   FreeNormEqnList(junk->theorems);
}


/*-------------------------------------------------------------------------

FUNCTION         : void FreeDomList(Dom_p junk)

Beschreibung     : Gibt eine doppelt verkettet Domain-Liste (bis auf
                   den Anker) zurueck.

Globale Variable : -

Seiteneffekte    : Speicheroperationen

Aenderungen      : <1> 8.9.1994 neu

-------------------------------------------------------------------------*/

void FreeDomList(Dom_p junk)
{
   Dom_p handle,
         help;

   handle = junk->next; 

   while(handle!=junk)
   {
      help = handle->next;
      FreeDom(handle);
      handle = help;
   }
   junk->next = junk->prev = junk;
}


MakeAlloc(BrainCell);
MakeFree(BrainCell);


/*-------------------------------------------------------------------------

FUNCTION         : void FreeBrain(Brain_p junk) 

Beschreibung     : Gib Brain zur"uck. Dabei wird FreeEqnList()
                   verwendet, 

Globale Variable : -

Seiteneffekte    : Speicheroperationen

Aenderungen      : <1> 18.8.1994 neu

-------------------------------------------------------------------------*/

void FreeBrain(Brain_p junk)
{
   FREE(junk->name);
   FreeEqnOccurList(junk->examples);
   FreeEqnTree(junk->facts);
}


/*-------------------------------------------------------------------------

FUNCTION         : NormSubst_p DomEquivSubst(Dom_p dom1, Dom_p dom2)

Beschreibung     : Testet, ob die angegebenen Domaenen aequivalente
                   Spezifikationen bzw. Ziele haben, also Daten zur
		   gleichen Aufgabe enthalten. Falls ja, so wird eine
		   Normsubst zurueckgegeben, die dom1 auf dom2
		   abbildet. Sonst NULL.

Globale Variable : -

Seiteneffekte    : Speicheroperationen, die picked-Felder der sigs
                   koennen sich aendern.

Aenderungen      : <1> 6.9.1994 neu

-------------------------------------------------------------------------*/

NormSubst_p DomEquivSubst(Dom_p dom1, Dom_p dom2)
{
   if((dom1->type == SpecDom) && (dom2->type == SpecDom))
   {
      return EqnListEquiv(dom1->axioms, dom1->sig, dom2->axioms,
			  dom2->sig); 
   }
   else if((dom1->type == GoalDom) && (dom2->type == GoalDom))
   {
      return EqnListEquiv(dom1->theorems, dom1->sig, dom2->theorems,
			  dom2->sig);  
   }
   else 
   {
      return NULL;
   }
}


/*-------------------------------------------------------------------------

FUNCTION         : Dom_p MergeDoms(Dom_p dom1, Dom_p dom2, 
                                   NormSubst_p make_equal) 

Beschreibung     : Erzeugt aus den zwei Domaenen eine gemeinsame
                   Domaene. Die beiden Domaenen sollten kompatibel
		   sein und make_equal sollte die signatur von dom1
		   auf die von dom2 abbilden. Das kann z.B. durch
		   DomEquivSubst() erreicht und getestet werden - es
		   wird NICHT mehr ueberprueft.

Globale Variable : out

Seiteneffekte    : dom1 wird freigegeben, dom2 wird verandert.

Aenderungen      : <1> 6.9.1994 neu

-------------------------------------------------------------------------*/

Dom_p MergeDoms(Dom_p dom1, Dom_p dom2, NormSubst_p make_equal)
{
   EqnOccur_p red_ex;
   FILE       *store_out;

   if(strcmp(dom1->name,dom2->name)==0)
   {
      fprintf(stderr, "Warning: Tried to merge identical domains!\n");
      FreeDom(dom1);
      return dom2;
   }

   if((red_ex = EqnOccurNameIntersect(dom1->examples,
				      dom2->examples)))
   {
      fprintf(stderr, 
	      "Warning: Domains to merge contain identical examples:\n"); 
      store_out = out;
      out = stderr;
      PrintOccurNames(red_ex);
      fprintf(stderr, "\n");
      out = store_out;
      FreeEqnOccurList(red_ex);
   }

   dom2->examples = MergeEqnOccurLists(dom1->examples, dom2->examples,
				       TRUE);
   dom1->examples = NULL;

   PartNormEqnTree(dom1->lemmas,make_equal);
   PartNormEqnTree(dom1->facts,make_equal);
   
   MergeEqnTrees(&(dom2->lemmas), dom1->lemmas);
   MergeEqnTrees(&(dom2->facts), dom1->facts);
   
   dom1->lemmas = dom1->facts = NULL;
   FreeDom(dom1);
   
   return dom2;
}


/*-------------------------------------------------------------------------

FUNCTION         : Brain_p MergeBrains(Brain_p brain1, Brain_p brain2)

Beschreibung     : Vereinigt zwei Brains. Die Brains werden auf jeden
                    Fall vereinigt, wenn sie verschiedenen Namen
		    haben, Probleme sollten vorher abgefangen werden
		    (hier gibt es nur eine Warnung).
                      

Globale Variable : out

Seiteneffekte    : brain1 wird freigegeben, brain2 wird verandert.

Aenderungen      : <1> 9.9.1994 neu

-------------------------------------------------------------------------*/

Brain_p MergeBrains(Brain_p brain1, Brain_p brain2)
{
   EqnOccur_p red_ex;
   FILE       *store_out;

   if(!strcmp(brain1->name,brain2->name))
   {
      fprintf(stderr, "Warning: Tried to merge identical databases!\n");
      FreeBrain(brain1);
      return brain2;
   }

   if((red_ex = EqnOccurNameIntersect(brain1->examples,
				      brain2->examples)))
   {
      fprintf(stderr, 
	      "Warning: Databases to merge contain identical examples:\n"); 
      store_out = out;
      out = stderr;
      PrintOccurNames(red_ex);
      fprintf(stderr, "\n");
      out = store_out;
      FreeEqnOccurList(red_ex);
   }

   if(!BrainsCompatible(brain1,brain2))
   {
      fprintf(stderr, 
	      "Warning: Merging databases with different types!\n"); 
   }
   
   brain2->examples = MergeEqnOccurLists(brain1->examples,
					 brain2->examples, 
					 TRUE);
   brain1->examples = NULL;
   
   MergeEqnTrees(&(brain2->facts), brain1->facts);
   
   brain1->facts = NULL;

   FreeBrain(brain1);
   
   return brain2;
}


/*-------------------------------------------------------------------------

FUNCTION         : BOOL IsUnusedDomName(char* name, Dom_p domlist)

Beschreibung     : Testet, ob name bereits Name einer der Domaenen in
                   domlist ist oder nicht.

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 9.8.1994 neu

-------------------------------------------------------------------------*/

BOOL IsUnusedDomName(char* name, Dom_p domlist)
{
   Dom_p handle;

   for(handle = domlist->next; handle!=domlist; handle = handle->next)
   {
      if(!strcmp(name,handle->name))
      {
	 return FALSE;
      }
   }
   return TRUE;
}
   

/*-------------------------------------------------------------------------

FUNCTION         : BOOL IsUnusedExampleName(char* name, Brain_p brain)

Beschreibung     : Testet, ob name ein verwendter Name eines
                   Besipieles in brain ist.

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 9.8.1994 neu

-------------------------------------------------------------------------*/

BOOL IsUnusedExampleName(char* name, Brain_p brain)
{
   EqnOccur_p handle;

   for(handle = brain->examples; handle; handle = handle->next)
   {
      if(!strcmp(name,handle->example))
      {
	 return FALSE;
      }
   }
   return TRUE;
}


/*-------------------------------------------------------------------------

FUNCTION         : void PrintDomHead(Dom_p prt)

Beschreibung     : Gibt den Kopf einer Domaene (bis zu den Zielen bzw.
                   der Spezifikation (einschliesslich)) aus.

Globale Variable : out

Seiteneffekte    : Ausgabe

Aenderungen      : <1> 6.9.1994 neu

-------------------------------------------------------------------------*/

void PrintDomHead(Dom_p prt)
{
   fprintf(out,"\nDomain: "NAME_FORMAT"\n\n", prt->name);
   fprintf(out,"Type: %s\n\n", dom_type_str(prt->type));
   fprintf(out,"Signature:\n");
   PrintSig(prt->sig);
   fprintf(out,"\nExamples: ");
   PrintOccurNames(prt->examples);
   if(prt->type == SpecDom)
   {
      fprintf(out,"\n\nSpecification:\n\n");
      PrintNormEqnList(prt->axioms,FALSE);
   }
   if(prt->type == GoalDom)
   {
      fprintf(out,"\n\nGoals:\n\n");
      PrintNormEqnList(prt->theorems,FALSE);
   }
}

/*-------------------------------------------------------------------------

FUNCTION         : void PrintDomHeadList(Dom_p prt)

Beschreibung     : Gibt eine Liste von Domainkoepfen aus.

Globale Variable : Nur indirekt

Seiteneffekte    : Ausgabe

Aenderungen      : <1> 8.9.1994 neu

-------------------------------------------------------------------------*/

void PrintDomHeadList(Dom_p prt)
{
   Dom_p handle;

   for(handle = prt->next; handle != prt; handle = handle->next)
   {
      PrintDomHead(handle);
   }
}



/*-------------------------------------------------------------------------

FUNCTION         : void PrintDomBody(Dom_p prt)

Beschreibung     : Gibt den Body einer Domaene aus, beginnend mit den
                   Lemmata. 

Globale Variable : out

Seiteneffekte    : Ausgabe

Aenderungen      : <1> 6.9.1994 neu

-------------------------------------------------------------------------*/

void PrintDomBody(Dom_p prt)
{
   fprintf(out,"\nLemmas:\n\n");
   PrintOptEqnTree(prt->lemmas);

   fprintf(out,"\nFacts:\n\n");
   PrintOptEqnTree(prt->facts);
}


/*-------------------------------------------------------------------------

FUNCTION         : void PrintDom(Dom_p prt)

Beschreibung     : Gibt die Beschreibung einer Domaene aus.

Globale Variable : out

Seiteneffekte    : Ausgabe

Aenderungen      : <1> 8.8.1994 neu

-------------------------------------------------------------------------*/

void PrintDom(Dom_p prt)
{
   PrintDomHead(prt);
   PrintDomBody(prt);
}



/*-------------------------------------------------------------------------

FUNCTION         : void PrintBrain(Brain_p prt)

Beschreibung     : Gibt die Beschreibung eines Brains aus.

Globale Variable : out

Seiteneffekte    : Ausgabe

Aenderungen      : <1> 19.8.1994 neu

-------------------------------------------------------------------------*/

void PrintBrain(Brain_p prt)
{
   fprintf(out,"\nBrain: %s\n\n", prt->name);
   fprintf(out,"Type: %s\n\n", brain_type_str(prt->type));
   fprintf(out,"Examples: ");
   PrintOccurNames(prt->examples);
   fprintf(out,"\n\nFacts:\n\n");
   PrintOptEqnTree(prt->facts);
}



/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/


