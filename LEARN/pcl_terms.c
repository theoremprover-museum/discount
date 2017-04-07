/*************************************************************************/
/*                                                                       */
/*   File:        pcl_terms.c                                            */
/*                                                                       */
/*   Autor:       Stephan Schulz                                         */
/*                                                                       */
/*   Inhalt:      Elementare Funktionen auf Termen und Termpaaren        */
/*                                                                       */
/*   Aenderungen: <1> 30.10.1991 neu                                     */
/*                <2> 30.3.1998 TermDepth hinzugefuegt                   */
/*                                                                       */
/*************************************************************************/

#include "pcl_terms.h"




/*----------------------------------------------------------------------------*/
/*                      Global Variable                                       */
/*----------------------------------------------------------------------------*/

BOOL NoBrackets = FALSE;



/*----------------------------------------------------------------------------*/
/*                        Exportierte Funktionen                              */
/*----------------------------------------------------------------------------*/

MakeAlloc(TermCell);

MakeFree(TermCell);

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void FreeTerm(Term_p junk)                              */
/*                    IN     Term_p junk                                      */
/*                                                                            */
/* Beschreibung     : Gibt den Term an die Speicherverwaltung zurueck.        */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Durch Aufruf von FreeTermCell (siehe dort)              */
/*                                                                            */
/* Aenderungen      : <1> 13.2.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

void FreeTerm(Term_p junk)
{
   if(junk)
   {
      FreeTermList(junk->args);
      FreeTermCell(junk);
   }
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void FreeArgs(Term_p junk)                              */
/*                    IN     Term_p junk                                      */
/*                                                                            */
/* Beschreibung     : Gibt die Argumente an die Speicherverwaltung zurueck    */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Durch Aufruf von FreeTerm                               */
/*                                                                            */
/* Aenderungen      : <1> 15.2.1991                                           */
/*                                                                            */
/******************************************************************************/

void FreeArgs(Term_p junk)
{
   if(junk)
   {
      FreeTermList(junk->args);
      junk->args = NULL;
   }
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void FreeTermList(Term_p junk);                         */
/*                    IN     Term_p junk                                      */
/*                                                                            */
/* Beschreibung     : Gibt eine Termliste an die Speicherverwaltung zurueck   */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Durch Aufruf von FreeTerm                               */
/*                                                                            */
/* Aenderungen      : <1> 27.1.1992 neu                                       */
/*                                                                            */
/******************************************************************************/

void FreeTermList(Term_p junk)
{
   Term_p help,
          handle;

   handle = junk;
   while(handle)
   {
      help = handle->chain;
      FreeTerm(handle);
      handle = help;
   }
} 



/*-------------------------------------------------------------------------
//
// Function: GetTermCellArity()
//
//  Gibt zu einer Termzelle die Anzahl der Subterme (also die
//  Stelligkeit des Top-Symbols) an. 
//
// Globale Variable : -
//
// Seiteneffekte    : -
//
//-----------------------------------------------------------------------*/

long GetTermCellArity(Term_p term)
{
   long res = 0;
   Term_p handle;

   for(handle = term->args; handle; handle = handle->chain)
   {
      res++;
   }

   return res;
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : long   TermLength(Term_p prt)                           */
/*                    IN      Term_p prt                                      */
/*                                                                            */
/* Beschreibung     : Ermittelt die Laenge der gedruckten Darstellung eines   */
/*                    Terms.                                                  */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : -                                                       */
/*                                                                            */
/* Aenderungen      : <1> 30.1.1992                                           */
/*                                                                            */
/******************************************************************************/

long TermLength(Term_p prt)
{
   long res;
   Term_p handle;
   
   res = strlen(prt->id);
 
   if(!prt->isvar)
   {
      if(prt->args || !NoBrackets)
      {
        res += 2;  /*  fuer die ()  */
      }
      handle = prt->args;
      while(handle)
      {
         res += TermLength(handle);
         handle = handle->chain;
         if(handle)
         {
            res++;  /*  ,  */
         }
      }
  }
  return res;
}

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : long   TermDepth(Term_p prt)                            */
/*                    IN      Term_p prt                                      */
/*                                                                            */
/* Beschreibung     : Ermittelt die max. Tiefe eines Terms                    */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : -                                                       */
/*                                                                            */
/* Aenderungen      : <1> 30.3.1998 new                                       */
/*                                                                            */
/******************************************************************************/

long TermDepth(Term_p prt)
{
   long d, max = -1;
   Term_p handle;
 
   if(prt->isvar) return 0;

   handle = prt->args;
   while(handle)
   {
      d = TermDepth(handle);
      if(d > max) max = d;
      handle = handle->chain;
   }

   return 1 + max;
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : long NumberOfFuncs(Term_p term)                         */
/*                    IN      Term_p term                                     */
/*                                                                            */
/* Beschreibung     : Zaehlt die in einem Term vorkommenden Funktionssymbole  */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : -                                                       */
/*                                                                            */
/* Aenderungen      : <1> 7.7.1992 Uerbernahme aus pcl_lemma.h                */
/*                                                                            */
/******************************************************************************/

long NumberOfFuncs(Term_p term)
{
   long res = 1;
   Term_p handle;

   if(!term->isvar)
   {
      for(handle=term->args;handle;handle=handle->chain)
      {
         res+=NumberOfFuncs(handle);
      }
   }
   return res;
}


MakeAlloc(PairCell);
MakeFree(PairCell);

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void FreePair(Pair_p junk)                              */
/*                    IN     Pair_p junk                                      */
/*                                                                            */
/* Beschreibung     : Gibt das Paar an die Speicherverwaltung zurueck.        */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Durch Aufruf von FreePairCell (siehe dort)              */
/*                                                                            */
/* Aenderungen      : <1> 13.2.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

void FreePair(Pair_p junk)
{
   if(junk)
   {
      FreeTerm(junk->lside);
      FreeTerm(junk->rside);
      FreePairCell(junk);
   }
}

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void PrintTerm(Term_p prt)                              */
/*                    IN     Term_p prt                                       */
/*                                                                            */
/* Beschreibung     : Gibt den Term, auf die prt zeigt, aus. Benutzt die      */
/*                    Variable NoBrackets, um festzustellen, ob Konstanten    */
/*                    mit Klammern ausgegeben werden sollen.                  */
/*                                                                            */
/* Globale Variable : out, NoBrackets                                         */
/*                                                                            */
/* Seiteneffekte    : Ausgabe                                                 */
/*                                                                            */
/* Aenderungen      : <1> 15.5.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

void PrintTerm(Term_p prt)
{
   if(prt)
   {
      fprintf(out,"%s",prt->id);
      if(!(prt->isvar))
      {
         if(!NoBrackets || prt->args)
         {
            fprintf(out,"(");
            PrintArgList(prt->args);
            fprintf(out,")");
         }
      }
   }
}

 
/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void PrintArgList(Term_p prt)                           */
/*                    IN     Arg_p prt                                        */
/*                                                                            */
/* Beschreibung     : Gibt die Argumentliste, auf die prt zeigt, aus.         */
/*                                                                            */
/* Globale Variable : out                                                     */
/*                                                                            */
/* Seiteneffekte    : Ausgabe                                                 */
/*                                                                            */
/* Aenderungen      : <1> 15.5.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

void PrintArgList(Term_p prt)
{
   if(prt)
   {
      PrintTerm(prt);
      for(prt = prt->chain; prt; prt = prt->chain)
      {
         fprintf(out,",");
         PrintTerm(prt);
      }
   }
}

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void PrintTermPair(Pair_p prt)                          */
/*                    IN     Pair_p prt                                       */
/*                                                                            */
/* Beschreibung     : Gibt das Termpaar, auf das prt zeigt, aus.              */
/*                                                                            */
/* Globale Variable : out                                                     */
/*                                                                            */
/* Seiteneffekte    : Ausgabe                                                 */
/*                                                                            */
/* Aenderungen      : <1> 15.5.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

void PrintTermPair(Pair_p prt)
{
   PrintTerm(prt->lside);
   if(prt->type == eqn)
   {
      fprintf(out," = ");
   }
   else
   {
      fprintf(out," -> ");
   }
   PrintTerm(prt->rside);
}



/******************************************************************************/
/*                                                                            */
/* FUNCTION         : Term_p CopyTerm(Term_p term)                            */
/*                    IN     Term_p term                                      */
/*                                                                            */
/* Beschreibung     : Gibt einen Pointer auf eine neu angelegt Kopie des      */
/*                    Termes zurueck                                          */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Ueber Alloc-Funktionen                                  */
/*                                                                            */
/* Aenderungen      : <1> 5.12.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

Term_p CopyTerm(Term_p term)
{
   Term_p handle;
   Term_p help1, *help2;

   handle = NULL;

   if(term)
   {
      handle = AllocTermCell();
      handle->id = secure_strdup(term->id);
      handle->isvar = term->isvar;
#ifdef LEARN_VERSION
      handle->norm_id = term->norm_id;
      handle->term_weight = term->term_weight;
#endif      
      handle->chain = NULL;
      help1 = term->args;
      help2 = &(handle->args);
      while(help1)
      {
         *help2 = CopyTerm(help1);
         help2 = &((*help2)->chain);
         help1 = help1->chain;
      }
      *help2 = NULL;
   }
   return handle;
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : Term_p Subterm(Term_p term, NumList_p place)            */
/*                    IN     Term_p term                                      */
/*                    IN     NumList_p place                                  */
/*                                                                            */
/* Beschreibung     : Gibt eine Pointer auf den Beginn des durch place be-    */
/*                    schriebenen Subtermes von term zurueck. Es wird keine   */
/*                    Kopie des Subtermes angelegt !!                         */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : -                                                       */
/*                                                                            */
/* Aenderungen      : <1> 9.12.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

Term_p Subterm(Term_p term, NumList_p place)
{
   long   help;

   if(place)
   {
      if(!term)
      {
         Error("Empty term in Subterm (pcl_terms.c)");
      }
      term = term->args;
      if(place->value <= 0)
      {
         Error("Illegal place in Subterm (pcl_terms.c)");
	 FAKE_RETURN;
      }
      else
         {
         for(help=1; help < place->value; help++)
         {
            if(!term)
            {
               Error("Illegal place in Subterm (pcl_terms.c)");
            }
            else
            {
               term = term->chain;
            }
         }
         return Subterm(term ,place->rest);
      }
   }
   else
   {
      return term;
   }
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : Term_p Replace(Term_p term, NumList_p place,            */
/*                                   Term_p repterm)                          */
/*                    IN    Term_p    term                                    */
/*                    IN    NumList_p place                                   */
/*                    IN    Term_p    repterm                                 */
/*                                                                            */
/* Beschreibung     : Ersetzt den durch place beschriebenen Subterm in term   */
/*                    durch repterm. Der ersetzte Subterm wird freigegeben,   */
/*                    der Originalterm veraendert und zurueckgegeben.         */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Duch Speicherverwaltung                                 */
/*                                                                            */
/* Aenderungen      : <1> 8.2.1991 neu                                        */
/*                                                                            */
/******************************************************************************/

Term_p Replace(Term_p term, NumList_p place, Term_p repterm)
{
   Term_p handle;
   long   help;

   handle = term;

   if(!handle)
   {
      Error("Empty term in Replace  (pcl_terms.c)");
   }

   while(place)
   {
      handle = handle->args;

      if(place->value<=0)
      {
         Error("Tried to access argument at position <=0 in Replace (pcl_terms.c)");
      }
      else if(!handle)
      {
         Error("Term is of insuficient depth in Replace (pcl_terms.c)");
      }
      else
      {
         for(help=1; help < place->value; help++)
         {
            if(!handle->chain)
            {
               Error("Illegal place in Replace  (pcl_terms.c)");
            }
            else
            {
               handle = handle->chain;
            }
         }
      }
      place = place->rest;
   }
   
   ReplaceThis(handle,repterm);

   return term;
}
         

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : Term_p ReplaceThis(Term_p term, Term_p repterm)         */
/*                                                                            */
/* Beschreibung     : Ersetzt term durch repterm                              */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Duch Speicherverwaltung                                 */
/*                                                                            */
/* Aenderungen      : <1> 8.2.1991 neu                                        */
/*                                                                            */
/******************************************************************************/

Term_p ReplaceThis(Term_p term, Term_p repterm)
{
   Term_p handle,
          help1,
          help2;
   
   /* term zeigt auf den zu ersetzenden Teilterm. Jetzt werden dessen    */
   /* Argumente freigegeben und der Inhalt der obersten Termzelle von    */
   /* repterm auf die verbleibende Zelle kopiert. Dann wird die alte     */
   /* Wurzel von repterm freigegeben.                                    */
   handle = term->args;
   help2 = term->chain;
   while(handle)
   {
      help1 = handle->chain;
      FreeTerm(handle);
      handle = help1;
   }
   *term = *repterm;
   term->chain = help2;
   FreeTermCell(repterm);
   
   return term;
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : BOOL TermEqual(Term_p term1,Term_p term2)               */
/*                    IN     Term_p term1                                     */
/*                    IN     Term_p term2                                     */
/*                                                                            */
/* Beschreibung     : Gibt einen Pointer auf eine neu angelegt Kopie des      */
/*                    Termes zurueck                                          */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : -                                                       */
/*                                                                            */
/* Aenderungen      : <1> 5.12.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

BOOL TermEqual(Term_p term1,Term_p term2)
{
   Term_p help1, help2;
   BOOL   res = TRUE;

   DEBUG(DTEQ,fprintf(out,"TermEqual: ");PrintTerm(term1);fprintf(out,"\n           ");PrintTerm(term2);fprintf(out,"\n"));


   if(!term1 || !term2)
   {
      return term1 == term2;
   }
   if(term1->isvar)
   {
      return term2->isvar && !strcmp(term1->id,term2->id);
   } 
   if(strcmp(term1->id,term2->id))
   {
      return FALSE;
   }
   help1 = term1->args;
   help2 = term2->args;
   while(help1 && help2 &&res)
   {
      res = res && TermEqual(help1,help2);
      help1 = help1->chain;
      help2 = help2->chain; 
   }
   return res && (help1 == help2);
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : Term_p ConcatTermLists(Term_p tl1, Term_p tl2)          */
/*                    IN      Term_p tl1                                      */
/*                    IN      Term_p tl2                                      */
/*                                                                            */
/* Beschreibung     : Haengt destruktiv die beiden Listen aneinander, Ergeb-  */
/*                    nis ist Pointer auf die Gesamtliste.                    */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Veraenderung der Listen                                 */
/*                                                                            */
/* Aenderungen      : <1> 27.1.1992 neu                                       */
/*                                                                            */
/******************************************************************************/

Term_p ConcatTermLists(Term_p tl1, Term_p tl2)
{
   Term_p handle;
   
   if(!tl1)
   {
      return tl2;
   }
   else
   {
      handle = tl1;
      while(handle->chain)
      {
         handle = handle->chain;
      }
      handle->chain = tl2;
      return tl1;
   }
}
      

/*----------------------------------------------------------------------------*/
/*                         Ende des Files                                     */
/*----------------------------------------------------------------------------*/


