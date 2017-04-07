/*-------------------------------------------------------------------------

File        : lrn_insert.c

Autor       : Stephan Schulz

Inhalt      : Funktionen, die sich mit dem Einfuegen von neuen Daten
              in eine vorhandene Wissensbasis beschaeftigen.

Aenderungen : <1> 9.9.1994 neu
              <2> 2.3.1998 WriteSelectionData hinzugefuegt

-------------------------------------------------------------------------*/

#include "lrn_insert.h"

/*-----------------------------------------------------------------------*/
/*           Forward-Deklaration interner Funktionen                     */
/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/*                     Interne Funktionen                                */
/*-----------------------------------------------------------------------*/



/*-----------------------------------------------------------------------*/
/*                  Exportierte Funktionen                               */
/*-----------------------------------------------------------------------*/


/*-------------------------------------------------------------------------
 
FUNCTION         : Brain_p ReadBrain(String_p kb_path, char* fname,
                           char* example) 

Beschreibung     : Liest ein Brain ein brain und testet, ob exmaple
                   eine neuer, legaler Beispielname ist. Falls ja, so
		   wird das Brain zurueckgegeben, sonst bricht die
		   Funktion mit einer Fehlermeldung ab.

Globale Variable : in, indirekt parser-Variablen

Seiteneffekte    : Speicheroperationen, IO

Aenderungen      : <1> 11.9.1994 neu

-------------------------------------------------------------------------*/
extern StepCell anchor;

Brain_p ReadBrain(char* kb_path, char* fname, char* example)
{
   String_p filename;
   Brain_p  brain;

   filename = AllocString();
   SetString(filename, kb_path);
   AppendChar(filename, '/');
   AppendString(filename, fname);
   
   OpenInput(ViewString(filename));
   InitScanner(in,ViewString(filename));

   NextRealToken();

   brain = ParseBrain();
   fclose(in);
   
   if(!IsUnusedExampleName(example, brain))
   {
      fprintf(stderr,"ERROR: Example %s already used in knowledge base.\n",example);
      exit(ARG_ERROR);
   }
   FreeString(filename);
   
   return brain;
}

/*-------------------------------------------------------------------------
 
FUNCTION         : Dom_p ReadDom(char* kb_path, char* fname) 

Beschreibung     : Liest die Domain in kbpath/fname

Globale Variable : in, indirekt parser-Variablen

Seiteneffekte    : Speicheroperationen, IO

Aenderungen      : <1> 14.9.1994 neu

-------------------------------------------------------------------------*/

Dom_p ReadDom(char* kb_path, char* fname) 
{
   String_p filename;
   Dom_p    dom;

   filename = AllocString();
   SetString(filename, kb_path);
   AppendChar(filename, '/');
   AppendString(filename, fname);
   
   OpenInput(ViewString(filename));
   InitScanner(in,ViewString(filename));
   NextRealToken();
   dom = ParseDom();
   fclose(in);

   FreeString(filename);
   
   return dom;
}


/*-------------------------------------------------------------------------
 
FUNCTION         : Dom_p ReadDomHeadList(char* kb_path, char* fname, 
                                     char* domname) 

Beschreibung     : Liest eine Liste von DomainHeads ein und prueft, ob
                   domname ein neuer Name in dieser Liste ist. Ist
		   dies der Fall, so wird die Liste zurueckgegeben,
		   sonst bricht die Funktion mit einer Fehlermeldung ab.

Globale Variable : in, indirekt parser-Variablen

Seiteneffekte    : Speicheroperationen, IO

Aenderungen      : <1> 11.9.1994 neu

-------------------------------------------------------------------------*/

Dom_p ReadDomHeadList(char* kb_path, char* fname, char* domname)
{
   String_p filename;
   Dom_p  domlist;

   filename = AllocString();
   SetString(filename, kb_path);
   AppendChar(filename, '/');
   AppendString(filename, fname);
   
   OpenInput(ViewString(filename));
   InitScanner(in,ViewString(filename));
   NextRealToken();
   domlist = ParseDomHeadList();
   fclose(in);
   
   if(!IsUnusedDomName(domname, domlist))
   {
      fprintf(stderr,"ERROR: Domain %s already used in knowledge base.\n", 
	      domname);
      exit(ARG_ERROR);
   }
   FreeString(filename);
   
   return domlist;
}



/*-------------------------------------------------------------------------

FUNCTION         : Dom_p FindNewInsertPlace(Dom_p list, Dom_p new_dom)

Beschreibung     : Sucht in einer (bereits nach Groesse der
                   Spezifikation bzw. Anzahl der Ziele geordneten
		   Liste) nach der Stelle, vor der die neue Domain
		   eingeordnet werden sollte. Dabei kommen grosse (=
		   speielle) Domanen vor kleinen (allgemeinen)
		   Domaenen, so das eine moeglichst genau passende
		   Domaene gefunden wird.
		   
Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 11.9.1994 neu

-------------------------------------------------------------------------*/

Dom_p FindNewInsertPlace(Dom_p list, Dom_p new_dom)
{
   long new_size;
   Dom_p handle;

   if(new_dom->type == SpecDom)
   {
      new_size = EqnListSize(new_dom->axioms);

      for(handle = list->next; handle != list; handle = handle->next)
      {
	 if(EqnListSize(handle->axioms)<=new_size)
	 {
	    return handle;
	 }
      }
      return list;
   }
   else
   {
      new_size = EqnListSize(new_dom->theorems);

      for(handle = list->next; handle != list; handle = handle->next)
      {
	 if(EqnListSize(handle->theorems)<=new_size)
	 {
	    return handle;
	 }
      }
      return list;
   }
}



/*-------------------------------------------------------------------------

FUNCTION         : Dom_p FindCompatDom(Dom_p list, Dom_p dom,
                                       NormSubst_p *make_equal)  

Beschreibung     : Durchsucht die Liste nach einer kompatiblen
                   Domaene. Rueckgabewert ist der Pointer auf die
		   Domaene, in *make_equal steht die NormSubst, die
		   Signatur von dom in die der gefundenen Domain
		   ueberfuehrt. Existiert keine geeignete Domain, so
		   wird NULL zurueckgegeben. 

Globale Variable : -

Seiteneffekte    : Speicheroperationen, *make_equal

Aenderungen      : <1> 9.9.1994 neu

-------------------------------------------------------------------------*/

Dom_p FindCompatDom(Dom_p list, Dom_p dom, NormSubst_p *make_equal)
{
   Dom_p handle;

   for(handle = list->next; handle != list; handle = handle->next)
   {
      if((*make_equal = DomEquivSubst(dom, handle)))
      {
	 return handle;
      }
   }
   return NULL;
}

/*-------------------------------------------------------------------------

FUNCTION         : BOOL InsertNewDom(char* kb_path, Dom_p new_dom,
                                     Dom_p domlist) 

Beschreibung     : Testet, ob eine zu new_dom kompatible Domaene in
                   domlist beschrieben ist. Ist das der Fall, so
		   wird diese geladen, die beiden Domaenen vereinigt
		   und wieder zurueckgeschrieben. Die Liste wird
		   entsprechend veraendert. Rueckgabewert ist FALSE.
		   Existiert keine kompatible Domaene, so wird die
		   neue Domaene geschrieben und in die Liste
		   eingefuegt. Rueckgabewert ist TRUE.

Globale Variable : 

Seiteneffekte    : 

Aenderungen      : <1> .1994 neu

-------------------------------------------------------------------------*/

BOOL InsertNewDom(char* kb_path, Dom_p new_dom, Dom_p domlist)
{
   NormSubst_p make_equal;
   Dom_p       insert, tmp_dom;
   String_p    filename;

   filename = AllocString();
   SetString(filename, new_dom->type == SpecDom ? "SPECDOMS" :
		"GOALDOMS");
   AppendChar(filename, '/');
   
   insert = FindCompatDom(domlist, new_dom, &make_equal);

   if(insert)
   {
      VERBOSE(fprintf(stderr,\
		      "New Domain is already known as %s, domains\
 will be merged.\n",insert->name););

      AppendString(filename, insert->name);
      AppendString(filename, new_dom->type == SpecDom ? ".sdm" : ".gdm"); 

      tmp_dom = ReadDom(kb_path, ViewString(filename));
      tmp_dom = MergeDoms(new_dom, tmp_dom, make_equal);
      WriteDom(tmp_dom, kb_path, ViewString(filename));
      
      tmp_dom->next = insert->next;
      tmp_dom->prev = insert->prev;
      (insert->next)->prev = tmp_dom;
      (insert->prev)->next = tmp_dom;
      
      FreeDom(insert);
            
      VERBOUT("Modified domain written.\n")
      return FALSE;
   }
   else
   {
      insert = FindNewInsertPlace(domlist, new_dom);
      VERBOUT("New Domain is unknown.\n");
      AppendString(filename, new_dom->name);
      AppendString(filename, new_dom->type == SpecDom ? ".sdm" : ".gdm");  
      WriteDom(new_dom, kb_path, ViewString(filename));
      new_dom->next = insert;
      new_dom->prev = insert->prev;
      (insert->prev)->next = new_dom;
      insert->prev = new_dom;
      VERBOUT("New domain written\n");
      return TRUE;
   }
}
   


/*-------------------------------------------------------------------------

FUNCTION         : void WriteExample(Step_p anchor, char* kb_path,
                                     char* example) 

Beschreibung     : Schreibt das PCL-Listing, das bei anchor verankert
                   ist, in kb_path/EXAMPLES/example.xtr..

Globale Variable : -

Seiteneffekte    : Ausgabe

Aenderungen      : <1> 11.9.1994 neu

-------------------------------------------------------------------------*/

void WriteExample(Step_p anchor, char* kb_path, char* example)
{
   String_p filename;

   filename = AllocString();
   SetString(filename, kb_path);
   AppendString(filename, "/EXAMPLES/");
   AppendString(filename, example);
   AppendString(filename, ".xtr");
   
   OpenOutput(ViewString(filename));
   PrintOut(anchor);
   EndIo();
   FreeString(filename);
}

/*-------------------------------------------------------------------------

FUNCTION         : void WriteSelectionData(Step_p anchor, char* kb_path,
                                     char* example) 

Beschreibung     : Schreibt Daten, die zur Beispielauswahl benötigt werden
                   in kb_path/SELECTIONDATA/example.sel.

Globale Variable : (indirekt) out

Seiteneffekte    : Ausgabe

Aenderungen      : <1> 2.3.1998 neu

-------------------------------------------------------------------------*/

void WriteSelectionData(Step_p anchor, char* kb_path, char* example)
{
   NormEqn_p axioms, goals, handle;
   Symbol_p sig;
   String_p filename;
   long i, axiomscount, maxarity, arity[DEFAULT_MAX_ARITY],
        d[MAXAXIOMS], goaldepth;
   double averagedepth = 0, deviationdepth = 0;

   SetExampleName(example);

   axioms = GetAxioms(anchor);
   OrientNormEqns(axioms);
   goals = GetGoals(anchor);
   axiomscount = EqnListSize(axioms);
   sig = GetSig(axioms, NULL);
   sig = GetSig(goals, sig);
   maxarity = GetMaxArity(sig);
   for(i = 0; i<=maxarity; i++) arity[i] = GetNumberOfArity(sig, i);

   goals = goals->right;
   goaldepth = TermDepth(goals->lside) + TermDepth(goals->rside);

   /* Calculate Average Depth */

   handle = axioms->right;
   for(i = 0; i < 2*axiomscount; i+=2)
   {
      averagedepth += (d[i] = TermDepth(handle->lside));
      averagedepth += (d[i+1] = TermDepth(handle->rside));
      handle = handle->right;
   }
   averagedepth /= 2*axiomscount;

   /* Compute Depth Standard Deviation */

   for(i = 0; i < 2*axiomscount; i++) deviationdepth += 
				      (d[i]-averagedepth)*(d[i]-averagedepth);

   deviationdepth = sqrt(deviationdepth/(2*axiomscount));

   filename = AllocString();
   SetString(filename, kb_path);
   AppendString(filename, "/SELECTIONDATA/");
   AppendString(filename, example);
   AppendString(filename, ".sel");

   OpenOutput(ViewString(filename));
   fprintf(out, "Number_of_Axioms: %ld\n", axiomscount);
   fprintf(out, "Average_Depth: %f\n", averagedepth);
   fprintf(out, "Depth_Standard_Deviation: %f\n", deviationdepth);
   fprintf(out, "Goal_Depth: %ld\n", goaldepth);
   fprintf(out, "Max_Arity: %ld\n", maxarity);
   fprintf(out, "Arity_Frequencies: ");
   for (i = 0; i<=maxarity; i++) fprintf(out, "%ld ", arity[i]);
   fprintf(out, "\n\nAxioms:\n");
   PrintNormEqnList(axioms,FALSE);

   EndIo();
   FreeString(filename);
}


/*-------------------------------------------------------------------------
 
FUNCTION         : void WriteBrain(Brain_p brain, char* kb_path, char*
                                   fname) 

Beschreibung     : Schreibt das Brain in kb_path/fname.

Globale Variable : (indirekt) out

Seiteneffekte    : IO

Aenderungen      : <1> 13.9.1994 neu

-------------------------------------------------------------------------*/

void WriteBrain(Brain_p brain, char* kb_path, char* fname)
{
   String_p filename;

   filename = AllocString();
   SetString(filename, kb_path);
   AppendChar(filename, '/');
   AppendString(filename, fname);
   
   OpenOutput(ViewString(filename));
   PrintBrain(brain);
   EndIo();
   
   FreeString(filename);

}




/*-------------------------------------------------------------------------
 
FUNCTION         : void WriteDom(Dom_p dom, char* kb_path, char* fname) 

Beschreibung     : Schreibt die Domaene in kb_path/fname.

Globale Variable : (indirekt) out

Seiteneffekte    : IO

Aenderungen      : <1> 14.9.1994 neu

-------------------------------------------------------------------------*/

void WriteDom(Dom_p dom, char* kb_path, char* fname) 
{
   String_p filename;

   filename = AllocString();
   SetString(filename, kb_path);
   AppendChar(filename, '/');
   AppendString(filename, fname);
   
   OpenOutput(ViewString(filename));
   PrintDom(dom);
   EndIo();
   
   FreeString(filename);

}


/*-------------------------------------------------------------------------
 
FUNCTION         : void WriteDomHeadList(Dom_p domlist, char* kb_path,
                                         char* fname)  

Beschreibung     : Schreibt die Domaene in kb_path/fname.

Globale Variable : (indirekt) out

Seiteneffekte    : IO

Aenderungen      : <1> 14.9.1994 neu

-------------------------------------------------------------------------*/

void WriteDomHeadList(Dom_p domlist, char* kb_path, char* fname) 
{
   String_p filename;

   filename = AllocString();
   SetString(filename, kb_path);
   AppendChar(filename, '/');
   AppendString(filename, fname);
   
   OpenOutput(ViewString(filename));
   PrintDomHeadList(domlist);
   EndIo();
   
   FreeString(filename);
}

/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/


