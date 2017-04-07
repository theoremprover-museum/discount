/*-------------------------------------------------------------------------

File        : learn_nterms.c

Autor       : Stephan Schulz

Inhalt      : Funktionen zu Normtermen in DISCOUNT

Aenderungen : <1> 22.11.1994 neu

-------------------------------------------------------------------------*/

#include "learn_nterms.h"


bool  NormTermPreserveArity = true;

/*-----------------------------------------------------------------------*/
/*                  Exportierte Funktionen                               */
/*-----------------------------------------------------------------------*/


/*-------------------------------------------------------------------------

FUNCTION         : DNormSubst_p AllocDNormSubstCell()

Beschreibung     : Returns a pointer to a free DnormSubstCell. The
                   header is just a dummy, the funtionality is
		   provided in macros defined in learn_nterms.h

FUNCTION         : void FreeDNormSubstCell(DNormSubst_p junk)

Beschreibung     : Returns a DnormSubstCell to the memory management.
                   Is actually a macro, as above.
-------------------------------------------------------------------------*/



/*-------------------------------------------------------------------------

FUNCTION         : void FreeDNormSubst(DNormSubst_p junk)

Beschreibung     : Gibt eine ganze DNormSubstitution zurueck.

Globale Variable : -

Seiteneffekte    : Ueber FreeDNormSubstCell und VTclear

Aenderungen      : <1> 22.11.1994 neu

-------------------------------------------------------------------------*/

void FreeDNormSubst(DNormSubst_p junk)
{
   if(junk)
   {
      VTclear(&(junk->variables));
      FreeDNormSubstCell(junk);
   }
}


/*-------------------------------------------------------------------------

FUNCTION         : DNormSubst_p AllocEmptyDNormSubst()

Beschreibung     : Allociert eine initialisierte, leere DNormSubstCell.

Globale Variable : FuncCount, ArityMax (aus term.[hc], Effizienzhack)

Seiteneffekte    : Speicher

Aenderungen      : <1> 23.11.1994 neu

-------------------------------------------------------------------------*/

DNormSubst_p AllocEmptyDNormSubst()
{
   DNormSubst_p handle;
   
   handle = AllocDNormSubstCell();
   
   handle->f_count = handle ->v_count = 0;
   
   memset(handle->norm_id, 0, (FuncCount+1) * sizeof(short));
   memset(handle->arity_id, 0, (ArityMax+1) * sizeof(short));

   handle->next = NULL;
   handle->variables = NULL;

   return handle;
}


/*-------------------------------------------------------------------------

FUNCTION         : DNormSubst_p DNormTermVars(term* t, DNormSubst_p subst)

Beschreibung     : Normiert einen Term (bzw. baut die DNormSubst)
                   unter Beruecksichtigung der bereits vorgenommenen
		   Bindungen. Nur Variablen werden normiert.

Globale Variable : -

Seiteneffekte    : subst wird veraendert

Aenderungen      : <1> 22.11.1994 neu

-------------------------------------------------------------------------*/

DNormSubst_p DNormTermVars(term* t, DNormSubst_p subst)
{
   short   i;
   
   if(!subst)
   {
      subst = AllocEmptyDNormSubst();
   }

   if (varp(t))
   {
      if(!VTfind(subst->variables, t->fcode))
      {
	 VTadd(&(subst->variables), t->fcode, ++(subst->v_count));
      }
   }
   else
   {
      for(i=0; i<t->arity; i++)
      {
	 DNormTermVars(t, subst); 
      }
   }
   return subst;
}




/*-------------------------------------------------------------------------

FUNCTION         : DNormSubst_p DNormTermFuns(term* t, DNormSubst_p subst)

Beschreibung     : Normiert einen Term unter Beruecksichtigung der
                   bereits vorgenommenen Bindungen. Noch ungebundene
		   Funktionssymbole werden aufsteigend duchnummeriert,
		   Variablen bleiben unveraendert.

Globale Variable : -

Seiteneffekte    : subst wird veraendert

Aenderungen      : <1> 22.11.1994 neu

-------------------------------------------------------------------------*/

DNormSubst_p DNormTermFuns(term* t, DNormSubst_p subst)
{
   short   i;
   
   if(!subst)
   {
      subst = AllocEmptyDNormSubst();
   }

   if (!varp(t))
   {
      if(!subst->norm_id[t->fcode])
      {
	 if(!NormTermPreserveArity)
	 {
	    subst->norm_id[t->fcode] = ++(subst->f_count);
	 }
	 else
	 {
	    subst->norm_id[t->fcode] = ++(subst->arity_id[t->arity]);
	 }
	 for(i=0; i<t->arity; i++)
	 {
	    DNormTermFuns(t, subst); 
	 }
      }
   }
   return subst;
}


/*-------------------------------------------------------------------------

FUNCTION         : DNormSubst_p DNormTerm(term* t, DNormSubst_p subst)

Beschreibung     : Normiert einen Term unter Beruecksichtigung der
                   bereits vorgenommenen Bindungen. Noch ungebundene
		   Funktionssymbole werden aufsteigend duchnummeriert,
		   Variablen ebenso. Diese Funktion koennte eigentlich
		   durch Verkettung von NormTermVars() und
		   NormTermFuns() dargestellt werden. Um eine
		   moeglichst effiziente Ausfuehrung zu erreichen,
		   wird jedoch eine Spezialfunktion verwendet, die
		   beides auf einmal erledigt.

Globale Variable : -

Seiteneffekte    : subst wird veraendert

Aenderungen      : <1> 24.11.1994 neu

-------------------------------------------------------------------------*/

DNormSubst_p DNormTerm(term* t, DNormSubst_p subst)
{
   short   i;
   
/*    printf("DNormTerm(");
   printterm(t);
   printf(") called...\n");
   */

   if(!subst)
   {
      subst = AllocEmptyDNormSubst();
   }

   if (varp(t))
   {
      if(!VTfind(subst->variables, t->fcode))
      {
	 VTadd(&(subst->variables), t->fcode, ++(subst->v_count));
      }
   }  
   else
   {
      if(!subst->norm_id[t->fcode])
      {
	 if(!NormTermPreserveArity)
	 {
	    subst->norm_id[t->fcode] = ++(subst->f_count);
	 }
	 else
	 {
	    subst->norm_id[t->fcode] = ++(subst->arity_id[t->arity]);
	 }
      }
      for(i=0; i<t->arity; i++)
      {
	 DNormTerm(t->argument[i], subst); 
      }
      
   }

/*   printf("...and returns\n");*/
   return subst;
}


/*-------------------------------------------------------------------------

FUNCTION         : long CmpNormTerms(term* t1, DNormSubst_p subst1,
                                     term* t2, DNormSubst_p subst2) 

Beschreibung     : Vergleicht zwei genormte DISCOUNT-Terme.
                   Rueckgabewert ist >0, =0, <0 wie bei strcmp.

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 25.11.1994 neu

-------------------------------------------------------------------------*/

long CmpNormTerms(term* t1, DNormSubst_p subst1, term* t2,
		  DNormSubst_p subst2) 
{
   long res, f;

   if((res = (t1->weight - t2->weight)))
   {
      return res;
   }
   else if(varp(t1)) /* varpt(t2) ensured by the equality of the weights! */
   {
      return VTfind(subst1->variables, t1->fcode) -
	 VTfind(subst2->variables, t2->fcode);
   }
   else if(NormTermPreserveArity && (res=(t1->arity-t2->arity)))
   {
      return res;
   }
   else if((res=subst1->norm_id[t1->fcode] -
	    subst2->norm_id[t2->fcode])) 
   {
      return res;
   }
   
   for(f=0; f< min(t1->arity, t2->arity); f++)
   {
      if((res = CmpNormTerms(t1->argument[f], subst1,
			     t2->argument[f], subst2)))
      {
	 return res;
      }
   }
   if((res = t1->arity - t2->arity))
   {
      return res;
   }
   return 0;
}



/*-------------------------------------------------------------------------

FUNCTION         : DnormSubst_p OrderNormTerms(**term t1, **term t2) 

Beschreibung     : Norms the two terms, compares them, orders them and
                   renorm the larger one, so that both together can be
		   considered as ordered normpair. Return value is the
		   DNormSubst computed, but the reordering of the terms
		   is just as important.

Globale Variable : -

Seiteneffekte    : Memory operations (for the DNormSubsts),
                   potentially exchanging of the two terms.

Aenderungen      : <1> 18.12.1994 neu
                   <2>   4.1.1995 Renamed, moved to learn_nterms.c

-------------------------------------------------------------------------*/

DNormSubst_p OrderNormTerms(term **t1, term **t2)
{
   DNormSubst_p subst1, subst2;
   term         *help;

   subst1 = DNormTerm(*t1, NULL);
   subst2 = DNormTerm(*t2, NULL);
   
   if(CmpNormTerms(*t1, subst1, *t2, subst2) > 0)
   {
      FreeDNormSubst(subst1);
      subst1 = subst2;
      help = *t1;
      *t1 = *t2;
      *t2 = help;
   }
   else
   {
      FreeDNormSubst(subst2);
   }
   subst1 = DNormTerm(*t2, subst1);

   return subst1;
}
      

/*-------------------------------------------------------------------------

FUNCTION         : void PrintDNormSubst(DNormSubst_p subst)

Beschreibung     : Ausgabe einer DNormSubst, nur zu Testzwecken.

Globale Variable : -

Seiteneffekte    : Ausgabe

Aenderungen      : <1> 24.11.1994 neu

-------------------------------------------------------------------------*/

void PrintDNormSubst(DNormSubst_p subst)
{
   short i;
   
   printf("---------- PrintDNormSubst(%d)-----\n", (int)subst);
   for(i = 1; i <= FuncCount; i++ )
   {
      printf ( "    %-26s = %d = %d_%d\n", Function[i].ident, (int)i,
	       (int)Function[i].arity,(int)subst->norm_id[i]);
   }
   printf("f_count = %d \n", (int)subst->f_count);
   printf("Variables:\n\n");
   VTprint(subst->variables);
   printf("v_count = %d \n", (int)subst->v_count);
   printf("-----------------------------------\n\n");
}
   



/*-------------------------------------------------------------------------

FUNCTION         : void PrintDNormTerm(term* t, DNormSubst_p subst)

Beschreibung     : Gibt einen genormten DISCOUNT-Term aus.

Globale Variable : -

Seiteneffekte    : Ausgabe

Aenderungen      : <1> 25.11.1994 neu

-------------------------------------------------------------------------*/

void PrintDNormTerm(term* t, DNormSubst_p subst)
{
   short i;

   if (varp(t))
   {
      printf("x%ld", VTfind(subst->variables, t->fcode));
   }  
   else
   {
      if(NormTermPreserveArity)
      {
	 printf("f%d_%d(", t->arity,subst->norm_id[t->fcode]);
      }
      else
      {
	 printf("f%d(", subst->norm_id[t->fcode]);
      }
      if(t->arity)
      {
	 PrintDNormTerm(t->argument[0], subst); 
      }
      for(i=1; i<t->arity; i++)
      {
	 printf(",");
	 PrintDNormTerm(t->argument[i], subst); 
      }
      printf(")");
   }
}




/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/


