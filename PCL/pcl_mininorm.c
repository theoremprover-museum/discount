/*-----------------------------------------------------------------------

File  : pcl_mininorm.c

Author: Stephan Schulz

Contents
 
  Funktionen zum "normalisieren von PCL-Listings, also zum Entfernen
  von reinen "cite"-Expressions und zum Entfernen von duplizierten
  Ergebnissen.

Changes

<1>  2.3.1994 neu
<2> 30.9.1996 Normale Baeume durch AVL-Baeume ersetzt (Nur f"ur
              NumListTrees), teilweise neue Comment-Boxen 

-----------------------------------------------------------------------*/


#include "pcl_mininorm.h"


/*---------------------------------------------------------------------*/
/*                        Global Variables                             */
/*---------------------------------------------------------------------*/

BOOL Normalize  = FALSE;
BOOL Normalize2 = FALSE;

/*---------------------------------------------------------------------*/
/*                      Forward Declarations                           */
/*---------------------------------------------------------------------*/

StepType max_step_type(StepType first, StepType last);


/*---------------------------------------------------------------------*/
/*                         Internal Functions                          */
/*---------------------------------------------------------------------*/


/*-------------------------------------------------------------------------

FUNCTION         : StepType max_step_type(StepType first, StepType last)

Beschreibung     : Gibt zu zwei Typen den "groesseren" an. Die Ordnung
                   dabei ist 
		   tes_final > tes_lemma > tes_intermed > tes_eqn = tes_rule
		   tes_final > tes_intermedgoal > tes_goal
		   tes-final > crit_intermedgoal > crit_goal
		   Ist der Vergleich nicht zu entscheiden, so ist die
		   Antwort NoPair.

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 4.3.1994 neu

-------------------------------------------------------------------------*/

StepType max_step_type(StepType first, StepType last)
{
   switch(first)
   {
   case tes_final: 
      return tes_final;
      break;
   case tes_lemma: 
      if(last==tes_final)
      {
	 return last;
      }
      if(last==tes_lemma||last==tes_intermed||last==tes_eqn||last==tes_rule)
      {
	 return first;
      }
      break;
   case tes_intermed:
      if(last==tes_final||last==tes_lemma)
      {
	 return last;
      }
      if(last==tes_intermed||last==tes_eqn||last==tes_rule)
      {
	 return first;
      }
      break;
   case tes_eqn:
   case tes_rule:
      if(last==tes_final||last==tes_lemma||last==tes_intermed)
      {
	 return last;
      }
      if(last==tes_eqn||last==tes_rule)
      {
	 return first;
      }
      break;
   case tes_intermedgoal:
      if(last==tes_final)
      {
	 return last;
      }
      if(last==tes_intermedgoal||last==tes_goal)
      {
	 return first;
      }
      break;
   case tes_goal:
      if(last==tes_final||last==tes_intermedgoal)
      {
	 return last;
      }
      if(last==tes_goal)
      {
	 return first;
      }
      break;
   case crit_intermedgoal:
      if(last==tes_final)
      {
	 return last;
      }
      if(last==crit_intermedgoal||last==crit_goal)
      {
	 return first;
      }
      break;
   case crit_goal:
      if(last==tes_final||last==crit_intermedgoal)
      {
	 return last;
      }
      if(last==crit_goal)
      {
	 return first;
      }
      break;
   default: 
      Error("Illegal StepType in max_step_type()");
      break;
   }
   return NoPair;
}

/*---------------------------------------------------------------------*/
/*           Exported Functions: Memory Handling                       */
/*---------------------------------------------------------------------*/



MakeAlloc(NumListTreeCell);
MakeFree(NumListTreeCell);

MakeAlloc(StringTreeCell);
MakeFree(StringTreeCell);


/*-------------------------------------------------------------------------

FUNCTION         : void FreeNumListTree(NumListTree_p junk)

Beschreibung     : Gibt den von einem NumList-Baum benutzten Speicher
                   zurueck, nicht aber die Listen selbst (die Pointer
		   sind nur Kopien...). Baeume haben keinen Anker (bzw
		   einen Anker von anderem Typ...). 

Globale Variable : -

Seiteneffekte    : Speicheroperationen

Aenderungen      : <1> 1.3.1994 neu

-------------------------------------------------------------------------*/

void FreeNumListTree(NumListTree_p junk)
{
   if(junk)
   {
      FreeNumListTree(junk->left);
      FreeNumListTree(junk->right);
      junk->key = NULL;
      junk->value = NULL; /* This is only a 2nd reference! */
      FreeNumListTreeCell(junk);
   }
}


/*-------------------------------------------------------------------------

FUNCTION         : void FreeStringTree(StringTree_p junk)

Beschreibung     : Gibt den von einem String-Baum benutzten Speicher
                   zurueck, nicht die Strings selbst (die Pointer sind
		   nur Kopien). Baeume haben keinen Anker (bzw einen
		   Anker von anderem Typ...).

Globale Variable : -

Seiteneffekte    : Speicheroperationen

Aenderungen      : <1> 8.3.1994 neu

-------------------------------------------------------------------------*/

void FreeStringTree(StringTree_p junk)
{
   if(junk)
   {
      FreeStringTree(junk->left);
      FreeStringTree(junk->right);
      junk->key = 0;
      junk->value = NULL; /* This is only a 2nd reference! */
      FreeStringTreeCell(junk);
   }
}



/*---------------------------------------------------------------------*/
/*          Exported Functions: Operation on NumListTrees              */
/*---------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: NumListTreeInsert()
//
//   Inserts the Cell *new into the AVL Tree at anchor. Return value
//   is TRUE if the tree height increased, FALSE otherwise.
//
// Global Variables: -
//
// Side Effects    : Changes the tree, sets the child pointers in *new
//                   to NULL
//
/----------------------------------------------------------------------*/

BOOL NumListTreeInsert(NumListTree_p *anchor, NumListTree_p new)
{
   long          cmp_res;
   NumListTree_p subtree;

   if(!(*anchor))
   {
      *anchor = new;
      new->left = new->right = NULL;
      new->balance = 0;
      return TRUE;
   }
    else
   {
      cmp_res = CmpNumList((*anchor)->key, new->key);
      
      if(cmp_res > 0)
      {
         if(NumListTreeInsert(&((*anchor)->left), new))
         {
            (*anchor)->balance--;
            if((*anchor)->balance == -2)
            {
               if((*anchor)->left->balance == -1)
               { /* Einfache Rotation R */
                  subtree = (*anchor)->left;
                  (*anchor)->left = subtree->right;
                  (*anchor)->balance = 0;
                  subtree->right = (*anchor);
                  subtree->balance = 0;
                  *anchor = subtree;
               }
               else
               { /* Doppelte Rotation */
                  subtree =  (*anchor)->left->right;
                  (*anchor)->left->right = subtree->left;
                  (*anchor)->left->balance = 
                     subtree->balance == -1 ? 1 : 0;
                  subtree->left = (*anchor)->left;
                  (*anchor)->left = subtree;
                  
                  subtree = (*anchor)->left;
                  (*anchor)->left = subtree->right;
                  (*anchor)->balance = subtree->balance == 1 ? -1 : 0;
                  subtree->right = *anchor;
                  subtree->balance = 0;
                  *anchor = subtree;
               }
               return FALSE;
            }
            else
            {
               return TRUE;
            }
         }
 
         return FALSE;
      }
      else if(cmp_res < 0)
      {
         if(NumListTreeInsert(&((*anchor)->right), new))
         {
            (*anchor)->balance++;
            if((*anchor)->balance == 2)
            {
               if((*anchor)->right->balance == 1)
               { /* Einfache Rotation L */
                  subtree = (*anchor)->right;
                  (*anchor)->right = subtree->left;
                  (*anchor)->balance = 0;
                  subtree->left = (*anchor);
                  subtree->balance = 0;
                  *anchor = subtree;
               }
               else
               { /* Doppelte Rotation */
                  subtree =  (*anchor)->right->left;
                  (*anchor)->right->left = subtree->right;
                  (*anchor)->right->balance = 
                     subtree->balance == 1 ? -1 : 0;
                  subtree->right = (*anchor)->right;
                  (*anchor)->right = subtree;
                  
                  subtree = (*anchor)->right;
                  (*anchor)->right = subtree->left;
                  (*anchor)->balance = subtree->balance == -11 ? 1 : 0;
                  subtree->left = *anchor;
                  subtree->balance = 0;
                  *anchor = subtree;
               }
               return FALSE;
            }
            else
            {
               return TRUE;
            }
         }
         return FALSE;
      }
      else
      {
	 Error("Double identifier in NumListTreeInsert()");
         return FALSE;
      }
   }
}


/*-------------------------------------------------------------------------

FUNCTION         : NumListTree_p NumListTreeFind(NumListTree_p anchor,
                                                 NumList_p search)

Beschreibung     : Suche den Key <search> in dem Baum, Rueckgabe ist
                   Pointer auf die NumListTreeCell mit key == search oder
		   NULL, falls eine solche Zelle nicht existiert.

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 2.3.1993 neu

-------------------------------------------------------------------------*/

NumListTree_p NumListTreeFind(NumListTree_p anchor, NumList_p search)
{
   long cmp_res;

   if(anchor)
   {
      cmp_res = CmpNumList(anchor->key, search);
      if(cmp_res>0)
      {
	 return NumListTreeFind(anchor->left, search);
      }
      else if(cmp_res<0)
      {
	 return NumListTreeFind(anchor->right, search);
      }
      else
      {
	 return anchor;
      }
   }
   else
   {
      return 0;
   }
}
	

/*-------------------------------------------------------------------------

FUNCTION         : void PrintNumListTree(NumListTree_p anchor)

Beschreibung     : Gib den Baum in Vorordnung aus - sowohl key wie auch
                   value. Dient nur zu Debug-Zwecken...

Globale Variable : -

Seiteneffekte    : Augabe

Aenderungen      : <1> 2.3.1993 neu

-------------------------------------------------------------------------*/

void PrintNumListTree(NumListTree_p anchor)
{
   if(anchor)
   {
      PrintNumListTree(anchor->left);
      fprintf(out,"Key: ");
      PrintNumList(anchor->key);
      fprintf(out,"   Value:\n");
      PrintStep(anchor->value);
      PrintNumListTree(anchor->right);
   }
}


/*---------------------------------------------------------------------*/
/*          Exported Functions: Operation on StringTrees              */
/*---------------------------------------------------------------------*/


/*-------------------------------------------------------------------------

FUNCTION         : void StringTreeInsert(StringTree_p *anchor, StringTree_p new)

Beschreibung     : Fuege die Zelle *new in den Baum ein, der bei
                   *anchor beginnt. Wenn der Schluessel bereits
		   enthalten ist, so erzeuge einen Fehler...

Globale Variable : -

Seiteneffekte    : Veraenderung des Baumes und der Pointer in *new

Aenderungen      : <1> 2.3.1993 neu

-------------------------------------------------------------------------*/

void StringTreeInsert(StringTree_p *anchor, StringTree_p new)
{
   long cmp_res;

   if(*anchor)
   {
      cmp_res = strcmp((*anchor)->key, new->key);
      if(cmp_res>0)
      {
	 StringTreeInsert(&((*anchor)->left), new);
      }
      else if(cmp_res<0)
      {
	 StringTreeInsert(&((*anchor)->right), new);
      }
      else
      {
	 Error("Double identifier in StringTreeInsert()");
      }
   }
   else
   {
      new->left = NULL;
      new->right = NULL;
      *anchor = new;
   }
}
	 

/*-------------------------------------------------------------------------

FUNCTION         : StringTree_p StringTreeFind(StringTree_p anchor,
                                               char* search)

Beschreibung     : Suche den Key <search> in dem Baum, Rueckgabe ist
                   Pointer auf die StringTreeCell mit key == search oder
		   NULL, falls eine solche Zelle nicht existiert.

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 2.3.1993 neu

-------------------------------------------------------------------------*/

StringTree_p StringTreeFind(StringTree_p anchor, char* search)
{
   long cmp_res;

   if(anchor)
   {
      cmp_res = strcmp(anchor->key, search);
      if(cmp_res>0)
      {
	 return StringTreeFind(anchor->left, search);
      }
      else if(cmp_res<0)
      {
	 return StringTreeFind(anchor->right, search);
      }
      else
      {
	 return anchor;
      }
   }
   else
   {
      return 0;
   }
}
	

/*-------------------------------------------------------------------------

FUNCTION         : void PrintStringTree(StringTree_p anchor)

Beschreibung     : Gib den Baum in Vorordnung aus - sowohl key wie auch
                   value. Dient nur zu Debug-Zwecken...

Globale Variable : -

Seiteneffekte    : Augabe

Aenderungen      : <1> 2.3.1993 neu

-------------------------------------------------------------------------*/

void PrintStringTree(StringTree_p anchor)
{
   if(anchor)
   {
      PrintStringTree(anchor->left);
      fprintf(out,"Key: %s",anchor->key);
      fprintf(out,"   Value:\n");
      PrintStep(anchor->value);
      PrintStringTree(anchor->right);
   }
}



/*---------------------------------------------------------------------*/
/*                 Exported Functions: All the Rest                    */
/*---------------------------------------------------------------------*/



/*-------------------------------------------------------------------------

FUNCTION         : miniJust_p RenumJust(miniJust_p just, NumListTree_p data)

Beschreibung     : Ersetzt in just alle Vorkommen von Identifiern, die
                   in dem Baum enthalten sind, durch die Identifier
		   des assoziierten PCL-Schrittes.

Globale Variable : -

Seiteneffekte    : Veraenderung der Justifikation

Aenderungen      : <1> 2.3.1994 neu

-------------------------------------------------------------------------*/

miniJust_p RenumJust(miniJust_p just, NumListTree_p data)
{
   NumListTree_p assoc;

   switch(just->operation)
   {
      case initial:
      case hypothesis:
                       break;
      case orientx:
      case orientu:    
                       RenumJust(just->arg1.rarg,data);
                       break;
      case cp:
      case tes_red:
      case instance:
                       RenumJust(just->arg1.rarg,data);
                       RenumJust(just->arg2.rarg,data);
		       break;
      case quotation:  if((assoc = NumListTreeFind(data, just->arg1.targ)))
                       {
			  FreeNumList(just->arg1.targ);
			  just->arg1.targ = CopyNumList(assoc->value->id);
		       }
                       break;
      default:         Error("Illegal Operation in PCL-Justification");
                       break;
   }
   return just;
}


/*-------------------------------------------------------------------------

FUNCTION         : miniStep_p FindStepById(NumLisp_p id, 
                                           miniStep_p anchor,
					   miniStep_p final)

Beschreibung     : Suche in der PCL-miniStep-Liste bei anchor nach dem
                   Schritt mit dem Identifier id. Beginne am Anfang und
		   terminiere bei final. (Ist final == NULL, so
		   durchsuche die ganze Liste - intendiert, aber auch
		   automatischer Effekt). Wird kein passender Schritt
		   gefunden, so ist der Rueckgabewert NULL.

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 3.3.1994 neu

-------------------------------------------------------------------------*/

miniStep_p FindStepById(NumList_p id, miniStep_p anchor, miniStep_p final)
{
   miniStep_p result = NULL,
              handle;

   for(handle = anchor->succ; 
       (handle!=anchor) && (handle!=final) && (!result); 
       handle = handle->succ)
   {
      if(CmpNumList(id, handle->id)==0)
      {
	 result = handle;
      }

   }
   return result;
}
   



/*-------------------------------------------------------------------------

FUNCTION         : miniStep_p FindStepByIdBack(NumList_p id, 
                                               miniStep_p anchor, 
					       miniStep_p final)

Beschreibung     : Suche in der PCL-miniStep-Liste bei anchor nach dem
                   Schritt mit dem Identifier id. Beginne bei final, 
		   terminiere bei anchor. Wird kein passender Schritt
		   gefunden, so ist der Rueckgabewert NULL. Diese
		   Funktion ersetzt die bisherige Funktion
		   FindStepById(), die allerdings, als allgemeinere
		   Loesung, ebenfalls erhalten bleibt.

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 3.3.1994 neu

-------------------------------------------------------------------------*/

miniStep_p FindStepByIdBack(NumList_p id, miniStep_p anchor, miniStep_p final)
{
   miniStep_p result = NULL,
              handle;

   for(handle = final->pred;
       (handle!=anchor) && (!result); 
       handle = handle->pred)
   {
      if(CmpNumList(id, handle->id)==0)
      {
	 result = handle;
      }

   }
   return result;
}
   


/*-------------------------------------------------------------------------

FUNCTION         : long NormalizeProof(miniStep_p anchor)

Beschreibung     : Entfernt reine quote-Expressions aus dem Beweis.
                   Genauer: Veraendere alle Referenzen so, da"s sie
		   auf den jeweils ersten Schritt mit einem gegebenen
		   Fakt zeigen. Das tataechliche Entfernen uebernehmen
		   die Extract-Funktion. Redundante Schritte werden
		   als trivial markiert, Rueckgabewert ist die Anzahl
		   der eliminierten Redundanzen. 

Globale Variable : -

Seiteneffekte    : Veraenderung des Beweises bei anchor.

Aenderungen      : <1> 4.3.1994 new

-------------------------------------------------------------------------*/

long NormalizeProof(miniStep_p anchor)
{
   miniStep_p    handle,
                 primary;
   NumListTree_p renum_data = NULL, 
                 new_renum;
   StepType      max_type;
   long          done = 0;


   handle = anchor->succ;

   while(handle != anchor)
   {
      RenumJust(handle->just,renum_data);
      if(handle->just->operation==quotation)
      {
	 primary = FindStepByIdBack(handle->just->arg1.targ, anchor,
				    handle);
	 if(primary)
	 {
	    new_renum = AllocNumListTreeCell();
	    new_renum->key = handle->id;
	    new_renum->value = primary;
	    NumListTreeInsert(&renum_data, new_renum);
	    max_type = max_step_type(primary->type,handle->type);
	    if(max_type == NoPair)
	    {
	       Error("Incompatible types in NormalizeProof()");
	    }
	    handle->type = primary->type; /* Swap larger type to the */
	    primary->type = max_type;     /* first occurence...      */
	    handle->trivial = 1;
	    done++;
	 }
      }
      handle = handle->succ;
   }
   FreeNumListTree(renum_data);
   return done;
}
	    



/*-------------------------------------------------------------------------

FUNCTION         : long StrongNormalizeProof(miniStep_p anchor)

Beschreibung     : Entfernt doppelte Herleitungen aus dem Beweis -
                   interessant nur fuer TEAMWORK (und fuer die
		   LevelExtraktion). Auch hier werden Redundanzen nur
		   mark=iert und Referenzen auf sie eleminitert
		   (vergleiche NormalizeProof). Rueckgabewert ist die
		   Anzahl der eliminierten Redundanzen. 

Globale Variable : -

Seiteneffekte    : Veraenderung des Beweises bei anchor.

Aenderungen      : <1> 8.3.1994 new

-------------------------------------------------------------------------*/

long StrongNormalizeProof(miniStep_p anchor)
{
   miniStep_p    handle;
   NumListTree_p renum_data = NULL,
                 new_renum;
   StringTree_p  goal_tree  = NULL,
                 crit_tree  = NULL,
                 final_tree = NULL,
                 fact_tree  = NULL,
                 new_fact,
                 primary,
                 *tree_to_use;
   StepType      max_type;
   long          done = 0;

   handle = anchor->succ;


   while(handle != anchor)
   {
      RenumJust(handle->just,renum_data);

      switch(handle->type)
      {
      case tes_intermedgoal:
      case tes_goal:
	 tree_to_use = &goal_tree;
	 break;
      case crit_intermedgoal:
      case crit_goal:
	 tree_to_use = &crit_tree;
	 break;
      case tes_final:
	 tree_to_use = &final_tree;
	 break;
      case tes_lemma:
      case tes_intermed:
      case tes_eqn:
      case tes_rule:
	 tree_to_use = &fact_tree;
	 break;
      default:
	 tree_to_use = 0;
	 Error("Illegal type in StrongNormalizeProof()");
	 break;
      }

      primary = StringTreeFind(*tree_to_use, handle->pair);

      if(primary)
      {
	 new_renum = AllocNumListTreeCell();
	 new_renum->key = handle->id;
	 new_renum->value = primary->value;
	 NumListTreeInsert(&renum_data, new_renum);
	 max_type = max_step_type(primary->value->type,handle->type);
	 if(max_type == NoPair)
	 {
	    Error("Incompatible types in StrongNormalizeProof()");
	 }
	 handle->type = primary->value->type; /* Swap larger type to the */
	 primary->value->type = max_type;     /* first occurence.... */
	 handle->trivial = 1;
	 done++;
      }
      else
      {
	 new_fact = AllocStringTreeCell();
	 new_fact->key = handle->pair;
	 new_fact->value = handle;
	 StringTreeInsert(tree_to_use,new_fact);
      }
      handle = handle->succ;
   }

   FreeStringTree(goal_tree);
   FreeNumListTree(renum_data);
   FreeStringTree(crit_tree);
   FreeStringTree(final_tree);
   FreeStringTree(fact_tree);
   return done;
}
	    


/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

