/*-------------------------------------------------------------------------

File        : lrn_norm.c

Autor       : Stephan Schulz

Inhalt      : Funktionen zu normierten Termen und Termpaaren, d.h.
              Termen, in denen die Funktionssymbole in aufsteigender
	      Reihenfolge auftreten. ZU beachten ist, das nur bereits
	      gewichtete Terme zu (legalen) Norm-Termen gemacht werden
	      duerfen - sonst versagt z.B. der Vergleich zweier
	      Normterme. 

Aenderungen : <1> 19.6.1994

-------------------------------------------------------------------------*/


#include "lrn_norm.h"


/*-----------------------------------------------------------------------*/
/*                   Exportierte Variablen                               */
/*-----------------------------------------------------------------------*/

BOOL PreserveArity = FALSE;


/*-----------------------------------------------------------------------*/
/*           Forward-Deklaration interner Funktionen                     */
/*-----------------------------------------------------------------------*/


NormSubst_p find_norm_val(NormSubst_p list, char* id);

long  cmp_term_cell(Term_p term1, Term_p term2);




/*-----------------------------------------------------------------------*/
/*                     Interne Funktionen                                */
/*-----------------------------------------------------------------------*/



/*-------------------------------------------------------------------------

FUNCTION         : NormSubst_p find_norm_val(NormSubst_p list, char* id) 

Beschreibung     : Suche in der Liste nach einem Norm-Id, der zum id
                   passt. Rueckgabewert ist ein Pointer auf die Zelle,
		   wenn eine solche existiert, NULL sonst.

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 19.6.1994 neu

-------------------------------------------------------------------------*/

NormSubst_p find_norm_val(NormSubst_p list, char* id)
{
   NormSubst_p handle;

   for(handle = list; 
       handle && strcmp(id,handle->id);  
       handle = handle->next);  
   
   return handle;
}


/*-------------------------------------------------------------------------

FUNCTION         : long cmp_term_cell(Term_p term1, Term_p term2)

Beschreibung     : Vergleicht zwei Termzellen nach Norm-ids: NULL ist
                   kleiner als jede andere, ansonten entscheidet erst
		   term_weight, dann (eventuel) arity und norm_ids. 

Globale Variable : 

Seiteneffekte    : -

Aenderungen      : <1> 15.7.1994 neu

-------------------------------------------------------------------------*/

long cmp_term_cell(Term_p term1, Term_p term2)
{
   long res;
   
   if(term1 && !term2)
   {
      res = 1;
   }
   else if (!term1 && term2)
   {
      res = -1;
   }
   else if(!(res = (term1->term_weight - term2->term_weight)))
   {
      if(!PreserveArity || !(res = (term1->arity - term2->arity)))
      {
	 res = (term1->norm_id - term2->norm_id);
      }
   }
   return res;
}




/*-----------------------------------------------------------------------*/
/*                  Exportierte Funktionen                               */
/*-----------------------------------------------------------------------*/

MakeAlloc(NormAdminCell);
MakeFree(NormAdminCell);

/*-------------------------------------------------------------------------
//
// Function: CreateNormAdmin()
//
//  Create an initialize NormAdminCell. Information is stored for
//  arities 0..DEFAULT_MAX_ARITY-1, and is initialized to 0.
//
// Globale Variable : -
//
// Seiteneffekte    : Memory operations
//
//-----------------------------------------------------------------------*/

NormAdmin_p CreateNormAdmin()
{
  NormAdmin_p handle = AllocNormAdminCell();
  long        i;

  handle->size  = DEFAULT_MAX_ARITY;
  handle->array = (long*)secure_malloc(sizeof(long)*DEFAULT_MAX_ARITY);
  
  for(i=0; i<DEFAULT_MAX_ARITY; i++)
  {
    handle->array[i]=0;
  }
  return handle;
}

/*-------------------------------------------------------------------------
//
// Function: FreeNormAdmin()
//
//   Frees the AdminCell and the array it contains.
//
// Globale Variable : -
//
// Seiteneffekte    : Memory operations.
//
//-----------------------------------------------------------------------*/

void FreeNormAdmin(NormAdmin_p junk)
{
   if(junk)
   {
      if(junk->array)
      {
	 FREE(junk->array);
      }
      FreeNormAdminCell(junk);
   }
}

/*-------------------------------------------------------------------------
//
// Function: CopyNormAdmin()
//
//   Returns a Pointer to a copy of the NormAdminCell. This function
//   will handle NULL correctly (i.e. return NULL).
//
// Globale Variable : -
//
// Seiteneffekte    : Memory Operations
//
//-----------------------------------------------------------------------*/

NormAdmin_p CopyNormAdmin(NormAdmin_p source)
{
   NormAdmin_p handle = NULL;

   if(source)
   {
      handle = AllocNormAdminCell();
      
      handle->size = source->size;
      handle->array = (long*)secure_malloc(sizeof(long) * source->size);
      memcpy(handle->array, source->array, sizeof(long) * source->size);
   }
   return handle;
}


/*-------------------------------------------------------------------------
//
// Function: SetNormAdminVal()
//
//  Sets the value associated with arity in the NormAdminCell to
//  value. If arity is greater than the size already covered, new
//  memory is allocated and the block is moved. Return value is the
//  value set.
//
// Globale Variable : -
//
// Seiteneffekte    : Memory operations.
//
//-----------------------------------------------------------------------*/

long SetNormAdminVal(NormAdmin_p cell, long arity, long value)
{
  long* tmp_array;

  if(arity>=cell->size)
  {
    tmp_array =
      (long*)secure_malloc(sizeof(long)*(arity+1));
    memcpy(tmp_array, cell->array, (cell->size+1));
    free(cell->array);
    cell->size = arity+1;
  }
  cell->array[arity] = value;

  return value;
}
		 

/*-------------------------------------------------------------------------
//
// Function: GetNormAdminVal()
//
//  Returns the value associated with arity in the NormAdminCell. If
// arity is greater than the size already covered, returns 0 (this is
// not an error!). 
//
// Globale Variable : -
//
// Seiteneffekte    : -
//
//-----------------------------------------------------------------------*/

long GetNormAdminVal(NormAdmin_p cell, long arity)
{
   long ret;

  if(arity>=cell->size)
  {
    ret= 0;
  }
  else
  {
     ret = cell->array[arity];
  }

  return ret;  
}

/*-------------------------------------------------------------------------
//
// Function: IncNormAdminVal()
//
// Increments the value associated with arity in the NormAdminCell by
// one. Returns new value.  
//
// Globale Variable : -
//
// Seiteneffekte    : May lead to memory operations via SetNormVal()
//
//-----------------------------------------------------------------------*/

long IncNormAdminVal(NormAdmin_p cell, long arity)
{
  return SetNormAdminVal(cell, arity, 
		    GetNormAdminVal(cell, arity)+1);
}


MakeAlloc(NormSubstCell);
MakeFree(NormSubstCell);


/*-------------------------------------------------------------------------

FUNCTION         : void FreeNormSubst(NormSubst_p junk)

Beschreibung     : Gib NormSubst-Liste zurueck. Die Strings werden
                   ebenfalls deallokiert, auch die NormAdminCell,
		   falls der Pointer im Head nicht NULL ist.

Globale Variable : -

Seiteneffekte    : Speicheroperationen 

Aenderungen      : <1> 20.6.1994 neu

-------------------------------------------------------------------------*/

void FreeNormSubst(NormSubst_p junk)
{
   NormSubst_p help;

   if(junk && junk->f_admin)
   {
      FreeNormAdmin(junk->f_admin);
      junk->f_admin = NULL;
   }

   while(junk)
   {
      help = junk->next;
      FREE(junk->id);
      if(junk->f_admin)
      {
	 Error("NormSubst has NormAdmin-Info in non-head-cell "
	       "(FreeNormSubst() in lrn_norm.c\n");
      }
      FreeNormSubstCell(junk);
      junk = help;
   }
}


/*-------------------------------------------------------------------------

FUNCTION         : NormSubst_p FunNormTermList(Term_p t_list,
                                               NormSubst_p* subst,
					       BOOL modify) 

Beschreibung     : Normiere die Funktionssymbole in einer Liste von
                   Termen, d.h. fuege in jede Termzelle die
		   Norm-Identifier ein, die fuer jedes in der Liste
		   vorkommende Funktionssymbol in aufsteigender
		   Reihenfolge vergeben werden. Rueckgabewert ist ein
		   Pointer auf die dabei aufgebaute Norm-Substitution.
		   Ist modify == FALSE, so wird nur die Substitution
		   aufgebaut, die Terme bleiben unver"andert.

Globale Variable : -

Seiteneffekte    : Eine NormSubst wird aufgebaut.

Aenderungen      : <1> 20.6.1994 neu

-------------------------------------------------------------------------*/

NormSubst_p FunNormTermList(Term_p t_list, NormSubst_p subst, BOOL modify)
{
   Term_p handle;
   NormSubst_p help;

   for(handle = t_list; handle; handle = handle->chain)
   {
      if(!(handle->isvar))
      {
	 if((help = find_norm_val(subst, handle->id)))
	 {
	    if(modify)
	    {
	       handle->arity = help->arity;
	       handle->norm_id = help->norm_id;
	    }
	 }
	 else
	 {
	    help = AllocNormSubstCell();
	    help->id = secure_strdup(handle->id);
	    help->next = subst;
	    if(subst)
	    {
	       help->v_count = subst->v_count;
	       help->f_admin = subst->f_admin;
	       subst->f_admin = NULL;
	    }
	    else
	    {
	       help->v_count = 0;
	       help->f_admin = CreateNormAdmin();
	    }
	    if(PreserveArity)
	    {
	       help->arity = GetTermCellArity(handle);
	       help->norm_id = IncNormAdminVal(help->f_admin,
					       help->arity);
	    }
	    else
	    {     
	       help->arity = 0;
	       help->norm_id = IncNormAdminVal(help->f_admin, 0);
	    }
	    subst = help;
	    
	    if(modify)
	    {
	       handle->arity = help->arity;
	       handle->norm_id = help->norm_id;
	    }
	 }
      }
      subst = FunNormTermList(handle->args, subst,modify);
   }
   return subst;
}


/*-------------------------------------------------------------------------

FUNCTION         : NormSubst_p VarNormTermList(Term_p t_list,
                                            NormSubst_p* subst,
					    BOOL modify) 

Beschreibung     : Normiere die Variablensymbole in einer Liste von
                   Termen, d.h. fuege in jede Termzelle die
		   Norm-Identifier ein, die fuer jedes in der Liste
		   vorkommende Variablensymbol in aufsteigender
		   Reihenfolge vergeben werden. Rueckgabewert ist ein
		   Pointer auf die dabei aufgebaute Norm-Substitution.
		   Ist modify == FALSE, so wird nur die Substitution
		   aufgebaut, die Terme bleiben unver"andert.

Globale Variable : -

Seiteneffekte    : Eine NormSubst wird aufgebaut.

Aenderungen      : <1> 20.6.1994 neu

-------------------------------------------------------------------------*/

NormSubst_p VarNormTermList(Term_p t_list, NormSubst_p subst, BOOL modify)
{
   Term_p handle;
   NormSubst_p help;

   for(handle = t_list; handle; handle = handle->chain)
   {
      if(handle->isvar)
      {
	 if((help = find_norm_val(subst, handle->id)))
	 {
	    if(modify)
	    {
	       handle->arity = 0;
	       handle->norm_id = help->norm_id;
	    }
	 }
	 else
	 {
	    help = AllocNormSubstCell();
	    help->id = secure_strdup(handle->id);
	    help->next = subst;
	    if(subst)
	    {
	       help->v_count = subst->v_count;
	       help->f_admin = subst->f_admin;
	       subst->f_admin = NULL;
	    }
	    else
	    {
	       help->v_count = 0;
	       help->f_admin = CreateNormAdmin();
	    }
	    help->arity = 0;
	    help->norm_id = ++(help->v_count);
	    subst = help;
	    
	    if(modify)
	    {
	       handle->arity = 0;
	       handle->norm_id = help->norm_id;
	    }
	 }
      }
      subst = VarNormTermList(handle->args, subst,modify);
   }
   return subst;
}


/*-------------------------------------------------------------------------

FUNCTION         : NormSubst_p NormTermList(Term_p t_list,
                                            NormSubst_p subst,
                                            BOOL modify) 

Beschreibung     : Normiere eine Liste von Termen, d.h. fuege in jede
                   Termzelle die Norm-Identifier ein, die fuer jedes
                   in der Liste vorkommende Funktions- bzw.
                   Variablensymbol in aufsteigender Reihenfolge
                   vergeben werden. Rueckgabewert ist ein Pointer auf
                   die dabei aufgebaute Norm-Substitution. Ist
                   modify == FALSE, so wird nur die Substitution
                   aufgebaut. 

Globale Variable : -

Seiteneffekte    : Eine NormSubst wird aufgebaut.

Aenderungen      : <1> 20.6.1994 neu

-------------------------------------------------------------------------*/

NormSubst_p NormTermList(Term_p t_list, NormSubst_p subst, BOOL modify) 
{
   return VarNormTermList(t_list, FunNormTermList(t_list, subst,
						  modify), modify);
}


/*-------------------------------------------------------------------------

FUNCTION         : NormSubst_p CopyNormSubst(NormSubst_p subst)

Beschreibung     : Kopiert eine NormSubst-Liste.

Globale Variable : -

Seiteneffekte    : Speicheroperationen

Aenderungen      : <1> 14.8.1994 neu

-------------------------------------------------------------------------*/

NormSubst_p CopyNormSubst(NormSubst_p subst)
{
   NormSubst_p handle = NULL;

   if(subst)
   {
      handle = AllocNormSubstCell();
      handle->id = secure_strdup(subst->id);
      handle->arity = subst->arity;
      handle->norm_id = subst->norm_id;
      handle->v_count = subst->v_count;
      handle->f_admin = CopyNormAdmin(subst->f_admin);
      handle->next = CopyNormSubst(subst->next);
   }
   return handle;
}


/*-------------------------------------------------------------------------

FUNCTION         : long WeightTerm(Term_p term)

Beschreibung     : Gewichte einen Term (Variablen=1, Funktionssymbole
                   =2, komplex: Summe der Komponenten)

Globale Variable : -

Seiteneffekte    : Termgewichte aendern sich

Aenderungen      : <1> 6.8.1994 neu

-------------------------------------------------------------------------*/

long WeightTerm(Term_p term)
{
   Term_p handle; 
   long   weight = 0;

   if(!term)
   {
      return 0;
   }

   for(handle = term->args; handle; handle = handle->chain)
   {
      weight = weight + WeightTerm(handle);
   }

   if(term->isvar)
   {
      weight += 1;
   }
   else
   {
      weight += 2;
   }
   term->term_weight = weight;

   return weight;
}
   


/*-------------------------------------------------------------------------

FUNCTION         : Term_p NormTerm(Term_p term)

Beschreibung     : Normiert einen Term, gibt Pointer auf ihn zurueck.

Globale Variable : siehe NormTermList

Seiteneffekte    : Setzen der norm_id's im Term, term->chain = NULL!

Aenderungen      : <1> 26.6.1994 neu

-------------------------------------------------------------------------*/

Term_p NormTerm(Term_p term)
{
   term->chain = NULL ; /* Nur zur Vorsicht */

   FreeNormSubst(NormTermList(term,NULL,TRUE));

   return term;
}



/*-------------------------------------------------------------------------

FUNCTION         : Term_p ApplyNormSubst(Term_p term, NormSubst_p subst)

Beschreibung     : Wende die NormSubst auf den Term an. Die Identifier
                   im Term werden ver"andert, in subst nicht belegte
		   Symbole erhalten 0. Die Funktion erwartet
		   term->chain = NULL (oder sie bearbeitet die Liste,
		   die dort haengt).

Globale Variable : -

Seiteneffekte    : Veraenderung der norm_ids in term

Aenderungen      : <1> 6.7.1994 neu

-------------------------------------------------------------------------*/

Term_p ApplyNormSubst(Term_p term, NormSubst_p subst)
{
   Term_p handle;
   NormSubst_p help;

   for(handle = term; handle; handle = handle->chain)
   {

      if((help = find_norm_val(subst, handle->id)))
      {
	 handle->arity = help->arity;
	 handle->norm_id = help->norm_id;
      }
      else
      {
	 handle->arity = GetTermCellArity(term);
	 handle->norm_id = 0;
      }
      ApplyNormSubst(handle->args, subst);
   }
   return term;
}
  




/*-------------------------------------------------------------------------

FUNCTION         : long CmpNormTerms(Term_p term1, Term_p term2)

Beschreibung     : Vergleicht zwei Terme lexikographisch nach ihren
                   norm_ids. Rueckgabewert ist >,<,= 0 analog zum
		   Verhalten von strcmp. 

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 27.6.1994 neu

-------------------------------------------------------------------------*/

long CmpNormTerms(Term_p term1, Term_p term2)
{
   long res = 0;
   Term_p handle1, handle2;

   res = cmp_term_cell(term1,term2);

   if(!res && term1 && term2)
   {
      handle1 = term1->args;
      handle2 = term2->args;

      while(!res &&(handle1 || handle2))
      {
	 res = CmpNormTerms(handle1,handle2);
	 
	 if(!res && handle1 && handle2)
	 {
	    handle1 = handle1->chain;
	    handle2 = handle2->chain;
	 }
      }
   }      

   return res;
}


/*-------------------------------------------------------------------------

FUNCTION         : void DebugPrintNormSubst(NormSubst_p subst)

Beschreibung     : Gibt die Normsubst in einer gut lesbaren Form auf
                   stdout aus.

Globale Variable : -

Seiteneffekte    : Ausgabe

Aenderungen      : <1> 14.9.1994 neu

-------------------------------------------------------------------------*/

void DebugPrintNormSubst(NormSubst_p subst)
{
   NormSubst_p handle;
   
   for(handle = subst; handle; handle = handle->next)
   {
      printf("[%s -> %ld_%ld]", handle->id, handle->arity,
	     handle->norm_id); 
   }
   printf("\n");
}


/*-------------------------------------------------------------------------
//
// Function: PrintNormTerm()
//
//  Gibt term mit den norm_ids aus. Benutzt die Variable NoBrackets,
//  um festzustellen, ob Konstanten mit Klammern ausgegeben werden
//  sollen, und PreserveArity, um die genaue Form des Normidents
//  festzulegen. 
//
// Globale Variable : 
//
// Seiteneffekte    : 
//
//-----------------------------------------------------------------------*/

void PrintNormTerm(Term_p term)
{
   if(term)
   {
      if(!(term->isvar))
      {
	 if(term->norm_id)
	 {
	    if(PreserveArity)
	    {
	       fprintf(out,"f%ld_%ld", term->arity, term->norm_id);
	    }
	    else
	    {
	       fprintf(out,"f%ld", term->norm_id);
	    }
	 }
	 else
	 {
	    fprintf(out,term->id);
	 }
         if(!NoBrackets || term->args)
         {
            fprintf(out,"(");
            PrintNormArgList(term->args);
            fprintf(out,")");
         }
      }
      else
      {
	 if(term->norm_id)
	 {	 
	    fprintf(out,"x%ld",term->norm_id); 
	 }
	 else
	 {
	    fprintf(out,term->id);
	 }	    
      }
   }
}


/*-------------------------------------------------------------------------

FUNCTION         : void PrintNormArgList(Term_p list)

Beschreibung     : Gibt die Argumentliste, auf die list zeigt, mit
                   norm_ids aus. 

Globale Variable : out

Seiteneffekte    : Ausgabe

Aenderungen      : <1> 27.6.1994 neu (von PrintArgList())

-------------------------------------------------------------------------*/


void PrintNormArgList(Term_p list)
{
   if(list)
   {
      PrintNormTerm(list);
      for(list = list->chain; list; list = list->chain)
      {
         fprintf(out,",");
         PrintNormTerm(list);
      }
   }
}



/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/


