/*-------------------------------------------------------------------------

File        : lrn_filevars.c

Autor       : Stephan Schulz

Inhalt      : Funktionen zum Parsen von Variablen, die
              in Files gespeichert werden.

Aenderungen : <1> 15.12.196 neu

-------------------------------------------------------------------------*/

#include "lrn_filevars.h"


/*-----------------------------------------------------------------------*/
/*           Forward-Deklaration interner Funktionen                     */
/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/*                     Interne Funktionen                                */
/*-----------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
//
// Function: find_varcell()
//
//   Search for the entry with first element varname in the
//   {NULL,*,*,*} terminated array, return the position (or -1 if not
//   found). 
//
// Globale Variable : -
//
// Seiteneffekte    : -
//
//-----------------------------------------------------------------------*/

long find_varcell(char *varname, FVarCell vars[])
{
   long i;
   
   for(i=0; vars[i].var; i++)
   {
      if(strcmp(vars[i].var, varname)== 0)
      {
	 return i;
      }
   }
   return -1;
}

/*-----------------------------------------------------------------------*/
/*                  Exportierte Funktionen                               */
/*-----------------------------------------------------------------------*/

/*-------------------------------------------------------------------------
//
// Function: ReadFileVariables()
//
//   Parse the named for settings of the form var = value. Search for
//   var in the {NULL,*,*,*} terminated array and put the value
//   into the associated variable. Returns number of variables read.
//
// Globale Variable : out
//
// Seiteneffekte    : IO
//
//-----------------------------------------------------------------------*/

long ReadFileVariables(char* file, FVarCell vars[])
{
   long varcount = 0,
        varpos = 0;
   char* varname;

   OpenInput(file);
   InitScanner(in, file);
   
   NextRealToken();
   while(akttoken.token!=NoToken)
   {
      varname = secure_strdup(akttoken.literal);
      AcceptTok(Identifier, "Identifier (variable name)");
      AcceptTok(equ, "=");
      varpos = find_varcell(varname, vars);
      if(varpos==-1)
      {
	 fprintf(stderr, "Warning: Unknown variable found!\n");
	 NextRealToken();
      }
      else if(vars[varpos].bstore)
      {
	 if(test_id(ident, "TRUE"))
	 {
	    *vars[varpos].bstore = TRUE;
	 }
	 else if(test_id(ident, "FALSE"))
	 {
	    *vars[varpos].bstore = FALSE;
	 }
	 else
	 {
	    SetString(&ErrCell, 
		      "'TRUE' or 'FALSE' expected for boolean Variable ");
	    AppendString(&ErrCell, varname);
	    RdErr(ViewString(&ErrCell));
	 }
	 NextRealToken();
      }
      else if(vars[varpos].lstore)
      {
	 *vars[varpos].lstore = akttoken.numval;
	 AcceptTok(number, "Integer number");
      }
      else if(vars[varpos].sstore)
      {
	 *vars[varpos].sstore = secure_strdup(akttoken.literal);
	 AcceptTok(string, "String enclosed in \"\"");
      }
      else
      {
	 SetString(&ErrCell, 
		   "Internal Error: No type for variable ");
	 AppendString(&ErrCell, varname);
	 RdErr(ViewString(&ErrCell));
      }
      FREE(varname);
      varcount++;
   }
   fclose(in);
   ReadFromFile = FALSE;
   return varcount;
}

/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/





