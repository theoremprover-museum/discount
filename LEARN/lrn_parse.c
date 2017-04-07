/*-------------------------------------------------------------------------

File        : lrn_parse.c

Autor       : Stephan Schulz

Inhalt      : Funktionen zu Parser fuer Domaenen, Brains, und die
              dazugehoerigen Datentypen.

Aenderungen : <1> 25.8.1994 neu

-------------------------------------------------------------------------*/

#include "lrn_parse.h"


/*-----------------------------------------------------------------------*/
/*           Forward-Deklaration interner Funktionen                     */
/*-----------------------------------------------------------------------*/

EqnOccur_p parse_eqn_occur();

/*-----------------------------------------------------------------------*/
/*                     Interne Funktionen                                */
/*-----------------------------------------------------------------------*/

/*-------------------------------------------------------------------------

FUNCTION         : EqnOccur_p parse_eqn_occur()

Beschreibung     : Parst einen einzelnen Eintrag einer EqnOccurListe
                   (nur der Bequemlichkeit wegen...) 

Globale Variable : akttoken (aus pcl_parse.c)

Seiteneffekte    : Speicheroperationen, IO

Aenderungen      : <1> 26.8.1994 neu

-------------------------------------------------------------------------*/

EqnOccur_p parse_eqn_occur()
{
   EqnOccur_p handle;

   handle = AllocEqnOccurCell();
   handle->next = NULL;
   handle->example = secure_strdup(akttoken.literal);
#ifdef NAMES_ARE_STRINGS
   if(test(Identifier) || test(string))
   {
      NextRealToken();
   }
   else
   {
      RdErr("Example name (Identifier or \"\"-delimited String" 
	    " expected)");
   }
#else
   AcceptTok(Identifier, "Example name");
#endif /* NAMES_ARE_STRINGS */
   AcceptTok(openbracket, "(");
   if(PosAndNeg)
   {
      if(!(test_id(ident,"T") || (test_id(ident,"F"))))
      {
	 RdErr("Ident ('T' or 'F') expected");
      }
      handle->used = test_id(ident,"T");
      NextRealToken();
      AcceptTok(comma,",");
      handle->cp_cost = akttoken.numval;
      AcceptTok(number,"Number (cost in generated critical pairs)");
      AcceptTok(comma,",");    
   }
   handle->references = akttoken.numval;
   AcceptTok(number,"Number of References");
   AcceptTok(comma, ",");
   handle->goal_dist = akttoken.numval;
   AcceptTok(number, "Distance from Goal");
   AcceptTok(closebracket, ")");

   return handle;
}

/*-----------------------------------------------------------------------*/
/*                  Exportierte Funktionen                               */
/*-----------------------------------------------------------------------*/


/*-------------------------------------------------------------------------

FUNCTION         : Term_p ParseNormTerm()

Beschreibung     : Parst Normterme (durch Aufruf von parse_term(),
                   setzt die norm_ids entsprechend dem Eingabefile und
		   wichtet die Terme mit WeightTerm.

Globale Variable : akttoken (aus pcl_parse.c)

Seiteneffekte    : Speicher, IO

Aenderungen      : <1> 26.8.1994 neu

-------------------------------------------------------------------------*/

Term_p ParseNormTerm()
{
   Term_p term;

   term = parse_term();
   WeightTerm(term);

   return term;
}


/*-------------------------------------------------------------------------

FUNCTION         : NormEqn_p  ParseNormEqn()

Beschreibung     : Parst die NormEqn (nur die Gleichung -
                   ParseNormEqnLine parst das Zubehoer).

Globale Variable : akttoken (aus pcl_parse.c)

Seiteneffekte    : Parser halt :-)

Aenderungen      : <1> 26.8.1994 neu

-------------------------------------------------------------------------*/

NormEqn_p  ParseNormEqn()
{
   NormEqn_p handle;

   handle = AllocNormEqnCell();

   handle->lside = ParseNormTerm();
   AcceptTok(equ, "=");
   handle->rside = ParseNormTerm();
   handle->occur = NULL;

   return handle;
}   


/*-------------------------------------------------------------------------

FUNCTION         : EqnOccur_p ParseOccurList()

Beschreibung     : Parst eine komplette EqnOccurList.

Globale Variable : akttoken (aus pcl_parse.c)

Seiteneffekte    : Parser halt :-)

Aenderungen      : <1> 26.8.1994 neu

-------------------------------------------------------------------------*/

EqnOccur_p ParseOccurList()
{
   EqnOccur_p handle = NULL, akt;

#ifdef NAMES_ARE_STRINGS
   if(test(Identifier) || test(string))
#else
   if(test(Identifier))
#endif /* NAMES_ARE_STRINGS */
   {
      handle = parse_eqn_occur();
      
      akt = handle;
      
      while(test(comma))
      {
	 AcceptTok(comma, ",");
	 akt->next = parse_eqn_occur();
	 akt = akt->next;
      }
   }
   return handle;
}


/*-------------------------------------------------------------------------

FUNCTION         : NormEqn_p  ParseNormEqnLine()

Beschreibung     : Parst eine NormEqn samt EqnOccurList.

Globale Variable : akttoken (aus pcl_parse.c)

Seiteneffekte    : Wie ueblich beim Parsen...

Aenderungen      : <1> 26.8.1994 neu

-------------------------------------------------------------------------*/

NormEqn_p  ParseNormEqnLine()
{
   NormEqn_p handle;

   handle = ParseNormEqn();
   AcceptTok(colon, ":");
   handle->occur = ParseOccurList();
   handle->ave_ref = AverageReferences(handle->occur);
   handle->tot_ref =  TotalReferences(handle->occur);
   handle->goal_dist =  AverageGoalDist(handle->occur);
   
   return handle;
}


/*-------------------------------------------------------------------------

FUNCTION         : long ParseNormEqnList(NormEqn_p anchor, BOOL add)

Beschreibung     : Parst eine NormEqnList und haengt sie an den Anker
                   (eine existierende, eventuell leere, Liste) an.
		   Rueckgabewert ist der groesste wert von goal_dist
		   in einer der Gleichungen. Ist add == false, so wird
		   eine Liste ohne EqnOccurs erwartet - Rueckgabewert
		   ist dann 0. 

Globale Variable : akttoken (aus pcl_parse.c)

Seiteneffekte    : Siehe oben...

Aenderungen      : <1> 26.8.1994 neu

-------------------------------------------------------------------------*/

long ParseNormEqnList(NormEqn_p anchor, BOOL add)
{
   NormEqn_p handle;
   long      max_goal_dist = 0;
   
   while(test(Identifier))
   {
       if(add)
      {
	 handle = ParseNormEqnLine();
	 max_goal_dist = max(max_goal_dist, handle->goal_dist);
      }
      else
      {
	 handle = ParseNormEqn();
      }
      InsertLastNormEqnList(anchor, handle);
   }
   return max_goal_dist;
}


/*-------------------------------------------------------------------------

FUNCTION         : long ParseEqnTree(NormEqn_p *anchor)

Beschreibung     : Parst eine Liste von NormEqnLines und fuegt das
                   Ergebnis in den Baum bei *root ein. Rueckgabewert
		   ist (wie oben) der groesste wert von goal_dist in
		   einer der Gleichungen. 

Globale Variable : akttoken (aus pcl_parse.c)

Seiteneffekte    : Siehe oben...

Aenderungen      : <1> 27.8.1994 neu

-------------------------------------------------------------------------*/

long ParseEqnTree(NormEqn_p *anchor)
{
   NormEqn_p handle;
   long      max_goal_dist = 0;
   
   while(test(Identifier))
   {
      handle = ParseNormEqnLine();
      max_goal_dist = max(max_goal_dist, handle->goal_dist);
      TreeInsertEqn(anchor, handle);
   }
   return max_goal_dist;
}


/*-------------------------------------------------------------------------

FUNCTION         : Sympol_p   ParseSig()

Beschreibung     : Parst eine Signatur.

Globale Variable : akttoken (aus pcl_parse.c)

Seiteneffekte    : IO, Speicher

Aenderungen      : <1> 26.8.1994 neu

-------------------------------------------------------------------------*/

Symbol_p ParseSig()
{
   Symbol_p handle = NULL, *akt;

   akt = &handle;

   while(test(Identifier))
   {
      *akt = AllocSymbolCell();
      (*akt)->next = NULL;
      (*akt)->id = secure_strdup(akttoken.literal);
      (*akt)->norm_id = akttoken.numval;
      AcceptTok(Identifier, "Function symbol");
      AcceptTok(colon, ":");
      (*akt)->arity = akttoken.numval;
      AcceptTok(number, "Arity");
      akt = &((*akt)->next);
   }
   return handle;
}
	 

/*-------------------------------------------------------------------------

FUNCTION         : EqnOccur_p ParseOccurNamesList()

Beschreibung     : Parst eine Liste von Namen und leget sie in einer
                   EqnOccur-Liste ab.

Globale Variable : akttoken (aus pcl_parse.c)

Seiteneffekte    : IO, Speicher

Aenderungen      : <1> 26.8.1994 neu

-------------------------------------------------------------------------*/

EqnOccur_p ParseOccurNamesList()
{
   EqnOccur_p handle = NULL, akt;
   
#ifdef NAMES_ARE_STRINGS
   if(test(Identifier) || test(string))
#else
   if(test(Identifier))
#endif /* NAMES_ARE_STRINGS */
   {
      handle = AllocEqnOccurCell();
      handle->example = secure_strdup(akttoken.literal);
      handle->next = NULL;
#ifdef NAMES_ARE_STRINGS
   if(test(Identifier) || test(string))
   {
      NextRealToken();
   }
   else
   {
      RdErr("Example name (Identifier or \"\"-delimited String" 
	    " expected)");
   }
#else
   AcceptTok(Identifier, "Example name");
#endif /* NAMES_ARE_STRINGS */
      akt = handle;
      
      while(test(comma))
      {
	 AcceptTok(comma, ",");
	 akt->next = AllocEqnOccurCell();
	 (akt->next)->example = secure_strdup(akttoken.literal);
	 (akt->next)->next = NULL;
#ifdef NAMES_ARE_STRINGS
	 if(test(Identifier) || test(string))
	 {
	    NextRealToken();
	 }
	 else
	 {
	    RdErr("Example name (Identifier or \"\"-delimited String" 
		  " expected)");
	 }
#else
	 AcceptTok(Identifier, "Example name");
#endif /* NAMES_ARE_STRINGS */
	 akt = akt->next;
      }
   }
   return handle;
} 


/*-------------------------------------------------------------------------

FUNCTION         : Dom_p ParseDomHead()

Beschreibung     : Parst den Domain-Kopf bis hinter die Theorem bzw.
                   Axiome. 

Globale Variable : akttoken (aus pcl_parse.c)

Seiteneffekte    : Parsen...

Aenderungen      : <1> 26.8.1994 neu

-------------------------------------------------------------------------*/
   
Dom_p ParseDomHead()
{
   Dom_p handle;
   long dummy;

   handle=AllocDomCell();

   AcceptId(colonident,"domain:");
   handle->name = secure_strdup(akttoken.literal);
#ifdef NAMES_ARE_STRINGS
   if(test(Identifier) || test(string))
   {
      NextRealToken();
   }
   else
   {
      RdErr("Domain name (Identifier or \"\"-delimited String" 
	    " expected)");
   }
#else
   AcceptTok(Identifier, "Example name");
#endif /* NAMES_ARE_STRINGS */

   AcceptId(colonident,"type:");
   LOWER_STR(akttoken.literal);
   if(test_id(ident,"goal"))
   {
      handle->type = GoalDom;
   }
   else if(test_id(ident,"specification"))
   {
      handle->type = SpecDom;
   }
   else
   {
      RdErr("'goal' or 'specification' expected");
   }
   AcceptTok(ident, "goal or specification");
   
   AcceptId(colonident,"signature:");
   handle->sig = ParseSig();
   
   AcceptId(colonident,"examples:");
   handle->examples = ParseOccurNamesList();

   if(handle->type == SpecDom)
   {
      AcceptId(colonident, "specification:");
      handle->axioms = AllocNormEqnCell();
      (handle->axioms)->left = (handle->axioms)->right = handle->axioms;
      dummy = ParseNormEqnList(handle->axioms,FALSE);

      handle->theorems = NULL;
   }
   else
   {
      AcceptId(colonident, "goals:");
      handle->theorems = AllocNormEqnCell();
      (handle->theorems)->left = (handle->theorems)->right = handle->theorems;
      dummy = ParseNormEqnList(handle->theorems,FALSE);

      handle->axioms = NULL;
   }

   handle->lemmas = handle->facts = NULL; 
   /* This ensures that the function returns a pointer to a correct */
   /* domain that can be freed by passing it to FreeDom()           */

   return handle;
}


/*-------------------------------------------------------------------------

FUNCTION         : Dom_p ParseDomHeadList()

Beschreibung     : Parst eine Liste von Domain-Koepfen.

Globale Variable : -

Seiteneffekte    : Parsen, wie immer

Aenderungen      : <1> 8.9.1994 neu

-------------------------------------------------------------------------*/

Dom_p ParseDomHeadList()
{
   Dom_p res,
         handle;

   res = AllocDomCell();
   res->next = res->prev = res;

   while(test_id(colonident,"domain:"))
   {
      handle = ParseDomHead();
      handle->next = res;
      handle->prev = res->prev;
      (res->prev)->next = handle;
      res->prev = handle;
   }
   return res;
}



/*-------------------------------------------------------------------------

FUNCTION         : void ParseDomBody(Dom_p dom)

Beschreibung     : Parst Lemmata und Fakten, baut daraus Baeume und
                   haengt sie in die angegebene DomCell.

Globale Variable : akttoken (aus pcl_parse.c)

Seiteneffekte    : Parsen

Aenderungen      : <1> 27.8.1994 neu

-------------------------------------------------------------------------*/

void ParseDomBody(Dom_p dom)
{
   AcceptId(colonident, "lemmas:");
   ParseEqnTree(&(dom->lemmas));
   AcceptId(colonident, "facts:");
   ParseEqnTree(&(dom->facts));
}   


/*-------------------------------------------------------------------------

FUNCTION         : Dom_p ParseDom()

Beschreibung     : Parst eine komplette Domaene, gibt Pointer auf
                   Beschreibung zurueck.

Globale Variable : indirekt (akktoken, out,...)

Seiteneffekte    : Parsen

Aenderungen      : <1> 26.8.1994 neu

-------------------------------------------------------------------------*/

Dom_p ParseDom()
{
   Dom_p handle;
   
   handle = ParseDomHead();
   ParseDomBody(handle);
  
   return handle;
}


/*-------------------------------------------------------------------------

FUNCTION         : Brain_p ParseBrain()

Beschreibung     : Parst ein Brain...

Globale Variable : akktoken

Seiteneffekte    : Indirekt, Speicheroperationen

Aenderungen      : <1> 28.8.1994 neu

-------------------------------------------------------------------------*/

Brain_p ParseBrain()
{
   Brain_p handle;

   handle = AllocBrainCell();

   AcceptId(colonident, "brain:");
   
   handle->name = secure_strdup(akttoken.literal);
   AcceptTok(Identifier, "Brain name");
   
   AcceptId(colonident, "type:");
   
   LOWER_STR(akttoken.literal);
   if(test_id(ident, "complete"))
   {
      handle->type = Complete;
   }
   else if(test_id(ident, "pruned"))
   {
      handle->type = Pruned;
   }
   else
   {
      RdErr("'complete' or 'pruned' expected");
   }
   AcceptTok(ident, "'complete' or 'pruned'");
   
   AcceptId(colonident,"examples:");
   handle->examples = ParseOccurNamesList();

   AcceptId(colonident,"facts:");
   handle->facts = NULL;
   ParseEqnTree(&(handle->facts));

   return handle;
}
   


/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/



