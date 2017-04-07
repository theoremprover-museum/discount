/*-------------------------------------------------------------------------

File        : lrn_sigs.c

Autor       : Stephan Schulz

Inhalt      : Funktionen zu Signaturen (teilweise aus lrn_eqn.c)

Aenderungen : <1> 2.9.1994 neu
              <2> 3.3.1998 GetNumberOfArity und GetMaxArity hinzugefuegt

-------------------------------------------------------------------------*/

#include "lrn_sigs.h"


/*-----------------------------------------------------------------------*/
/*           Forward-Deklaration interner Funktionen                     */
/*-----------------------------------------------------------------------*/

Symbol_p get_funs_pair(NormEqn_p eqn, Symbol_p sig);

Symbol_p find_next_unpicked(Symbol_p sig, long arity);
Symbol_p find_nth_unpicked(Symbol_p sig, long arity, long n);

NormSubst_p eqnlist_equiv_subset(NormEqn_p list1, Symbol_p sig1,
				 NormEqn_p list2, Symbol_p sig2,
				 NormSubst_p bound);

/*-----------------------------------------------------------------------*/
/*                     Interne Funktionen                                */
/*-----------------------------------------------------------------------*/



/*-------------------------------------------------------------------------

FUNCTION         : Symbol_p get_funs_pair(NormEqn_p eqn, Symbol_p sig)

Beschreibung     : Hole die Signatur aus einem Termpaar.

Globale Variable : -

Seiteneffekte    : Speicheroperationen, Sig wird aufgebaut.

Aenderungen      : <1> 21.7.1994 neu

-------------------------------------------------------------------------*/

Symbol_p get_funs_pair(NormEqn_p eqn, Symbol_p sig)
{
   return GetFuns(eqn->rside,GetFuns(eqn->lside,sig));
}


/*-------------------------------------------------------------------------

FUNCTION         : Symbol_p find_next_unpicked(Symbol_p sig, long arity)

Beschreibung     : Sucht das naechste unmarkierte Funktionssymbol der
                   angegebenen Stelligkeit in sig. Falls keines
		   existiert: NULL

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 2.9.1994 neu

-------------------------------------------------------------------------*/

Symbol_p find_next_unpicked(Symbol_p sig, long arity)
{
   Symbol_p handle = 0;

   while(sig && (!handle))
   {
      if((sig->arity == arity) && !(sig->picked))
      {
	 handle = sig;
      }
      else
      {
	 sig = sig->next;
      }
   }
   return handle;
}



/*-------------------------------------------------------------------------

FUNCTION         : Symbol_p find_nth_unpicked(Symbol_p sig, long
                                              arity, long n)

Beschreibung     : Sucht das n-te (unmarkierte) Funktionssymbol mit
                   der angegebenen Stelligkeit in sig. Falls keines
		   existiert wird NULL zurueckgegeben.

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 2.9.1994 neu

-------------------------------------------------------------------------*/

Symbol_p find_nth_unpicked(Symbol_p sig, long arity, long n)
{
   Symbol_p handle;
   
   if(n<=0)
   {
      Error("Cannot find nth function symbol for n<=0 in\
find_nth_unpicked()...");
   }
      
   for(handle = sig; n; n--)
   {
      if(!(handle = find_next_unpicked(handle,arity)))
      {
	 return NULL;
      }
   }
   return handle;
}




/*-------------------------------------------------------------------------

FUNCTION         : NormSubst_p eqnlist_equiv_subset(NormEqn_p list1,
                               Symbol_p sig1, NormEqn_p list2,
			       Symbol_p sig2, NormSubst_p bound) 

Beschreibung     : Testet, ob eine zu list1 aequivalente Untermenge
                   von list2 existiert. sig_i ist die Signatur zu
		   list_i. Rueckgabewert ist eine NormSubst
		   (2nd-order-subst), die list1 in die gesuchte
		   Teilmenge von list2 ueberfuehrt (NULL, falls das
		   unmoeglich ist). bound enthaelt die bereits
		   aufgebaute Substitution fuer die Rekursion. Beide
		   Gleichungs-Listen muessen normalisiert sein. Die
		   Funktion nutzt sowohl die ids als auch die norm_ids
		   und macht sich diese Dualitaet zunutze (damit
		   vermeide ich die einfuehrung von _noch_ einem
		   Datentyp). 

Globale Variable : -

Seiteneffekte    : Speicheroperationen, Zeit (ein einfacher,
                   abweisender Test ist eventuell in Erwaegung zu
		   ziehen). Die picked-Felder in sig2 werden
		   veraendert. list1 wird umsortiert und bekommt neue
		   norm_ids.

Aenderungen      : <1> 5.8.1994 neu

-------------------------------------------------------------------------*/

NormSubst_p eqnlist_equiv_subset(NormEqn_p list1, Symbol_p sig1,
				 NormEqn_p list2, Symbol_p sig2,
				 NormSubst_p bound)
{
   CmpResult   res;
   Symbol_p    binding;
   NormSubst_p new_elem, rek;
   NormAdmin_p store_admin = NULL;

   if(!sig1)
   {
      NormEqnList(list1, bound);
      SortNormEqnListInPlace(list1);
      res = CmpSortedEqnLists(list1,list2);
      
      if((res == subset) || (res == equal))
      {
	 return bound;
      }
      else
      {
	 return NULL;
      }
   }
   new_elem = AllocNormSubstCell();
   new_elem->id = secure_strdup(sig1->id);
   if(bound)
   {
      new_elem->f_admin = CopyNormAdmin(bound->f_admin);
      store_admin = bound->f_admin;
      bound->f_admin = NULL;
   }
   else
   {
       new_elem->f_admin = CreateNormAdmin();
   }
   new_elem->next = bound;

   binding = sig2;

   while((binding = find_next_unpicked(binding,sig1->arity)))
   {
      binding->picked = TRUE;
      new_elem->norm_id = binding->norm_id;
      new_elem->arity = binding->arity;

      SetNormAdminVal(new_elem->f_admin,
		      binding->arity,
		      max(GetNormAdminVal(new_elem->f_admin,
					  binding->arity),
			  binding->norm_id));
      if((rek = eqnlist_equiv_subset(list1, sig1->next, list2, sig2,
				     new_elem)))
      {
	 if(bound)
	 {
	    FreeNormAdmin(store_admin);
	 }
	 return rek;
      }
      binding->picked = FALSE;
      binding = binding->next;
   }
   if(bound)
   {
      bound->f_admin = store_admin;
   }
   new_elem->next = NULL;
   FreeNormSubst(new_elem);
   
   return NULL;
}



/*-----------------------------------------------------------------------*/
/*                  Exportierte Funktionen                               */
/*-----------------------------------------------------------------------*/



MakeAlloc(SymbolCell);
MakeFree(SymbolCell);

/*-------------------------------------------------------------------------

FUNCTION         : void FreeSymbolList(Symbol_p junk)

Beschreibung     : Gibt eine Symbolliste (= Signatur) zurueck.
                   Die Id's werden NICHT zurueckgegeben.

Globale Variable : -

Seiteneffekte    : 

Aenderungen      : 

-------------------------------------------------------------------------*/

void FreeSymbolList(Symbol_p junk)
{
   Symbol_p help;
   
   while(junk)
   {
      help = junk->next;
      FreeSymbolCell(junk);
      junk = help;
   }
}



/*-------------------------------------------------------------------------

FUNCTION         : Symbol_p FindSymbolCell(char* id, Symbol_p sig)  

Beschreibung     : Suche in einer Symbol (Signatur) nach der Zelle mit
                   dem angegebenen id. Nicht da -> NULL.

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 18.7.1994 neu

-------------------------------------------------------------------------*/

Symbol_p FindSymbolCell(char* id, Symbol_p sig)
{
   Symbol_p handle;

   for(handle = sig; handle && strcmp(id,handle->id); 
       handle = handle->next); 
   
   return handle;
}


/*-------------------------------------------------------------------------

FUNCTION         : long GetArity(char* id, Symbol_p sig) 

Beschreibung     : Hole aus einer Signatur die Stelligkeit eines
                   Funktionssymboles. 

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 18.7.1994 neu

-------------------------------------------------------------------------*/

long GetArity(char* id, Symbol_p sig)
{
   Symbol_p handle;

   if(!(handle = FindSymbolCell(id, sig)))
   {
      Error("Unknown Symbol in GetArity()");
   }
   return handle->arity;
}


/*-------------------------------------------------------------------------

FUNCTION         : Symbol_p GetFuns(Term_p term, Symbol_p sig)

Beschreibung     : Holt Signatur-Informationen "uber alle
                   Funktionssymbole im Term und verkn"upft sie mit
                   denen in sig.

Globale Variable : -

Seiteneffekte    : Speicheroperationen

Aenderungen      : <1> 18.7.1994 neu

-------------------------------------------------------------------------*/

Symbol_p GetFuns(Term_p term, Symbol_p sig)
{
   Symbol_p help;
   Term_p   handle;
   long     arity;

   if(term && !term->isvar)
   {
      arity = GetTermCellArity(term);

      if((help = FindSymbolCell(term->id, sig)))
      {
         if(help->arity != arity)
         {
            Error("Function has multiple arities");
         }
      }
      else
      {
         help = AllocSymbolCell();
         help->id = term->id;
         help->norm_id = term->norm_id;
         help->arity = arity;
         help->next = sig;
         sig = help;
      }
      for(handle = term->args; handle; handle = handle->chain)
      {
         sig = GetFuns(handle,sig);
      }
   }
   return sig;
}


/*-------------------------------------------------------------------------

FUNCTION         : Symbol_p GetSig(NormEqn_p pairlist, Symbol_p sig)

Beschreibung     : Ermittelt aus den vorkommenden Funktionssymbolen
                   eine Signatur, die alle Terme in der Eqn-List
                   beschreibt. 

Globale Variable : -

Seiteneffekte    : Speicheroperationen

Aenderungen      : <1> 21.7.1994 neu

-------------------------------------------------------------------------*/

Symbol_p GetSig(NormEqn_p pairlist, Symbol_p sig)
{
   NormEqn_p handle;

   for(handle = pairlist->right; handle != pairlist;
       handle = handle->right)
   {
      sig = get_funs_pair(handle,sig);
   }
   return sig;
}


/*-------------------------------------------------------------------------

FUNCTION         : void UnpickSig(Symbol_p sig)

Beschreibung     : Setzt alle "picked" bits in der Signatur auf FALSE. 

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 2.9.1994 neu

-------------------------------------------------------------------------*/

void UnpickSig(Symbol_p sig)
{
   Symbol_p point;

   for(point = sig; point; point = point->next)
   {
      point->picked = FALSE;
   }
}


  

/*-------------------------------------------------------------------------

FUNCTION         : void PrintSig(Symbol_p sig)

Beschreibung     : Gibt die Symbol-Liste in der Form fun:arity * aus. 

Globale Variable : out

Seiteneffekte    : Ausgabe

Aenderungen      : <1> 21.7.1994 neu

-------------------------------------------------------------------------*/

void PrintSig(Symbol_p sig)
{
   Symbol_p handle;

   for(handle = sig; handle; handle = handle->next)
   {
      if(PreserveArity)
      {
	 fprintf(out, "%sf%ld_%ld : %ld\n", SIG_IN, handle->arity,
		 handle->norm_id, handle->arity);
      }
      else
      {
	 fprintf(out, "%sf%ld : %ld\n", SIG_IN, handle->norm_id,
		 handle->arity); 
       }
   }
}


/*-------------------------------------------------------------------------

FUNCTION         : BOOL SigIsPart(Symbol_p sig1, Symbol_p sig2)

Beschreibung     : Gibt TRUE zurueck, falls sig1 ein isomorphes
                   Gegenstueck in sig2 hat, d.h., falls jedem
		   Funktionssymbol in sig1 eines gleicher Stelligkeit
		   in sig2 zugeordnet werden kann.

Globale Variable : -

Seiteneffekte    : Die picked-Elemente in sig2 aendern sich.

Aenderungen      : <1> 2.9.1994 neu

-------------------------------------------------------------------------*/

BOOL SigIsPart(Symbol_p sig1, Symbol_p sig2)
{
   Symbol_p help;

   UnpickSig(sig2);

   while(sig1)
   {
      if(!(help = find_next_unpicked(sig2,sig1->arity)))
      {
	 return FALSE;
      }
      help->picked = TRUE;
      sig1 = sig1->next;
   }
   return TRUE;
}


/*-------------------------------------------------------------------------

FUNCTION         : NormSubst_p EqnListSubsum(NormEqn_p list1, Symbol_p
                                             sig1, NormEqn_p list2,
					     Symbol_p sig2)

Beschreibung     : Sucht eine Substitution, die list2 zu einer Liste
                   macht, die nur Elemente aus list1 enthaelt. Die
		   sigs koennten eigentlich berechnet werden, sind in
		   der Anwendung aber schon bekannt und werden nicht
		   ernsthaft veraendert -> Zeit- und Platzersparnis.
		   Die Listen muessen normalisiert sein (und bleiben
		   es...)

Globale Variable : -

Seiteneffekte    : Speicheroperationen, Sortieren der Listen. List2
                   wird gleich geeignet 2nd-order-instanziiert.

Aenderungen      : <1> 5.9.1994 neu

-------------------------------------------------------------------------*/

NormSubst_p EqnListSubsum(NormEqn_p list1, Symbol_p sig1, NormEqn_p
			  list2, Symbol_p sig2)
{
   NormSubst_p res;

   UnpickSig(sig1);
   res = eqnlist_equiv_subset(list2, sig2, list1, sig1,
			      NULL);
   
   return res;
}


/*-------------------------------------------------------------------------

FUNCTION         : NormSubst_p EqnListEquiv(NormEqn_p list1, Symbol_p
                                            sig1, NormEqn_p list2,
					    Symbol_p sig2)

Beschreibung     : Sucht eine Substitution, die list1 zu list 2 macht.
                   Weiteres siehe EqnListSubsum().

Globale Variable : -

Seiteneffekte    : Wie bei EqnListSubsum(). Ausserdem wird list1
                   bei Erfolg entsprechend instanziiert.

Aenderungen      : <1> 5.9.1994 neu

-------------------------------------------------------------------------*/

NormSubst_p EqnListEquiv(NormEqn_p list1, Symbol_p sig1, NormEqn_p
			 list2, Symbol_p sig2) 
{
   NormSubst_p res;
   
   res =  EqnListSubsum(list2, sig2, list1, sig1);
   
   if(res)
   {
      if(CmpSortedEqnLists(list1,list2) == equal)
      {
	 return res;
      }
   }
   FreeNormSubst(res);
   NormalizeEqnList(list1);
   return NULL; 
}
   
/*-------------------------------------------------------------------------

FUNCTION         : long GetNumberOfArity(Symbol_p sig, long arity)

Beschreibung     : Gibt die Anzahl der n-stelligen Funktionssymbole in sig
                   zurück.

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 3.3.1998 neu

-------------------------------------------------------------------------*/

long GetNumberOfArity(Symbol_p sig, long arity)
{
   long count=0;

   while(sig)
   {
      if(sig->arity==arity) count++;
      sig = sig->next;
   }
   return count;
}
/*-------------------------------------------------------------------------

FUNCTION         : long GetMaxArity(Symbol_p sig)

Beschreibung     : Gibt die maximale Stelligkeit der Funktionssymbole in sig
                   zurück.

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 19.3.1998 neu

-------------------------------------------------------------------------*/

long GetMaxArity(Symbol_p sig)
{
   long max=-1;

   while(sig)
   {
      if(sig->arity > max) max = sig->arity;
      sig = sig->next;
   }
   return max;
}

/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/

