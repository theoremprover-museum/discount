/*-------------------------------------------------------------------------

File        : pcl_types.c    

Autor       : Stephan Schulz

Inhalt      : Sowohl von pcl_parse als auch pcl_miniparse benoetigte
              Typen mit Speicherfunktionen

Aenderungen : <1> 15.2.1992 neu 

-------------------------------------------------------------------------*/


#include "pcl_types.h"





/*-----------------------------------------------------------------------*/
/*        Exportierte Funktionen: Speicherfunktionen                     */
/*-----------------------------------------------------------------------*/

MakeAlloc(NumListCell);
MakeAlloc(NumListListCell);
MakeAlloc(PlaceCell);

MakeFree(NumListCell);
MakeFree(NumListListCell);
MakeFree(PlaceCell);

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void FreeNumList(NumList_p junk)                        */
/*                    IN     NumList_p junk                                   */
/*                                                                            */
/* Beschreibung     : Gibt den von einer ZahlenListe beanspruchten Speicher   */
/*                    an die Speicherverwaltung zurueck.                      */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Durch Aufruf von FreeNumListCellCell (siehe dort)       */
/*                                                                            */
/* Aenderungen      : <1> 05.4.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

void FreeNumList(NumList_p junk)
{
   NumList_p handle;

   while(junk)
   {
      handle = junk;
      junk = junk->rest;
      FreeNumListCell(handle);
   }
}

/*-------------------------------------------------------------------------

FUNCTION         : void FreeNumListList(NumListList_p junk)

Beschreibung     : Gibt den von einer NumList-Liste beanspruchten
                   Speicher an die Speicherverwaltung zurueck. Die
		   Zelle, auf die junk zeigt, wird als Anker
		   angesehen, nicht als Teil der Liste. Sie wird aber
		   ebenfalls zurueckgegeben.

Globale Variable : -

Seiteneffekte    : Speicheroperationen    

Aenderungen      : <1> 18.4.1991 neu      

-------------------------------------------------------------------------*/

void FreeNumListList(NumListList_p junk)
{
   NumListList_p help,handle;

   for(handle = junk->succ; handle != junk; handle = help)
   {
      help = handle->succ;
      FreeNumList(handle->this);
      FreeNumListListCell(handle);
   }
   FreeNumListListCell(junk);
}



/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void FreePlace(Place_p junk)                            */
/*                    IN     Place_p junk                                     */
/*                                                                            */
/* Beschreibung     : Gibt den von der Stelle belegten Speicherplatz frei.    */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Durch FreeNumList, FreePlaceCell                        */
/*                                                                            */
/* Aenderungen      : <1> 05.4.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

void FreePlace(Place_p junk)
{
   if(junk)
   {
      FreeNumList(junk->rest);
      FreePlaceCell(junk);
   }
   else
   {
      fprintf(stderr,"Warning: NULL-Pointer returned to FreePlace (pcl_mem.c)...\n");
   }
}


/*-----------------------------------------------------------------------*/
/*        Exportierte Funktionen: Operationen auf NumListen              */
/*-----------------------------------------------------------------------*/


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : NumList_p CopyNumList(NumList_p source)                 */
/*                                                                            */
/* Beschreibung     : Fertigt Kopie der Liste an, auf die source zeigt.       */
/*                    Rueckgabe ist Pointer auf neue Liste.                   */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Allokierung von Speicherplatz                           */
/*                                                                            */
/* Aenderungen      : <1> 23.4.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

NumList_p CopyNumList(NumList_p source)
{
   NumList_p handle,*help;

   help = &handle;

   while(source)
   {
      *help = AllocNumListCell();
      (*help)->value = source->value;
      source = source->rest;
      help = &((*help)->rest);
   }
   *help = NULL;

   return handle;
}

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : long CmpNumList(NumList_p id1, NumList_p id2)           */
/*                    IN     NumList_p id1                                    */
/*                    IN     NumList_p id2                                    */
/*                                                                            */
/* Beschreibung     : Vergleicht 2 Listen von Zahlen, Ergebnis                */
/*                    > 0 : id1 > id2                                         */
/*                    = 0 : id1 = id2                                         */
/*                    < 0 : id1 < id2                                         */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : -                                                       */
/*                                                                            */
/* Aenderungen      : <1> 29.4.1991 neu                                       */
/*                    <2> Umlagerung, Umbenennung                             */
/*                                                                            */
/******************************************************************************/

long CmpNumList(NumList_p id1, NumList_p id2)
{
   long res = 0;
   
   while(id1 && id2 &&(!res))
   {
      res = (id1->value) - (id2->value);
      id1 = id1->rest;
      id2 = id2->rest;
   }
   if(res)
   {
      return res;
   }
   else if(id1)
   {
      return 1;
   }
   else if(id2)
   {
      return -1;
   }
   else
   {
      return 0;
   }
}

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : NumList_p ConcatNumList(NumList_p l1, NumList_p l2)     */
/*                    IN      NumList_p l1                                    */
/*                    IN      NumList_p l2                                    */
/*                                                                            */
/* Beschreibung     : Concateniert zwei Listen von Zahlen                     */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Die Listen werden veraendert                            */
/*                                                                            */
/* Aenderungen      : <1> 6.2.1992 neu                                        */
/*                                                                            */
/******************************************************************************/

NumList_p ConcatNumList(NumList_p l1, NumList_p l2)
{
   NumList_p handle;
   
   if(!l1)
   {
      return l2;
   }
   else
   {
      handle = l1;
      while(handle->rest)
      {
         handle = handle->rest;
      }
      handle->rest = l2;
      return l1;
   }
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void PrintNumList(NumList_p prt)                        */
/*                    IN     NumList_p prt                                    */
/*                                                                            */
/* Beschreibung     : Gibt die NumList, auf die prt zeigt, aus.               */
/*                                                                            */
/* Globale Variable : out                                                     */
/*                                                                            */
/* Seiteneffekte    : Ausgabe                                                 */
/*                                                                            */
/* Aenderungen      : <1> 15.5.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

void PrintNumList(NumList_p prt)
{
   if(prt)
   {
      fprintf(out,"%ld",prt->value);
      for(prt = prt->rest;prt;prt = prt->rest)
      {
         fprintf(out,".%ld",prt->value);
      }
   }
   else
   {
      fprintf(out,"e");
   }
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void PrintIdList(NumList_p prt)                         */
/*                    IN     NumList_p prt                                    */
/*                                                                            */
/* Beschreibung     : Gibt die NumList, auf die prt zeigt, aus. Die erste     */
/*                    Zahl wird rechtsbuendig ausgegeben, dadurch sehen       */
/*                    Listings mit "normalen" Identifiern sauber aus.         */
/*                                                                            */
/* Globale Variable : out                                                     */
/*                                                                            */
/* Seiteneffekte    : Ausgabe                                                 */
/*                                                                            */
/* Aenderungen      : <1> 15.5.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

void PrintIdList(NumList_p prt)
{
   fprintf(out,"%9ld",prt->value);
   for(prt = prt->rest;prt;prt = prt->rest)
   {
      fprintf(out,".%ld",prt->value);
   }
}

/*-----------------------------------------------------------------------*/
/*       Exportierte Funktionen: Operationen auf NumListListen           */
/*-----------------------------------------------------------------------*/


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : NumListList_p CopyNumListList(NumListList_p source)     */
/*                    IN     NumListList_p source                             */
/*                                                                            */
/* Beschreibung     : Kopiert eine Liste von Zahlenlisten                     */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Speicher                                                */
/*                                                                            */
/* Aenderungen      : <1> 13.5.1992 neu                                       */
/*                                                                            */
/******************************************************************************/

NumListList_p CopyNumListList(NumListList_p source)
{
   NumListList_p handle,help,help2;

   handle = source->succ;
   help = AllocNumListListCell();
   help->succ = help;
   help->pred = help;

   while(handle != source)
   {
      help2 = AllocNumListListCell();
      help2->this = CopyNumList(handle->this);
      help2->value = handle->value;
      help2->pred = help->pred;
      help2->succ = help;
      help2->pred->succ = help2;
      help->pred = help2; 
      handle = handle->succ;
   }
   return help;
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : NumListList_p InsertNumList(NumListList_p list,         */
/*                                                NumList_p id)               */
/*                                                                            */
/* Beschreibung     : Fuegt Identifier in (sortierte) Liste ein. Der          */
/*                    darf danach nicht mehr extern referenziert werden!      */
/*                    Rueckgabewert ist der Pointer auf die Zelle, an der id  */
/*                    haengt. Rueckgabewert ist NICHT der Pointer auf die     */
/*                    neue Gesamtliste ! Ist eine neue Zelle angelegt worden, */
/*                    so wird value mit 0 initialisiert.                      */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Liste wird geaendert                                    */
/*                                                                            */
/* Aenderungen      : <1> 23.4.1991 neu                                       */
/*                    <2> 11.5.1991 Umbennenung (fueher insert_identifier)    */
/*                                  Verlagerung in pcl_types.* statt          */
/*                                  pcl_*analyse.*                            */
/*                                                                            */
/******************************************************************************/

NumListList_p InsertNumList(NumListList_p list,NumList_p id)
{
   NumListList_p handle,help; 
   long cmp=1;

   for(help = list->succ; (help != list) && ((cmp=CmpNumList(id,help->this))<0); 
       help = help->succ);

   if((help == list) || cmp) 
   { /* id ist noch nicht in Liste */
      handle = AllocNumListListCell();
      handle->this = id;
      handle->pred = help->pred;
      handle->succ = help;
      (help->pred)->succ = handle;
      help->pred = handle;
      handle->value = 0;
      return handle;
   }
   else
   {  /* Id ist schon vorhanden, ueberfluessige Kopie vernichten */
      FreeNumList(id);
      return help;
   }
}

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : NumListList_p MergeNumListLists(NumListList_p anchor,   */
/*                                                    NumListList_p new)      */
/*                    IN     NumListList_p anchor                             */
/*                    IN     NumListList_p new                                */
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
/*                    <2> 11.5.1991 Umbennenung (fueher merge_Id_lists)       */
/*                                  Verlagerung in pcl_types.* statt          */
/*                                  pcl_*analyse.*                            */
/*                                                                            */
/******************************************************************************/

NumListList_p MergeNumListLists(NumListList_p anchor, NumListList_p new)
{
   NumListList_p handle,help;

   while(new->succ != new)
   {
      handle = new->succ;
      help = InsertNumList(anchor,handle->this);
      /* Sum up values  */
      help->value += handle->value;
      new->succ = handle->succ;   /* Die Liste ist waehrend der Dekomposition */
                                  /* nicht sauber verkettet !                 */
      FreeNumListListCell(handle);
   }
   FreeNumListListCell(new);

   return anchor;
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : NumListList_p RemoveElems(NumListList_p list,           */
/*                                              NumListList_p to_rem)         */
/*                    IN    NumListList_p list                                */
/*                    IN    NumListList_p to_rem                              */
/*                                                                            */
/* Beschreibung     : Entfernt aus list die Elemente, die auch in to_rem      */
/*                    stehen. Beide Listen werden als sortiert vorausgesetzt. */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : list wird veraendert                                    */
/*                                                                            */
/* Aenderungen      : <1> 13.5.1992                                           */
/*                                                                            */
/******************************************************************************/

NumListList_p RemoveElems(NumListList_p list,NumListList_p to_rem)
{
   NumListList_p handle1,handle2,help;
   long cmpres;

   handle1 = list->succ;
   handle2 = to_rem->succ;

   while((handle1!=list) && (handle2!=to_rem))
   {
    /*  printf("RemoveElems: In: ");PrintNumList(handle1->this);
      printf("  Remove: )");PrintNumList(handle2->this);printf("\n"); */
      cmpres = CmpNumList(handle1->this,handle2->this);
      if(cmpres>0)
      {
         handle1 = handle1->succ;
      }
      else if(cmpres<0)
      {
         handle2 = handle2->succ;
      }
      else
      {
         help = handle1;
         handle1 = handle1->succ;
         help->pred->succ = help->succ;
         help->succ->pred = help->pred;
         FreeNumList(help->this);
         FreeNumListListCell(help);
      }
   }
   return list;
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void PrintNumListList(NumListList_p prt)                */
/*                    IN     NumListList_p prt                                */
/*                                                                            */
/* Beschreibung     : Gibt die NumListList, auf die prt zeigt, aus.           */
/*                                                                            */
/* Globale Variable : out                                                     */
/*                                                                            */
/* Seiteneffekte    : Ausgabe                                                 */
/*                                                                            */
/* Aenderungen      : <1> 3.11.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

void PrintNumListList(NumListList_p prt)
{
   NumListList_p handle;
   
   handle = prt->succ;
   
   while(handle!=prt) 
   {
      fprintf(out,"   ");
      PrintNumList(handle->this);
      fprintf(out," has Value: %ld\n",handle->value);
      handle = handle->succ; 
   }
}


/*-----------------------------------------------------------------------*/
/*              Exportierte Funktionen: Sonstiges                        */
/*-----------------------------------------------------------------------*/


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void PrintComment(char* prt)                            */
/*                    IN    char prt                                          */
/*                                                                            */
/* Beschreibung     : Gibt einen Kommentar aus. Dabei werden neue Zeilen      */
/*                    angemessen eingerueckt.                                 */
/*                                                                            */
/* Globale Variable : out                                                     */
/*                                                                            */
/* Seiteneffekte    : Ausgabe                                                 */
/*                                                                            */
/* Aenderungen      : <1> 16.4.1991 neu                                       */ 
/*                                                                            */
/******************************************************************************/

void PrintComment(char* prt)
{
   if(*prt)
   {
      fprintf(out,"            ");
      for(;*prt;prt++)
      {
         if(*prt == '\n')
         {
            fprintf(out,"\n            ");
         }
         else
         {
            putc(*prt,out);
         }
      }
      fprintf(out,"\n");
   }
}



/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/




