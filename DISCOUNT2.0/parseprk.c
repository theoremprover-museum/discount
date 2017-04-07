/*--------------------------------------------------------------------

File:    parseprk.c
Autor:   Stephan Schulz

Inhalt: Funktionen zum Einlesen des Kurzprotokolles f"ur die paralelle
Vervollst"andigung. Einiges "ubernommen und ver"andert aus pcl_scan.c.

Aenderung : Es werden aus dem Protokolfile jetzt pro Experte  zwei Angaben
	    "uber durchgef"uhrte Schritte gelesen --> Wegen des Database-
	    Experten. ( MK, 12.04.1994 )
	    
	    ReproMode ersetzt #ifdef REPRO, um 2 Versionen zu
	    sparen... 12.12.1994 StS


----------------------------------------------------------------------*/


#include "parseprk.h"



/*---------------------------------------------------------------------------
               Forward-Deklarationen interner Funktionen                    
----------------------------------------------------------------------------*/


long parse_expert(int* config, long* first_steps, long* second_steps);
long parse_cycle(CycleList_p* res);
long parse_prk(CycleList_p* res);



/*-----------------------------------------------------------------------*/
/*                       Exportierte Funktionen                          */
/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
                                                                           
Funktion: long ReadPrk(char* filename,CycleList_p *res)   

Autor:    StS

Beschreibung: Initialisiert Scanner, liest Datei ein. R"uckgabewert
ist Zahl der Zyklen.

Globale Variable: Akttoken, AktNum, Aktliteral (from s_scanner)

Seiteneffekte: Einlesen des files, etc.

-----------------------------------------------------------------------------*/

long ReadPrk(char* filename,CycleList_p *res)   
{
   long cycles;

   InitScanner(filename);

   cycles = parse_prk(res);

   EndScanner();

   return cycles;
}



/*--------------------------------------------------------------------------

Funktion: long parse_expert(int* config, long* first_steps, long*
                            second_steps) 

Beschreibung: Liest die Beschreibung eines Experten, schreibt
Konfigurationsnummer und Schrittzahlen in *config und *first_steps
bzw. *seconsteps. R"uckgabewert ist Nummer des Prozesses.

---------------------------------------------------------------------------*/

long parse_expert(int* config, long* first_steps, long* second_steps)
{
   long proc;

   AcceptIdent(ident,"process");

   proc = AktNum;
   AcceptToken(number);

   AcceptIdent(ident, "using");
   AcceptIdent(ident, "configuration");

   *config = AktNum;
   AcceptToken(number);

   AcceptToken(openbracket);
   AcceptToken(identifier);
   AcceptToken(closebracket);

   AcceptIdent(ident, "did");

   *first_steps = AktNum;
   AcceptToken(number);

   AcceptIdent(ident, "steps");
   AcceptIdent(ident, "and");

   *second_steps = AktNum;
   AcceptToken(number);
   AcceptIdent(ident, "steps");

   return proc;
}


/*--------------------------------------------------------------------------

Funktion: long parse_cycle(CycleList_p* res)

Beschreibung: Liest die Beschreibung eines Zyklus. Ergebnis kommt nach
*res, R"uckgabewert ist der Zyklus.

---------------------------------------------------------------------------*/

long parse_cycle(CycleList_p* res)
{
   long f,cycle;

   *res = SecureMalloc(sizeof(CycleListCell));

   AcceptIdent(ident,"cycle");

   cycle = AktNum;
   AcceptToken(number);
   
   AcceptIdent(ident, "master");
   AcceptToken(colon);

   (*res)->master = AktNum;

   AcceptToken(number);
   
   f=0;
   
   while(TestIdent(ident, "process"))
   {
      if(parse_expert(&((*res)->confi[f]), &((*res)->first_steps[f]),
		      &((*res)->second_steps[f]))!=f) 
      {
         sprintf(ErrorSpace,"process %d expected",f);
         ScannerError(ErrorSpace);
      }
      f++;
      if(f==MAXEXPERT)
      {
         ScannerError("Too many experts - recompile with larger MAXEXPERT");
      }
   }
   return cycle;
}


/*-------------------------------------------------------------------------

FUNCTION         : long parse_prk(CycleList_p* res)

Beschreibung     : Parses all of the protocol, puts the results into
                   *res and returns the number of cycles done.

Globale Variable : -

Seiteneffekte    : By calling the scanner functions

Aenderungen      : <1> 6.12.1994 Uses new scanner, debugging, decent
                                 comments 

-------------------------------------------------------------------------*/

long parse_prk(CycleList_p* res)
{
   long         i=0;
   CycleList_p* next;

   if(parse_cycle(res)!=i)
   {
      sprintf(ErrorSpace,"cycle %ld expected",i);
      ScannerError(ErrorSpace);
   }

   next = &((*res)->next);
   while(TestIdent(ident, "cycle"))
   {
      i++;
      if(i!=parse_cycle(next))
      {
         sprintf(ErrorSpace,"cycle %ld expected",i);
         ScannerError(ErrorSpace);
      }
      next = &((*next)->next);
   }
   *next = NULL;
   AcceptIdent(ident,"team");
   AcceptIdent(ident,"terminated");
   AcceptIdent(ident,"by");
   AcceptIdent(ident,"process");
   AcceptToken(number);

   if(TestIdent(ident, "during"))
   {
      NextRealToken();
      AcceptToken(ident); /* Eat "completion", "interreduction" or */
			  /* "initialization" */ 
   }
   else
   {
      AcceptIdent(ident,"from");
      AcceptIdent(ident,"unknown");
      AcceptIdent(ident,"function");
   }
   return i;
}


/*----------------------------------------------------------------------------*/
/*                         Ende des Files                                     */
/*----------------------------------------------------------------------------*/



