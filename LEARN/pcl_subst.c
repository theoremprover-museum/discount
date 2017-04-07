


/*************************************************************************/
/*                                                                       */
/*   File:        pcl_subst.c                                            */
/*                                                                       */
/*   Autor:       Stephan Schulz                                         */
/*                                                                       */
/*   Inhalt:      Funktionen im Zusammenhang mit Substitutionen und      */
/*                match, mgu, Anwendung einer Substitution...            */
/*                                                                       */
/*   Aenderungen: <1> 17.1.1992 neu                                      */
/*                                                                       */
/*************************************************************************/

#include "pcl_subst.h"



/*-----------------------------------------------------------------------*/
/*                  Externe Variable                                     */
/*-----------------------------------------------------------------------*/


BOOL PrologVars = FALSE;


/*----------------------------------------------------------------------------*/
/*                 Forward-Deklarationen interner Funktionen                  */
/*----------------------------------------------------------------------------*/


Subst_p subst_var(char* var,Subst_p sigma);

BOOL    occur(char* var,Term_p term);

Term_p  apply_subst_list(Term_p tlist, Subst_p subst);

char*   pretty_var(long count);




/*----------------------------------------------------------------------------*/
/*                    Interne Variable                                        */
/*----------------------------------------------------------------------------*/

long var_count = 0;


/*----------------------------------------------------------------------------*/
/*                    Interne Funktionen                                      */
/*----------------------------------------------------------------------------*/


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : Subst_p subst_var(char* var,Subst_p sigma)              */
/*                    IN      char*   var                                     */
/*                    IN      Subst_p sigma                                   */
/*                                                                            */
/* Beschreibung     : Durchsucht die Substitition auf das Vorkommen der       */
/*                    Variable mit dem Namen var. Gibt Pointer auf die Zelle  */
/*                    zurueck, in der var steht, sonst NULL                   */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : -                                                       */
/*                                                                            */
/* Aenderungen      : <1> 24.1.1992 neu                                       */
/*                                                                            */
/******************************************************************************/

Subst_p subst_var(char* var,Subst_p sigma)
{
   Subst_p handle;

   handle = sigma;

   while(handle && strcmp(handle->varid,var))
   {
      handle = handle->chain;
   }
   return handle;
} 


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : BOOL occur(char* var,Term_p term)                       */
/*                    IN    char*    var                                      */
/*                    IN    Term_p   term                                     */
/*                                                                            */
/* Beschreibung     : Untersucht, ob eine Variable in einem Term vorkommt.    */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : -                                                       */
/*                                                                            */
/* Aenderungen      : <1> 29.1.1992 neu                                       */
/*                                                                            */
/******************************************************************************/

BOOL occur(char* var,Term_p term)
{
   Term_p handle;

   if(term->isvar)
   {
      return !strcmp(var,term->id);
   }
   else
   {
      handle = term->args;
      while(handle)
      {
         if(occur(var,handle))
         {
            return TRUE;
         }
         else
         {
            handle = handle->chain;
         }
      }
      return FALSE;
   }
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : Term_p  apply_subst_list(Term_p tlist, Subst_p subst)   */
/*                    INOUT  Term_p tlist                                     */
/*                    IN     Subst_p subst                                    */
/*                                                                            */
/* Beschreibung     : Wendet die Substitution destruktiv auf die verketteten  */
/*                    Terme an.                                               */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Die Terme werden veraendert                             */
/*                                                                            */
/* Aenderungen      : <1> 29.1.1992 neu                                       */
/*                                                                            */
/******************************************************************************/

Term_p  apply_subst_list(Term_p tlist, Subst_p subst)
{
   Term_p handle;

   handle = tlist;
   while(handle)
   {
      ApplySubst(handle,subst);
      handle = handle->chain;
   }
   return tlist;
} 


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : char* pretty_var(long count)                            */
/*                    IN     long count                                       */
/*                                                                            */
/* Beschreibung     : Gibt die count'e "schoene" Variable zurueck.            */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : -                                                       */
/*                                                                            */
/* Aenderungen      : <1> 12.2.1992 neu                                       */
/*                                                                            */
/******************************************************************************/

char* pretty_var(long count)
{
   static char convstr[20];
   static char pretty[] = "xyzuvwpq";
   
   if(!PrologVars)
   {
      if(count<=7)
      {
	 convstr[0] = pretty[count];
	 convstr[1] = '\0';
      }
      else
      {
	 sprintf(convstr,"x%ld",count-8);
      }
   }
   else
   {
      sprintf(convstr,"_%ld",count);
   }
   
   return secure_strdup(convstr);
}




/*----------------------------------------------------------------------------*/
/*                 Exportierte Funktionen                                     */
/*----------------------------------------------------------------------------*/


MakeAlloc(SubstCell);
MakeFree(SubstCell);


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void    FreeSubst(Subst_p junk)                         */
/*                    IN     Subst_p junk                                     */
/*                                                                            */
/* Beschreibung     : Gibt den von einer Substitution belegten Speicherplatz  */
/*                    zurueck.                                                */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Ueber FreeSubstCell                                     */
/*                                                                            */
/* Aenderungen      : <1> 22.2.1992 neu                                       */
/*                                                                            */
/******************************************************************************/

void FreeSubst(Subst_p junk)
{
   Subst_p handle;

   while(junk)
   {
      handle = junk;
      junk = junk->chain;
      FREE(handle->varid);
      FreeTerm(handle->sterm);
      FreeSubstCell(handle);
   }
}



/******************************************************************************/
/*                                                                            */
/* FUNCTION         : Subst_p ApplySubstSubst(Subst_p ssb, Subst_p subst)     */
/*                    IN      Subst_p ssb                                     */
/*                    IN      Subst_p subst                                   */
/*                                                                            */
/* Beschreibung     : Wendet ssb auf die einzusetzenden Terme von subst an.   */
/*                    subst wird destruktiv veraendert, ssb bleibt unver-     */
/*                    aendert.                                                */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Zerstoerung von subst                                   */
/*                                                                            */
/* Aenderungen      : <1> 24.1.1992 neu                                       */
/*                                                                            */
/******************************************************************************/

Subst_p ApplySubstSubst(Subst_p ssb, Subst_p subst)
{
   Subst_p handle;
  
   handle = subst;
   while(handle)
   {
      ApplySubst(handle->sterm,ssb);
      handle = handle->chain;
   }
   return subst;
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : Subst_p CopySubst(Subst_p subst)                        */
/*                    IN       Subst_p subst                                  */
/*                                                                            */
/* Beschreibung     : Kopiert eine Substtitution                              */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : -                                                       */
/*                                                                            */
/* Aenderungen      : <1> 31.1.1992 neu                                       */
/*                                                                            */
/******************************************************************************/

Subst_p CopySubst(Subst_p subst)
{
   Subst_p handle;
   Subst_p *help;

   help = &handle;

   while(subst)
   {
      *help = AllocSubstCell();
      (*help)->varid = secure_strdup(subst->varid);
      (*help)->sterm = CopyTerm(subst->sterm);
      help = &((*help)->chain);
      subst = subst->chain;
   }
   *help = NULL;
  
   return handle;
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void    PrintSubst(Subst_p prt)                         */
/*                    IN      Subst_p prt                                     */
/*                                                                            */
/* Beschreibung     : Gibt eine Substitution aus                              */
/*                                                                            */
/* Globale Variable : out  (aus pcl_printer.c !)                              */
/*                                                                            */
/* Seiteneffekte    : -                                                       */
/*                                                                            */
/* Aenderungen      : <1> 22.2.1992 neu                                       */
/*                                                                            */
/******************************************************************************/

void PrintSubst(Subst_p prt)
{
   fprintf(out,"{");
   while(prt)
   {
      fprintf(out,"%s <- ",prt->varid);
      PrintTerm(prt->sterm);
      prt = prt->chain;
      if(prt)
      {
         fprintf(out, ", ");
      }
   }
   fprintf(out, "}");
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : Term_p  ApplySubst(Term_p term, Subst_p subst)          */
/*                    INOUT  Term_p  term                                     */
/*                    IN     Subst_p subst                                    */
/*                                                                            */
/* Beschreibung     : Wendet eine Substitution auf einen Term an. Der Term    */
/*                    wird destruktiv veraendert.                             */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Aenderung des Terms                                     */
/*                                                                            */
/* Aenderungen      : <1> 22.2.1992 neu                                       */
/*                                                                            */
/******************************************************************************/

Term_p ApplySubst(Term_p term, Subst_p subst)
{
   Term_p handle;
   Subst_p help;

   if(term->isvar)
   {
      help = subst_var(term->id,subst);
      if(help)
      {
         ReplaceThis(term,CopyTerm(help->sterm));
      }
   }
   else
   {
      handle = term->args;
      while(handle)
      {
         ApplySubst(handle,subst);
         handle = handle->chain;
      }
   }
   return term;
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : Subst_p ConcatSubst(Subst_p outer, Subst_p inner)       */
/*                    IN     Subst_p outer                                    */
/*                    IN     Subst_p inner                                    */
/*                                                                            */
/* Beschreibung     : Berechnet outer o inner, beide Eingabesubstitutionen    */
/*                    werden zerstoert.                                       */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Die Eingabesubstitutionen werden zerstoert.             */
/*                                                                            */
/* Aenderungen      : <1> 27.2.1992 neu                                       */
/*                                                                            */
/******************************************************************************/

Subst_p ConcatSubst(Subst_p outer, Subst_p inner)
{
   Subst_p handle, part;

   handle = outer;
   while(handle)
   {
      part = handle;
      handle = handle->chain;
      part->chain = NULL;
      ApplySubstSubst(part,inner);
      if(!subst_var(part->varid,inner))
      {
         part->chain = inner;
         inner = part;
      }
      else
      {
         FreeSubst(part);
      }
   }
   return inner;
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : BOOL Match(Term_p mterm, Term_p target, Subst_p* res    */
/*                    IN    Term_p mterm                                      */
/*                    IN    Term_p target                                     */
/*                    OUT   Subst_p* res                                      */
/*                                                                            */
/* Beschreibung     : Berechnet einen Match von mterm auf target,             */
/*                    Rueckgabe ist TRUE,res, falls ein solcher existiert,    */
/*                    FALSE,NULL sonst.                                       */
/*                    mterm und target werden als ueber chain verkettete      */
/*                    TermLISTEN aufgefasst !                                 */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Die Terme werden zerstoert, *res veraendert.            */
/*                                                                            */
/* Aenderungen      : <1> 29.1.1992 neu                                       */
/*                                                                            */
/******************************************************************************/

BOOL Match(Term_p mterm, Term_p target, Subst_p* res)
{
   Term_p handle1, handle2;
   Subst_p help;

   DEBUG(DMATCH,fprintf(out,"Match: ");PrintArgList(mterm);fprintf(out," >> ");PrintArgList(target);fprintf(out," :\n"););

   *res = NULL;
   while(mterm && target)
   {
      if(mterm->isvar)
      {
         help = subst_var(mterm->id, *res);
         if(help)
         {
            if(!TermEqual(target,help->sterm))
            {
               FreeSubst(*res);
               *res = NULL;
               FreeTermList(mterm);
               FreeTermList(target);
               return FALSE;
            }
         }
         else
         {
            help = AllocSubstCell();
            help->varid = secure_strdup(mterm->id);
            help->sterm = CopyTerm(target);
            help->chain = *res;
            *res = help;
         }
      } 
      else if(target->isvar)
      {
         FreeSubst(*res);
         *res = NULL;  
         FreeTermList(mterm);
         FreeTermList(target);
         return FALSE;
      }  
      else if(strcmp(mterm->id,target->id))
      {
         FreeSubst(*res);
         *res = NULL;  
         FreeTermList(mterm);
         FreeTermList(target);
         return FALSE;
      }
      else
      {
         mterm = ConcatTermLists(mterm,mterm->args);
         target = ConcatTermLists(target,target->args);
      }
      handle1 = mterm;
      handle2 = target;
      mterm = mterm->chain;
      target = target->chain;
      FreeTermCell(handle1);
      FreeTermCell(handle2);
   }
   DEBUG(DMATCH,fprintf(out,"...exists: ");PrintSubst(*res);fprintf(out,"\n"););
   return TRUE;
}

      
/******************************************************************************/
/*                                                                            */
/* FUNCTION         : BOOL Mgu(Term_p term1, Term_p term2, Subst_p* res)      */
/*                    IN     Term_p   term1                                   */
/*                    IN     Term_p   term1                                   */
/*                    OUT    Subst_p* subst                                   */
/*                                                                            */
/* Beschreibung     : Berechnet den MGU zwischen term1 und term2.             */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Die Terme werden zerstoert, *res veraendert             */
/*                                                                            */
/* Aenderungen      : <1> 29.1.1992 neu                                       */
/*                                                                            */
/******************************************************************************/

BOOL Mgu(Term_p term1, Term_p term2, Subst_p* res)
{
   Term_p handle1, handle2;
   Subst_p help;

   DEBUG(DMATCH,fprintf(out,"Mgu: ");PrintArgList(term1);fprintf(out," <> ");PrintArgList(term2);fprintf(out," :\n"););
   *res = NULL;
   while(term1 && term2)
   {
      if(!TermEqual(term1,term2))
      {
         if(term1->isvar)
         {
            if(occur(term1->id,term2))
            {
               FreeSubst(*res);
               *res = NULL;
               FreeTermList(term1);
               FreeTermList(term2);
               return FALSE;
            }
            else
            {
               help = AllocSubstCell();
               help->varid = secure_strdup(term1->id);
               help->sterm = CopyTerm(term2);
               help->chain = NULL;
               apply_subst_list(term1,help);
               apply_subst_list(term2,help);
               *res = ConcatSubst(help,*res);
            }
         }   /* if(term1->isvar)  */
         else if(term2->isvar)
         {
            if(occur(term2->id,term1))
            {
               FreeSubst(*res);
               *res = NULL;
               FreeTermList(term1);
               FreeTermList(term2);
               return FALSE;
            }
            else
            {
               help = AllocSubstCell();
               help->varid = secure_strdup(term2->id);
               help->sterm = CopyTerm(term1);
               help->chain = NULL;
               apply_subst_list(term1,help); 
               apply_subst_list(term2,help); 
               *res = ConcatSubst(help,*res);
            }
         }
         else if(strcmp(term1->id,term2->id))
         {
            FreeSubst(*res);
            *res = NULL;
            FreeTermList(term1);
            FreeTermList(term2);
            return FALSE;
         }
         else
         {
            term1 = ConcatTermLists(term1,term1->args);
            term2 = ConcatTermLists(term2,term2->args);
         }
      }   /* if(!TermEqual)...  */
      handle1 = term1;
      handle2 = term2;
      term1 = term1->chain;
      term2 = term2->chain;
      FreeTermCell(handle1);
      FreeTermCell(handle2);
   }  /* while  */
   DEBUG(DMATCH,fprintf(out,"...exists: ");PrintSubst(*res);fprintf(out,"\n"););
   return TRUE;
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : Subst_p UniqSubst(Subst_p yet, Term_p term)             */
/*                    IN    Subst_p yet                                       */
/*                    IN    Term_p  term                                      */
/*                                                                            */
/* Beschreibung     : Ergaenzt die Substitution yet, indem fuer jede Variable */
/*                    die nicht bereits in yet substituiert wird, eine neue,  */
/*                    noch nicht vorkommende Variable gewaehlt wird.          */
/*                                                                            */
/* Globale Variable : var_counter                                             */
/*                                                                            */
/* Seiteneffekte    : var_counter wird hochgezaehlt, ein weiterer Pointer auf */
/*                    yet kann erzeugt werden.                                */
/*                                                                            */
/* Aenderungen      : <1> 6.2.1992 neu                                        */
/*                                                                            */
/******************************************************************************/

Subst_p UniqSubst(Subst_p yet, Term_p term)
{
   static char convstr[20];
   Subst_p handle;
   Term_p  iter;

   if(term->isvar)
   {
      if(!subst_var(term->id,yet))
      {
         handle = AllocSubstCell();
         handle->varid = secure_strdup(term->id);
         handle->sterm = AllocTermCell();
         handle->sterm->isvar = TRUE;
         handle->sterm->chain = NULL;
         handle->sterm->args  = NULL;
         sprintf(convstr,"#%ld",var_count++);
         handle->sterm->id = secure_strdup(convstr);
         handle->chain = yet;
         yet = handle;
      }
   }
   else
   {
      for(iter = term->args; iter; iter = iter->chain)
      {
         yet = UniqSubst(yet,iter);
      }
   }
   return yet;
}

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : Subst_p PrettySubst(Subst_p yet, Term_p term)           */
/*                    IN    Subst_p yet                                       */
/*                    IN    Term_p  term                                      */
/*                                                                            */
/* Beschreibung     : Ergaenzt die Substitution yet, indem fuer jede Variable */
/*                    vom Typ "#no", die nicht bereits in yet substituiert    */
/*                    wird, eine neue "schoene" Variable gewaehlt wird.       */
/*                                                                            */
/* Globale Variable : var_counter                                             */
/*                                                                            */
/* Seiteneffekte    : var_counter wird hochgezaehlt, ein weiterer Pointer auf */
/*                    yet kann erzeugt werden.                                */
/*                                                                            */
/* Aenderungen      : <1> 6.2.1992 neu                                        */
/*                                                                            */
/******************************************************************************/

Subst_p PrettySubst(Subst_p yet, Term_p term)
{
   Subst_p used_vars,
           term_vars,
           handle;
   long    pretty_counter = 0;

   used_vars = NULL;
   handle = yet;
   while(handle)
   {
      used_vars = VarList(used_vars,handle->sterm);
      handle = handle->chain;
   }
   term_vars = VarList(NULL,term);
   while(term_vars)
   {
      if(!subst_var(term_vars->varid,yet) && (*(term_vars->varid) == '#'))
      {
         handle = term_vars;
         term_vars = term_vars->chain;
         FREE(handle->sterm->id);
         handle->sterm->id = pretty_var(pretty_counter++);
         while(subst_var(handle->sterm->id,used_vars))
         {
            FREE(handle->sterm->id);
            handle->sterm->id = pretty_var(pretty_counter++);
         }
         handle->chain = yet;
         yet = handle;
         used_vars = VarList(used_vars,handle->sterm); 
      }
      else if(*(term_vars->varid) != '#')
      {
         handle = term_vars;
         term_vars = term_vars->chain;
         handle->chain = yet;
         yet = handle; 
      }
      else
      {
         handle = term_vars;
         term_vars = term_vars->chain;
         FREE(handle->varid);
         FreeTerm(handle->sterm); 
         FreeSubstCell(handle);
       }
   }
   FreeSubst(used_vars);  

   return yet;
}



/******************************************************************************/
/*                                                                            */
/* FUNCTION         : Term_p MakeUniq(Term_p term)                            */
/*                    IN     Term_p term                                      */
/*                                                                            */
/* Beschreibung     : Ersetzt die Variablen in einem Term durch "neue"        */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Term wird veraendert                                    */
/*                                                                            */
/* Aenderungen      : 10.2.1992 neu                                           */
/*                                                                            */
/******************************************************************************/

Term_p MakeUniq(Term_p term)
{
   Subst_p uniq;

   uniq = UniqSubst(NULL,term);

   term = ApplySubst(term,uniq);

   FreeSubst(uniq);

   return term;
}

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : Term_p MakePretty(Term_p term)                          */
/*                    IN     Term_p term                                      */
/*                                                                            */
/* Beschreibung     : Ersetzt die Variablen in einem Term durch "schoene"     */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : Term wird veraendert                                    */
/*                                                                            */
/* Aenderungen      : 12.2.1992 neu                                           */
/*                                                                            */
/******************************************************************************/

Term_p MakePretty(Term_p term)
{
   Subst_p pretty;

   pretty = PrettySubst(NULL,term);

   term = ApplySubst(term,pretty);

   FreeSubst(pretty);

   return term;
}

/*-------------------------------------------------------------------------

FUNCTION         : Pair_p MakePrettyPair(Pair_p pair)

Beschreibung     : Generiert f"ur das Termpaar neue, sch"one Variable
                   und setzt diese ein.

Globale Variable : -

Seiteneffekte    : Terme werden destruktiv ver"andert

Aenderungen      : <1> 22.4.1994 neu

-------------------------------------------------------------------------*/

Pair_p MakePrettyPair(Pair_p pair)
{
   Subst_p pretty;

   pretty = UniqSubst(NULL,pair->lside);
   pretty = UniqSubst(pretty,pair->rside);
   pair->lside = ApplySubst(pair->lside,pretty);
   pair->rside = ApplySubst(pair->rside,pretty);
   FreeSubst(pretty);
   
   pretty = PrettySubst(NULL,pair->lside);
   pretty = PrettySubst(pretty,pair->rside);
   pair->lside = ApplySubst(pair->lside,pretty);
   pair->rside = ApplySubst(pair->rside,pretty);
   FreeSubst(pretty);

   return pair;
}



/******************************************************************************/
/*                                                                            */
/* FUNCTION         : Subst_p VarList(Subst_p yet, Term_p term)               */
/*                    IN    Subst_p yet                                       */
/*                    IN    Term_p term                                       */
/*                                                                            */
/* Beschreibung     : Erweitert die Substitution yet fuer alle Variablen in   */
/*                    term, die nicht bereits in yet substituiert sind, um    */
/*                    die triviale Substitution {x <- x}.                     */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : yet wird veraendert                                     */
/*                                                                            */
/* Aenderungen      : <1> 11.2.1992 neu                                       */
/*                                                                            */
/******************************************************************************/

Subst_p VarList(Subst_p yet, Term_p term)
{
   Subst_p handle;
   Term_p  help;

   if(term->isvar)
   {
      if(!subst_var(term->id,yet))
      {
         handle = AllocSubstCell();
         handle->varid = secure_strdup(term->id);
         handle->sterm = CopyTerm(term);
         handle->chain = yet;
         yet = handle;
      }
   }
   else
   {
      help = term->args;
      while(help)
      {
         yet = VarList(yet,help);
         help = help->chain;
      }
   }
   return yet;
}
      


/*----------------------------------------------------------------------------*/
/*                         Ende des Files                                     */
/*----------------------------------------------------------------------------*/


