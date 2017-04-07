/*-------------------------------------------------------------------------

File        : lrn_domains.h

Autor       : Stephan Schulz

Inhalt      : Deklarationen zum Umgang mit Domaenen und Brains.

Aenderungen : <1> 30.8.1994 neu (uebernimmt Teile aus dem obsoleten
                            lrn_makedom.h) 

-------------------------------------------------------------------------*/

#ifndef _lrn_domains

#define _lrn_domains

#include "lrn_eqntrees.h"
#include "lrn_sigs.h"



/*-----------------------------------------------------------------------*/
/*                                 Macros                                */
/*-----------------------------------------------------------------------*/

#define BrainsCompatible(brain1,brain2)\
((brain1)->type == (brain2)->type)


/*-----------------------------------------------------------------------*/
/*                   Typdeklarationen                                    */
/*-----------------------------------------------------------------------*/



typedef enum domtype
{
   SpecDom,
   GoalDom
}DomType;

/* Der folgende Datentyp beschreibt alle Dom"anen. Bei echten     */
/* Spezifikationsdom"anen bleibt "results" NULL, bei Zieldom"anen */
/* bleibt "axioms" NULL.                                          */

typedef struct domcell
{
   char*      name;
   DomType    type;
   Symbol_p   sig;
   EqnOccur_p examples;  /* Hier interessieren nur die Namen */
   NormEqn_p  axioms;
   NormEqn_p  theorems;
   NormEqn_p  lemmas;
   NormEqn_p  facts;
   struct domcell *next;
   struct domcell *prev; 
}DomCell, *Dom_p;


/* Neben den Dom"anen (die an Ziel oder Spezifikation gebunden werden) */
/* gibt es auch die "Brains", in denen NUR die Termpaare stehen - der  */
/* Beweiser lernt hier also universell gute Termpaare!                 */

typedef enum braintype
{
   Complete,     /* ALLE Termpaare */
   Pruned        /* Nur Lemmata    */
}BrainType;


typedef struct braincell
{
   char*      name;
   BrainType  type;
   EqnOccur_p examples;  /* Hier interessieren nur die Namen */
   NormEqn_p  facts;
}BrainCell, *Brain_p;




/*-----------------------------------------------------------------------*/
/*                 Deklaration exportierter Funktionen                   */
/*-----------------------------------------------------------------------*/


Dom_p       AllocDomCell();
void        FreeDomCell(Dom_p junk);
void        FreeDom(Dom_p junk);
void        FreeDomList(Dom_p junk);

Brain_p     AllocBrainCell();
void        FreeBrainCell(Brain_p junk);
void        FreeBrain(Brain_p junk);

NormSubst_p DomEquivSubst(Dom_p dom1, Dom_p dom2);

Dom_p       MergeDoms(Dom_p dom1, Dom_p dom2, NormSubst_p make_equal);
Brain_p     MergeBrains(Brain_p brain1, Brain_p brain2);

BOOL        IsUnusedDomName(char* name, Dom_p domlist);
BOOL        IsUnusedExampleName(char* name, Brain_p brain);

void        PrintDomHead(Dom_p prt);
void        PrintDomHeadList(Dom_p prt);
void        PrintDomBody(Dom_p prt);
void        PrintDom(Dom_p prt);
void        PrintBrain(Brain_p prt);


#endif


/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/





