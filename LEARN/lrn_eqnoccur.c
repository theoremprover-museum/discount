/*-------------------------------------------------------------------------

File        : lrn_eqnoccur.c

Autor       : Stephan Schulz

Inhalt      : Funktionen ZU EqnOccur-Listen

Aenderungen : <1> 10.8.1994 neu (ausgelagert aus lrn_eqn.c)

-------------------------------------------------------------------------*/

#include "lrn_eqnoccur.h"


/*-----------------------------------------------------------------------*/
/*                        Globale Variablen                              */
/*-----------------------------------------------------------------------*/

BOOL PosAndNeg = FALSE;

/*-----------------------------------------------------------------------*/
/*           Forward-Deklaration interner Funktionen                     */
/*-----------------------------------------------------------------------*/

void      print_eqn_occur_list(EqnOccur_p occur, BOOL data);


/*-----------------------------------------------------------------------*/
/*                     Interne Funktionen                                */
/*-----------------------------------------------------------------------*/


/*-------------------------------------------------------------------------

FUNCTION         : void print_eqn_occur_list(EqnOccur_p occur, BOOL data) 

Beschreibung     : Gibt die Occur-Liste aus - Wahlweise mit und ohne
                   Gesamtdatden. 

Globale Variable : out

Seiteneffekte    : Ausgabe

Aenderungen      : <1> 7.8.1994 neu

-------------------------------------------------------------------------*/

void print_eqn_occur_list(EqnOccur_p occur, BOOL data)
{
   if(occur)
   {
      fprintf(out,NAME_FORMAT, occur->example);
      if(data)
      {
	 if(!PosAndNeg)
	 {
	    fprintf(out,"(%ld,%ld) ", occur->references,
		    occur->goal_dist); 
	 }
	 else
	 {
	    fprintf(out,"(%c,%ld,%ld,%ld) ", occur->used ? 'T':'F',
		    occur->cp_cost, occur->references,
		    occur->goal_dist);
	 }
      }      
      for(occur = occur->next; occur; occur = occur->next)
      {
	 fprintf(out,", "NAME_FORMAT, occur->example);
	 if(data)
	 {
	    if(!PosAndNeg)
	    {
	       fprintf(out,"(%ld,%ld) ", occur->references,
		       occur->goal_dist); 
	    }
	    else
	    {
	       fprintf(out,"(%c,%ld,%ld,%ld) ", occur->used? 'T' :'F',
		       occur->cp_cost, occur->references,
		       occur->goal_dist);
	    }
	 }
      }      
   }
}


/*-----------------------------------------------------------------------*/
/*                  Exportierte Funktionen                               */
/*-----------------------------------------------------------------------*/


MakeAlloc(EqnOccurCell);
MakeFree(EqnOccurCell);

/*-------------------------------------------------------------------------

FUNCTION         : void FreeEqnOccurList(EqnOccur_p junk)

Beschreibung     : Gibt die Liste frei.

Globale Variable : -

Seiteneffekte    : Speicheroperationen

Aenderungen      : <1> 7.7.1994 neu

-------------------------------------------------------------------------*/

void FreeEqnOccurList(EqnOccur_p junk)
{
   EqnOccur_p help;

   while(junk)
   {
      help = junk->next;
      FREE(junk->example);
      FreeEqnOccurCell(junk);
      junk = help;
   }
}


/*-------------------------------------------------------------------------

FUNCTION         : EqnOccur_p CopyEqnOccur(EqnOccur_p source)

Beschreibung     : Kopiert eine einzelne EqnOccurCell (ohne
                   Nachfolger). 

Globale Variable : -

Seiteneffekte    : Speicheroperationen

Aenderungen      : <1> 28.8.1994 neu

-------------------------------------------------------------------------*/

EqnOccur_p CopyEqnOccur(EqnOccur_p source)
{
   EqnOccur_p handle = NULL;

   handle = AllocEqnOccurCell();
   handle->example = secure_strdup(source->example);
   handle->references = source->references;
   handle->goal_dist = source->goal_dist;
   handle->next = NULL;

   return handle;
}

/*-------------------------------------------------------------------------

FUNCTION         : EqnOccur_p CopyEqnOccurList(EqnOccur_p source)

Beschreibung     : Kopiert die EqnOccurListe. 

Globale Variable : -

Seiteneffekte    : Speicheroperationen

Aenderungen      : <1> 10.8.1994 neu
                   <2> 28.8.1994 Verwendet CopyEqnOccur()

-------------------------------------------------------------------------*/

EqnOccur_p CopyEqnOccurList(EqnOccur_p source)
{
   EqnOccur_p handle = NULL;

   if(source)
   {
      handle = CopyEqnOccur(source);
      handle->next = CopyEqnOccurList(source->next);
   }
   return handle;
}


/*-------------------------------------------------------------------------

FUNCTION         : long CmpEqnOccurs(EqnOccur_p occ1, EqnOccur_p occ2)

Beschreibung     : Vergleicht zwei EqnOccur-Zellen, Lexicographisch
                   nach example, references, goal_dist. Ergebnis ist
		   >0, falls occ1 groesser ist, <0, wenn occ2 groesser
		   ist, 0 sonst.

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 11.8.1994 neu

-------------------------------------------------------------------------*/

long CmpEqnOccurs(EqnOccur_p occ1, EqnOccur_p occ2)
{
   long res;

   if((res = strcmp(occ1->example, occ2->example)))
   {
      return res;
   }
   else if((res = occ1->references - occ2->references))
   {
      return res;
   }
   return occ1->goal_dist - occ2->goal_dist;
}
      

/*-------------------------------------------------------------------------

FUNCTION         : EqnOccur_p FindEqnOccur(EqnOccur_p occ, EqnOccur_p
                                           list) 

Beschreibung     : Sucht in der EqnOccur-Liste nach einem Eintrag, der
                   zu occ gleichwertig ist. Rueckgabewert ist ein
		   Pointer auf diesen Eintrag oder NULL.

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 11.8.1994 neu

-------------------------------------------------------------------------*/

EqnOccur_p FindEqnOccur(EqnOccur_p occ, EqnOccur_p list)
{
   while(list && (CmpEqnOccurs(occ,list)!=0))
   {
      list = list->next;
   }
   return list;
}


/*-------------------------------------------------------------------------

FUNCTION         : EqnOccur_p FindEqnOccurName(EqnOccur_p occ, EqnOccur_p
                                           list) 

Beschreibung     : Sucht in der EqnOccur-Liste nach einem Eintrag mit
                   dem selben example-namen. Rueckgabewert ist ein
		   Pointer auf diesen Eintrag oder NULL.

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 28.8.1994 neu

-------------------------------------------------------------------------*/

EqnOccur_p FindEqnOccurName(EqnOccur_p occ, EqnOccur_p list)
{
   while(list && (strcmp(occ->example,list->example)!=0))
   {
      list = list->next;
   }
   return list;
}



/*-------------------------------------------------------------------------

FUNCTION         : EqnOccur_p MergeEqnOccurLists(EqnOccur_p list1,
                                                 EqnOccur_p list2,
						 BOOL names_only) 

Beschreibung     : Erzeugt aus zwei EqnOccur-Listen eine
                   zusammengesetzte Liste. Rueckgabewert ist Pointer
		   auf die Gesamtliste. Ist names_only TRUE, so werden
		   Eintraege mit gleichem Namen als identisch
		   betrachtet.

Globale Variable : -

Seiteneffekte    : Die alten Listen werden zerstoert.

Aenderungen      : <1> 11.8.1994 neu

-------------------------------------------------------------------------*/

EqnOccur_p MergeEqnOccurLists(EqnOccur_p list1, EqnOccur_p list2, BOOL
			      names_only )
{
   EqnOccur_p help;
   
   while(list2)
   {
      help = list2->next;
#ifdef NEVER_DEFINED
      /* Eine peinlichere Sache: Es ist voellig legitim, das daselbe
	 Fakt oefter als einmal in einer Liste steht, naehmlich immer
	 dann, wenn zwei verschieden Gleichungen mit den selben
	 Parametern das selbe Pattern haben! Nur im Fall der
	 Beispielnamen ist das ein (moeglicher) Fehler.*/
      if(((!names_only) && FindEqnOccur(list2,list1)) 
	 || (names_only && FindEqnOccurName(list2,list1)))
      {
	 fprintf(stderr,
		 "Warning: Tried to enter known fact into an \
EqnOccur-List (MergeEqnOccurLists())\n");
	 FREE(list2->example);
	 FreeEqnOccurCell(list2);
     }
      else
      {
	 list2->next = list1;
	 list1 = list2;
      }
#endif
      if(names_only && FindEqnOccurName(list2,list1))
      {
	 fprintf(stderr,
		 "Warning: Tried to enter known name into an \
EqnOccur-List containing names only(MergeEqnOccurLists())\n");
	 FREE(list2->example);
	 FreeEqnOccurCell(list2);
     }
      else
      {
	 list2->next = list1;
	 list1 = list2;
      }

      list2 = help;
   }

   return list1;
}


/*-------------------------------------------------------------------------

FUNCTION         : EqnOccur_p EqnOccurNameIntersect(EqnOccur_p list1,
                                                    EqnOccur_p list2) 

Beschreibung     : Stellt eine Liste aller in beiden Listen
                   vorkommenden Beispiele zusammen.

Globale Variable : -

Seiteneffekte    : Speicheroperationenn

Aenderungen      : <1> 28.8.1994 neu

-------------------------------------------------------------------------*/

EqnOccur_p EqnOccurNameIntersect(EqnOccur_p list1, EqnOccur_p list2)
{
   EqnOccur_p handle = NULL, res, help;

   while(list1)
   {
      if((res = FindEqnOccurName(list1,list2)))
      {
	 help = handle;
	 handle = CopyEqnOccur(list1);
	 handle->next = help;
      }
      list1 = list1->next;
   }
   return handle;
}




/*-------------------------------------------------------------------------

FUNCTION         : long AverageGoalDist(EqnOccur_p list)

Beschreibung     : Berechnet den durchschnittlichen Zielabstand in der
                   Liste.

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 25.8.1994 neu

-------------------------------------------------------------------------*/

long AverageGoalDist(EqnOccur_p list)
{
   EqnOccur_p point;
   long       sum=0,count=0;

   for(point = list; point; point = point->next)
   {
      sum += point->goal_dist;
      count++;
   }
   if(count)
   {
      return sum/count;
   }
   return 0;
}

/*-------------------------------------------------------------------------

FUNCTION         : long AverageReferences(EqnOccur_p list)

Beschreibung     : Berechnet die durchschnittliche Anzahl der
                   Referenzen auf einen Schritt.

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 25.8.1994 neu

-------------------------------------------------------------------------*/

long AverageReferences(EqnOccur_p list)
{
   EqnOccur_p point;
   long       sum=0,count=0;

   for(point = list; point; point = point->next)
   {
      sum += point->references;
      count++;
   }
   if(count)
   {
      return sum/count;
   }
   return 0;
}


/*-------------------------------------------------------------------------

FUNCTION         : long TotalReferences(EqnOccur_p list)

Beschreibung     : Berechnet die Gesamtzahl der
                   Referenzen auf einen Schritt.

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 25.8.1994 neu

-------------------------------------------------------------------------*/

long TotalReferences(EqnOccur_p list)
{
   EqnOccur_p point;
   long       sum=0;

   for(point = list; point; point = point->next)
   {
      sum += point->references;
   }
   return sum;
}



/*-------------------------------------------------------------------------

FUNCTION         : void PrintEqnOccurList(EqnOccur_p occur)

Beschreibung     : Gibt die occur-Liste mit allen Informationen aus.

Globale Variable : out

Seiteneffekte    : Ausgabe

Aenderungen      : <1> 31.3.1994 neu

-------------------------------------------------------------------------*/

void PrintEqnOccurList(EqnOccur_p occur)
{
   print_eqn_occur_list(occur, TRUE);
}   

   
/*-------------------------------------------------------------------------

FUNCTION         : void PrintOccurNames(EqnOccur_p occur)

Beschreibung     : Gibt nur die Namen der Beispiele in der Occur-Liste
                   aus. 

Globale Variable : out

Seiteneffekte    : Ausgabe

Aenderungen      : <1> 7.8.1994 neu

-------------------------------------------------------------------------*/

void PrintOccurNames(EqnOccur_p occur)
{
   print_eqn_occur_list(occur, FALSE);
} 



/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/


