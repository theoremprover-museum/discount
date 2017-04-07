/*-------------------------------------------------------------------------

File        : lrn_eqn.c

Autor       : Stephan Schulz

Inhalt      : Funktionen zu Gleichungen und
              Vorkommen der Gleichungen in Beweisen - die
	      Grund-Datenstruktur zum Beweislernen.

Aenderungen : <1> 7.7.1994 neu

-------------------------------------------------------------------------*/

#include "lrn_eqn.h"


/*-----------------------------------------------------------------------*/
/*           Forward-Deklaration interner Funktionen                     */
/*-----------------------------------------------------------------------*/

void      split_normeqn_list(NormEqn_p list, NormEqn_p* first,
			     NormEqn_p* second);

NormEqn_p merge_normeqn_lists(NormEqn_p list1, NormEqn_p list2);

BOOL      is_subset(NormEqn_p list1, NormEqn_p pos1, 
		    NormEqn_p list2, NormEqn_p pos2);

/*-----------------------------------------------------------------------*/
/*                     Interne Funktionen                                */
/*-----------------------------------------------------------------------*/


/*-------------------------------------------------------------------------

FUNCTION         : void split_normeqn_list(NormEqn_p list, NormEqn_p*
                                           first, NormEqn_p second)

Beschreibung     : Teile list in zwei gleiche Teile. *list wird
                   freigegeben! 

Globale Variable : -

Seiteneffekte    : Liste wird zerlegt

Aenderungen      : <1> 7.7.1994 neu

-------------------------------------------------------------------------*/

void split_normeqn_list(NormEqn_p list, NormEqn_p* first, NormEqn_p* second) 
{
   NormEqn_p handle;
   BOOL      toggle = FALSE;
     
   *first = AllocNormEqnCell();
   *second = AllocNormEqnCell();

   (*first)->left = (*first)->right = *first;
   (*second)->left = (*second)->right = *second;

   for(handle = RemoveFirstNormEqnList(list); handle; 
       handle = RemoveFirstNormEqnList(list))
   {
      toggle = !toggle;
      if(toggle)
      {
	 InsertFirstNormEqnList(*first, handle);
      }
      else
      {
	 InsertFirstNormEqnList(*second, handle);
      }
   }
   FreeNormEqnCell(list);
}


/*-------------------------------------------------------------------------

FUNCTION         : NormEqn_p merge_normeqn_lists(NormEqn_p list1,
                                                 NormEqn_p list2) 

Beschreibung     : "Mischt" die beiden Eingabelisten. Rueckgabewert
                   ist der Pointer auf die Gesamtliste.

Globale Variable : -

Seiteneffekte    : Die Listen werden zerstoert.

Aenderungen      : <1> 7.7.1994 neu

-------------------------------------------------------------------------*/

NormEqn_p merge_normeqn_lists(NormEqn_p list1, NormEqn_p list2)
{
   NormEqn_p handle1,
             handle2,
             res;

   res = AllocNormEqnCell();
   res->left = res->right = res;

   handle1 = RemoveFirstNormEqnList(list1);
   handle2 = RemoveFirstNormEqnList(list2);

   while(handle1 || handle2)
   {
      if(!handle1)
      {
	 InsertLastNormEqnList(res, handle2);
	 handle2 = RemoveFirstNormEqnList(list2);
      }
      else if(!handle2)
      {
	 InsertLastNormEqnList(res, handle1);
	 handle1 = RemoveFirstNormEqnList(list1);
      }
      else if(CmpNormEqns(handle1,handle2)>=0)
      {  /* handle1 ist groesser! */
	 InsertLastNormEqnList(res, handle2);
	 handle2 = RemoveFirstNormEqnList(list2);
      }
      else
      {
	 InsertLastNormEqnList(res, handle1);
	 handle1 = RemoveFirstNormEqnList(list1);
      }
   }
   FreeNormEqnCell(list1);
   FreeNormEqnCell(list2);

   return res;
}
   

/*-------------------------------------------------------------------------

FUNCTION         : BOOL is_subset(NormEqn_p list1, NormEqn_p pos1, 
                                  NormEqn_p list2, NormEqn_p pos2)

Beschreibung     : Testet, ob list1 ab element pos1 komplett in list2
                   ab pos2 enthalten ist. Beide listen muessen
		   sortiert sein!

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 3.9.1994 neu

-------------------------------------------------------------------------*/

BOOL is_subset(NormEqn_p list1, NormEqn_p pos1, NormEqn_p list2,
	       NormEqn_p pos2) 
{
   long res;

   while(pos1!=list1)
   {
      res = 1;

      while(res > 0)
      {
	 if((pos2 == list2)||((res = CmpNormEqns(pos1,pos2))<0))
	 {
	    return FALSE;
	 }
	 pos2 = pos2->right;
      }
      pos1 = pos1->right;
   }
   return TRUE;
}
      


/*-----------------------------------------------------------------------*/
/*                  Exportierte Funktionen                               */
/*-----------------------------------------------------------------------*/


MakeAlloc(NormEqnCell);
MakeFree(NormEqnCell);


/*-------------------------------------------------------------------------

FUNCTION         : void FreeNormEqn(NormEqn_p junk)

Beschreibung     : Gibt ein NormEqn zurueck.

Globale Variable : -

Seiteneffekte    : Speicheroperationen

Aenderungen      : <1> 7.7.1994 neu

-------------------------------------------------------------------------*/
      
void FreeNormEqn(NormEqn_p junk)
{
   if(junk)
   {
      FreeTerm(junk->lside);
      FreeTerm(junk->rside);
      FreeEqnOccurList(junk->occur);
      FreeNormEqnCell(junk);
   }
}


/*-------------------------------------------------------------------------

FUNCTION         : void FreeNormEqnList(NormEqn_p junk)


Beschreibung     : Gib die Bei junk verankerte Liste zurueck. *junk
                   ist nur der Anker, wird aber auch zurueckgegeben.

Globale Variable : -

Seiteneffekte    : Speicheroperationen

Aenderungen      : <1> 7.8.1994 neu

-------------------------------------------------------------------------*/

void FreeNormEqnList(NormEqn_p junk)
{
   NormEqn_p handle,
             help;
   
   if(junk)
   {
      handle = junk->right;
      while(handle!=junk)
      {
	 help = handle->right;
	 FreeNormEqn(handle);
	 handle = help;
      }
      FreeNormEqnCell(junk);
   }
}


/*-------------------------------------------------------------------------

FUNCTION         : NormEqn_p InsertFirstNormEqnList(NormEqn_p list,
                                                 NormEqn_p cell)  

Beschreibung     : Fuegt cell als erstes Element in list ein.
                   Rueckgabewert ist list.

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 7.7.1994 neu

-------------------------------------------------------------------------*/

NormEqn_p InsertFirstNormEqnList(NormEqn_p list, NormEqn_p cell)
{
   cell->left = list;
   cell->right = list->right;
   (list->right)->left = cell;
   list->right = cell;
   return list;
}


/*-------------------------------------------------------------------------

FUNCTION         : NormEqn_p InsertLastNormEqnList(NormEqn_p list,
                                                     NormEqn_p cell)  

Beschreibung     : Fuegt cell als letztes Element in list ein.
                   Rueckgabewert ist list.

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 7.7.1994 neu

-------------------------------------------------------------------------*/

NormEqn_p InsertLastNormEqnList(NormEqn_p list, NormEqn_p cell)
{
   cell->right = list;
   cell->left = list->left;
   (list->left)->right = cell;
   list->left = cell;
   return list;
}


/*-------------------------------------------------------------------------

FUNCTION         : NormEqn_p RemoveFirstNormEqnList(NormEqn_p list)

Beschreibung     : Entfernt das erste Element aus list und gibt es
                   zurueck (NULL, falls die Liste leer ist).

Globale Variable : -

Seiteneffekte    : Liste wird veraendert

Aenderungen      : <1> 7.7.1994 neu

-------------------------------------------------------------------------*/

NormEqn_p RemoveFirstNormEqnList(NormEqn_p list)
{
   NormEqn_p help = NULL;

   if(list->left!=list)
   {
      help = list->right;
      (help->right)->left = list;
      list->right = help->right;
      help->left = help->right = NULL;
   }
   return help;
}


/*-------------------------------------------------------------------------

FUNCTION         : long EqnListSize(NormEqn_p list)

Beschreibung     : Gibt die Anzahl der Gleichungen in einer EqnList
                   zurueck. 

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 11.9.1994 neu

-------------------------------------------------------------------------*/

long EqnListSize(NormEqn_p list)
{
   long res = 0;
   NormEqn_p handle;
   
   if(!list)
   {
      return 0;
   }

   for(handle = list->right; handle!=list; handle = handle->right)
   {
      res++;
   }
   return res;
}
	 


/*-------------------------------------------------------------------------

FUNCTION         : long CmpNormEqns(NormEqn_p eqn1, NormEqn_p eqn2)

Beschreibung     : Vergleiche lexikographisch zwei normierte
                   Termpaare. 
                   Rueckgabewert ist 0, wenn beide gleich sind, >0,
		   falls eqn1 groesser ist, <0, falls eqn2 groesser
		   ist. 

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 7.7.1994 neu

-------------------------------------------------------------------------*/

long CmpNormEqns(NormEqn_p eqn1, NormEqn_p eqn2)
{
   long res;
   
   if(!(res =  CmpNormTerms(eqn1->lside, eqn2->lside)))
   {
      res = CmpNormTerms(eqn1->rside, eqn2->rside);
   }

   return res;
}


/*-------------------------------------------------------------------------

FUNCTION         : NormEqn_p CopyNormEqn(NormEqn_p eqn)

Beschreibung     : Kopiert die komplette Norm_eqn mit allen Daten
                   (aber ohne die links).

Globale Variable : -

Seiteneffekte    : Speicheroperationen

Aenderungen      : <1> 10.8.1994 neu

-------------------------------------------------------------------------*/

NormEqn_p CopyNormEqn(NormEqn_p eqn)
{
   NormEqn_p handle;

   handle = AllocNormEqnCell();

   handle->lside = CopyTerm(eqn->lside);
   handle->rside = CopyTerm(eqn->rside);
   handle->occur = CopyEqnOccurList(eqn->occur);
   handle->ave_ref = eqn->ave_ref;
   handle->tot_ref = eqn->tot_ref;
   handle->goal_dist = eqn->goal_dist;
   handle->balance = eqn->balance;
   handle->left = handle->right = NULL;
   return handle;
}
   
   

/*-------------------------------------------------------------------------

FUNCTION         : NormEqn_p SortNormEqnListInPlace(NormEqn_p list)

Beschreibung     : Sortiert eine NormEqn-Liste, ohne den Anker zu
                   veraendern (durch einen Trick, ok :-)

Globale Variable : -

Seiteneffekte    : Speicheroperationen

Aenderungen      : <1> 5.9.1994 neu

-------------------------------------------------------------------------*/

NormEqn_p SortNormEqnListInPlace(NormEqn_p list)
{
   NormEqn_p new_anchor;

   if(list->left == list)
   {
      return list;
   }

   new_anchor = AllocNormEqnCell();
   *new_anchor = *list;
   (list->left)->right = new_anchor;
   (list->right)->left = new_anchor;

   new_anchor = SortNormEqnList(new_anchor);

   *list = *new_anchor;
   (list->left)->right = list;
   (list->right)->left = list;

   FreeNormEqnCell(new_anchor);
   
   return list;
}


/*-------------------------------------------------------------------------

FUNCTION         : NormEqn_p SortNormEqnList(NormEqn_p list)

Beschreibung     : Sortiere list aufsteigend nach CmpNormEqns-Ordung

Globale Variable : -

Seiteneffekte    : Liste wird veraendert, Speicheroperationen

Aenderungen      : <1> 7.7.1994 neu

-------------------------------------------------------------------------*/

NormEqn_p SortNormEqnList(NormEqn_p list)
{
   NormEqn_p list1, list2;


   if((list->right->right == list) || (list->right == list))
   {
      return list;
   }

   split_normeqn_list(list, &list1, &list2);
  
   return merge_normeqn_lists(SortNormEqnList(list1),
			      SortNormEqnList(list2)); 
}


/*-------------------------------------------------------------------------

FUNCTION         : BOOL OrientNormEqn(NormEqn_p eqn, BOOL renorm)

Beschreibung     : Orientiert die Gleichung, indem die beiden Seiten
                   lexikographisch als NormTerme verglichen werden. 
		   Ist renorm = TRUE, so werden die Terme vorher und
		   das resultierende Termpaar nachher neu normiert.
		   Rueckgabewert ist TRUE, falls die Terme vertauscht
		   wurden. 

Globale Variable : -

Seiteneffekte    : Alte Normierungen gehen verloren.

Aenderungen      : <1> 13.7.1994 neu

-------------------------------------------------------------------------*/

BOOL OrientNormEqn(NormEqn_p eqn, BOOL renorm)
{
   Term_p help;
   BOOL   res = FALSE;

   if(renorm)
   {
      NormTerm(eqn->lside);
      NormTerm(eqn->rside);
   }

   if(CmpNormTerms(eqn->lside, eqn->rside)>0)
   {
      help = eqn->lside;
      eqn->lside = eqn->rside;
      eqn->rside = help;

      res = TRUE;
   }

   if(renorm)
   {
      FreeNormSubst(NormEqn(eqn,NULL));
   }
   
   return res;
}


/*-------------------------------------------------------------------------

FUNCTION         : void OrientNormEqns(NormEqn_p anchor)

Beschreibung     : Orientiert und normiert alle Termpaare in der bei
                   anchor verankerten Liste einzeln.

Globale Variable : -

Seiteneffekte    : Speicheroperationen

Aenderungen      : <1> 13.7.1994 neu

-------------------------------------------------------------------------*/

void OrientNormEqns(NormEqn_p anchor)
{
   NormEqn_p handle;

   for(handle = anchor->right; handle!=anchor; handle = handle->right)
   {
      OrientNormEqn(handle,TRUE);
   }
}



/*-------------------------------------------------------------------------

FUNCTION         : NormSubst_p NormEqn(NormEqn_p eqn, NormSubst_p subst)

Beschreibung     : Normiert eine Norm-Gleichung.

Globale Variable : -

Seiteneffekte    : NormSubst wird aufgebaut.

Aenderungen      : <1> 13.7.1994 neu

-------------------------------------------------------------------------*/

NormSubst_p NormEqn(NormEqn_p eqn, NormSubst_p subst)
{
   eqn->lside->chain = eqn->rside;
   eqn->rside->chain = NULL;
   
   subst = NormTermList(eqn->lside, subst, TRUE);
   eqn->lside->chain = NULL;
   
   return subst;
}


/*-------------------------------------------------------------------------

FUNCTION         : NormSubst_p NormEqnList(NormEqn_p anchor, 
                                           NormSubst_p subst) 

Beschreibung     : Normiert die Liste von Norm-Eqns, die bei Anchor
                   verankert ist. Es wird EINE NormSubst fuer die
		   gesamte Liste erstellt, allerdings werden die
		   Variablen fuer jedes Termpaar einzeln normiert.

Globale Variable : -

Seiteneffekte    : NormSubst wird aufgebaut

Aenderungen      : <1> 13.7.1994 neu

-------------------------------------------------------------------------*/

NormSubst_p NormEqnList(NormEqn_p anchor, NormSubst_p subst)
{
   NormEqn_p handle;

   for(handle = anchor->right; handle!=anchor; handle = handle->right)
   {
      handle->lside->chain = handle->rside;
      handle->rside->chain = NULL;
      
      subst = FunNormTermList(handle->lside, subst, TRUE);
      FreeNormSubst(VarNormTermList(handle->lside, NULL, TRUE));
      handle->lside->chain = NULL;
   }
   return subst;
}


/*-------------------------------------------------------------------------

FUNCTION         : void PartNormEqnList(NormEqn_p anchor, 
                                        NormSubst_p subst)

Beschreibung     : Normiert die Termpaare in der Liste einzeln, aber
                   unter Ber"ucksichtigung der in subst bereits
		   vorgenommenen (Funktionssymbol-) Bindungen. Im
		   Regelfall sollten die Termpaare schon gerichtet
		   sein. 

Globale Variable : -

Seiteneffekte    : Speicheroperationen, Normung

Aenderungen      : <1> 14.8.1994 neu

-------------------------------------------------------------------------*/

void PartNormEqnList(NormEqn_p anchor, NormSubst_p subst)
{
   NormEqn_p handle;

   for(handle = anchor->right; handle!=anchor; handle = handle->right)
   {
      handle->lside->chain = handle->rside;
      handle->rside->chain = NULL;
      
      FreeNormSubst(FunNormTermList(handle->lside, 
				    CopyNormSubst(subst), TRUE));
      FreeNormSubst(VarNormTermList(handle->lside, NULL, TRUE));
      handle->lside->chain = NULL;
   }
}
  


/*-------------------------------------------------------------------------

FUNCTION         : NormSubst_p NormalizeEqnList(NormEqn_p list)

Beschreibung     : Normalisiert eine Liste von Gleichungen. Die Terme
                   werden einzeln neu normalisiert, die Gleichungen
		   gerichtet, normiert, sortiert und noch einmal
		   (zusammenh"angend) normiert. Rueckgabewert ist die
		   NormSubst, die dabei erzeugt wird.

Globale Variable : -

Seiteneffekte    : Die Liste wird veraendert, Speicheroperationen.

Aenderungen      : <1> 2.9.1994 neu

-------------------------------------------------------------------------*/

NormSubst_p NormalizeEqnList(NormEqn_p list)
{
   NormSubst_p handle;

   OrientNormEqns(list);
   SortNormEqnListInPlace(list);
   handle =  NormEqnList(list,NULL);
   SortNormEqnListInPlace(list);
   
   return handle;
}


/*-------------------------------------------------------------------------

FUNCTION         : CmpResult CmpSortedEqnLists(NormEqn_p list1,
                                               NormEqn_p list2) 

Beschreibung     : Vergleicht zwei sortierte NormEqnLists. Ergebnis
                   ist equal, falls sie gleich sind, subset, falls
		   list1 eine Teilmenge von list2 beschreibt, setsub,
		   wenn list2 Teilmenge von list1 ist und incomparable
		   sonst. 

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 3.9.1994 neu

-------------------------------------------------------------------------*/

CmpResult CmpSortedEqnLists(NormEqn_p list1, NormEqn_p list2)
{
   long      eqncmp;
   NormEqn_p handle1,handle2;

   handle1 = list1->right;
   handle2 = list2->right;
   
   while((handle1!=list1) && (handle2!=list2))
   {
      eqncmp = CmpNormEqns(handle1,handle2);

      if(eqncmp > 0)
      {
	 if(is_subset(list1,handle1,list2,handle2))
	 {
	    return subset;
	 }
	 else
	 {
	    return incomparable;
	 }
      }
      else if(eqncmp < 0)
      {
	 if(is_subset(list2,handle2,list1,handle1))
	 {
	    return setsub;
	 }
	 else
	 {
	    return incomparable;
	 }
      }
      handle1 = handle1->right;
      handle2 = handle2->right;
   }

   if(handle1!=list1)
   {
      return setsub;
   }
   else if(handle2!=list2)
   {
      return subset;
   }
   return equal;
}
   



/*-------------------------------------------------------------------------

FUNCTION         : void PrintNormEqn(NormEqn_p eqn)
 
Beschreibung     : Gibt eine Norm-Eqn aus.

Globale Variable : out

Seiteneffekte    : Ausgabe

Aenderungen      : <1> 13.7.1994 neu

-------------------------------------------------------------------------*/

void PrintNormEqn(NormEqn_p eqn)
{
   PrintNormTerm(eqn->lside);
   fprintf(out, " = ");
   PrintNormTerm(eqn->rside);
}


/*-------------------------------------------------------------------------

FUNCTION         : void PrintNormEqnLine(NormEqn_p eqn)
 
Beschreibung     : Gibt eine Norm-Eqn aus, zusammen mit der
                   Occur-Liste und gefolgt von \n.

Globale Variable : out

Seiteneffekte    : Ausgabe

Aenderungen      : <1> 14.7.1994 neu

-------------------------------------------------------------------------*/

void PrintNormEqnLine(NormEqn_p eqn)
{
   PrintNormEqn(eqn);
   fprintf(out, " : ");
   PrintEqnOccurList(eqn->occur);
   fprintf(out, "\n");
}


/*-------------------------------------------------------------------------

FUNCTION         : void PrintNormEqnList(NormEqn_p anchor, BOOL add)

Beschreibung     : Gib Liste von Norm-Eqn aus. Ist add = true, so
                   werden auch Occurlist etc ausgegeben.

Globale Variable : -

Seiteneffekte    : Ausgabe

Aenderungen      : <1> 15.7.1994 neu

-------------------------------------------------------------------------*/

void PrintNormEqnList(NormEqn_p anchor, BOOL add)
{
   NormEqn_p handle;

   if(add)
   {
      for(handle = anchor->right; handle!=anchor; handle = handle->right) 
      {
	 PrintNormEqnLine(handle);
      }
   }
   else
   {
      for(handle = anchor->right; handle!=anchor; handle = handle->right) 
      {
	 PrintNormEqn(handle);
	 putc('\n',out);
      }
   }
}



/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/


