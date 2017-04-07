/*-------------------------------------------------------------------------

File        : lrn_eqntrees.c

Autor       : Stephan Schulz

Inhalt      : Funktionen zum Umgang mit AVL-Baeumen ueber NormEqns. 

Aenderungen : <1> 10.8.1994 neu

-------------------------------------------------------------------------*/

#include "lrn_eqntrees.h"


/*-----------------------------------------------------------------------*/
/*           Forward-Deklaration interner Funktionen                     */
/*-----------------------------------------------------------------------*/

BOOL print_tree_level(NormEqn_p root, long level);


/*-----------------------------------------------------------------------*/
/*                     Interne Funktionen                                */
/*-----------------------------------------------------------------------*/


/*-------------------------------------------------------------------------

FUNCTION         : BOOL print_tree_level(NormEqn_p root, long level)

Beschreibung     : Gibt einen "Level" des Baumes aus (die Wurel, d.h.
                   der erste Eintrag, hat Stufe 0). Rueckgabewert ist
		   TRUE, falls eine Knoten auf der Stufe existiert,
		   FALSE sonst.

Globale Variable : out

Seiteneffekte    : Ausgabe

Aenderungen      : <1> 12.10.1994 neu

-------------------------------------------------------------------------*/

BOOL print_tree_level(NormEqn_p root, long level)
{
   if(root)
   {
      if(!level)
      {
	 PrintNormEqnLine(root);
	 return TRUE;
      }
      else
      {
	 return print_tree_level(root->left, level-1) |
	    print_tree_level(root->right, level-1);
      }
   }
   return FALSE;
}
	 



/*-----------------------------------------------------------------------*/
/*                  Exportierte Funktionen                               */
/*-----------------------------------------------------------------------*/

/*-------------------------------------------------------------------------

FUNCTION         : void FreeEqnTree(NormEqn_p junk)

Beschreibung     : Gibt den Baum, auf den junk zeigt, frei.

Globale Variable : -

Seiteneffekte    : Speicheroperationen

Aenderungen      : <1> 10.8.1994 neu

-------------------------------------------------------------------------*/

void FreeEqnTree(NormEqn_p junk)
{
   if(junk)
   {
      FreeEqnTree(junk->left);
      FreeEqnTree(junk->right);
      FreeNormEqnCell(junk);
   }
}


/*-------------------------------------------------------------------------

FUNCTION         : BOOL TreeInsertEqn(NormEqn_p *root, NormEqn_p eqn)

Beschreibung     : Fuegt einen Eintrag in den Baum bei *root ein. Im
                   Moment wird nur eine bin"arer Suchbaum aufgebaut -
		   spaeter wird daraus ein AVL-Baum. 
		   Rueckgabewert ist TRUE, wenn 1

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 10.8.1994 neu 

-------------------------------------------------------------------------*/

BOOL TreeInsertEqn(NormEqn_p *root, NormEqn_p eqn)
{
   long cmp_res;
   NormEqn_p subtree;

   if(!(*root))
   {
      *root = eqn;
      eqn->left = eqn->right = NULL;
      eqn->balance = 0;
      return TRUE;
   }
   else
   {
      cmp_res = CmpNormEqns(*root, eqn);
      
      if(cmp_res > 0)
      {
	 if(TreeInsertEqn(&((*root)->left), eqn))
	 {
	    (*root)->balance--;
	    if((*root)->balance == -2)
	    {
	       if((*root)->left->balance == -1)
	       { /* Einfache Rotation R */
		  subtree = (*root)->left;
		  (*root)->left = subtree->right;
		  (*root)->balance = 0;
		  subtree->right = (*root);
		  subtree->balance = 0;
		  *root = subtree;
	       }
	       else
	       { /* Doppelte Rotation */
		  subtree =  (*root)->left->right;
		  (*root)->left->right = subtree->left;
		  (*root)->left->balance = 
		     subtree->balance == -1 ? 1 : 0;
		  subtree->left = (*root)->left;
		  (*root)->left = subtree;
		  
		  subtree = (*root)->left;
		  (*root)->left = subtree->right;
		  (*root)->balance = subtree->balance == 1 ? -1 : 0;
		  subtree->right = *root;
		  subtree->balance = 0;
		  *root = subtree;
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
	 if(TreeInsertEqn(&((*root)->right), eqn))
	 {
	    (*root)->balance++;
	    if((*root)->balance == 2)
	    {
	       if((*root)->right->balance == 1)
	       { /* Einfache Rotation L */
		  subtree = (*root)->right;
		  (*root)->right = subtree->left;
		  (*root)->balance = 0;
		  subtree->left = (*root);
		  subtree->balance = 0;
		  *root = subtree;
	       }
	       else
	       { /* Doppelte Rotation */
		  subtree =  (*root)->right->left;
		  (*root)->right->left = subtree->right;
		  (*root)->right->balance = 
		     subtree->balance == 1 ? -1 : 0;
		  subtree->right = (*root)->right;
		  (*root)->right = subtree;
		  
		  subtree = (*root)->right;
		  (*root)->right = subtree->left;
		  (*root)->balance = subtree->balance == -11 ? 1 : 0;
		  subtree->left = *root;
		  subtree->balance = 0;
		  *root = subtree;
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
	 (*root)->occur =
	    MergeEqnOccurLists((*root)->occur,eqn->occur, FALSE);
	 eqn->occur = NULL;
	 (*root)->tot_ref += eqn->tot_ref;
	 (*root)->ave_ref = AverageReferences((*root)->occur);
	 (*root)->goal_dist = AverageGoalDist((*root)->occur);
	 FreeNormEqn(eqn);
	 return FALSE;
      }
   }
}


/*-------------------------------------------------------------------------

FUNCTION         : NormEqn_p TreeFindEqn(NormEqn_p eqn, NormEqn_p root)

Beschreibung     : Sucht den zur Gleichung Eqn gehoerenden Eintrag in
                   dem Baum, auf den root zeigt.

Globale Variable : -

Seiteneffekte    : -

Aenderungen      :  <1> 10.8.1994 neu 

-------------------------------------------------------------------------*/

NormEqn_p TreeFindEqn(NormEqn_p eqn, NormEqn_p root)
{
   long cmp_res;

   if(!root)
   {
      return NULL;
   }
   cmp_res = CmpNormEqns(root, eqn);
   
   if(cmp_res > 0)
   {
      return TreeFindEqn(eqn, root->left);
   }
   else if(cmp_res < 0)
   {
      return TreeFindEqn(eqn, root->right);
   }
   else
   {
      return root;
   }
} 


/*-------------------------------------------------------------------------

FUNCTION         : NormEqn_p EqnListToTree(NormEqn_p* tree, NormEqn_p list)

Beschreibung     : Fuegt die Eqns aus der Liste in den Baum bei *root
                   ein. Die Liste wird dabei zerstoert.

Globale Variable : -

Seiteneffekte    : Liste wird zerlegt, Speicheroperationen

Aenderungen      : <1> 11.8.1994 neu

-------------------------------------------------------------------------*/

NormEqn_p EqnListToTree(NormEqn_p* tree, NormEqn_p list)
{
   NormEqn_p handle, 
             help;

   handle = list->right;
   
   while(handle!=list)
   {
      help = handle->right;
      TreeInsertEqn(tree, handle);
      handle = help;
   }
   FreeNormEqnCell(list);

   return *tree;
}


/*-------------------------------------------------------------------------

FUNCTION         : NormEqn_p MergeEqnTrees(NormEqn_p* tree, 
                                           NormEqn_p new) 

Beschreibung     : Fuegt die Daten aus new in den Baum bei *tree ein.
                   Rueckgabewert ist *tree.

Globale Variable : -

Seiteneffekte    : Die alten Baeume werden verandert bzw. zerstoert.

Aenderungen      : <1> 29.8.1994 neu

-------------------------------------------------------------------------*/

NormEqn_p MergeEqnTrees(NormEqn_p* tree, NormEqn_p new)
{
   if(new)
   {
      MergeEqnTrees(tree, new->left);
      MergeEqnTrees(tree, new->right);
      TreeInsertEqn(tree,new);
   }
   return *tree;
}
      

/*-------------------------------------------------------------------------

FUNCTION         : void PartNormEqnTree(NormEqn_p tree, 
                                        NormSubst_p subst)

Beschreibung     : Normiert die Termpaare in dem Baum einzeln, aber
                   unter Ber"ucksichtigung der in subst bereits
		   vorgenommenen (Funktionssymbol-) Bindungen. Im
		   Regelfall sollten die Termpaare schon gerichtet
		   sein. 

Globale Variable : -

Seiteneffekte    : Speicheroperationen, Normung

Aenderungen      : <1> 6.9.1994 neu

-------------------------------------------------------------------------*/

void PartNormEqnTree(NormEqn_p tree, NormSubst_p subst)
{
   if(tree)
   {
      PartNormEqnTree(tree->left, subst);
      PartNormEqnTree(tree->right, subst);
            
      tree->lside->chain = tree->rside;
      tree->rside->chain = NULL;
      
      FreeNormSubst(FunNormTermList(tree->lside, 
				    CopyNormSubst(subst), TRUE));
      FreeNormSubst(VarNormTermList(tree->lside, NULL, TRUE));
      tree->lside->chain = NULL;
   }
}


/*-------------------------------------------------------------------------

FUNCTION         : void PrintEqnTree(NormEqn_p root)

Beschreibung     : Gibt einen EqnTree (geordnet) aus.

Globale Variable : out

Seiteneffekte    : Ausgabe

Aenderungen      : <1> 12.8.1994 neu

-------------------------------------------------------------------------*/

void PrintEqnTree(NormEqn_p root)
{
   if(root)
   {
      PrintEqnTree(root->left);
      PrintNormEqnLine(root);
      PrintEqnTree(root->right);
   }
}



/*-------------------------------------------------------------------------

FUNCTION         : void PrintOptEqnTree(NormEqn_p root)

Beschreibung     : Gibt einen Baum so aus, da"s beim normalen Einbau
                   in einen Bin"ar-Baum wieder derselbe Baum entsteht
		   - insbesondere bleibt die AVL-Eigenschaft auch ohne
		   AVL-Operationen erhalten.

Globale Variable : out

Seiteneffekte    : Ausgabe

Aenderungen      : <1> 12.10.1994 neu

-------------------------------------------------------------------------*/

void PrintOptEqnTree(NormEqn_p root)
{
   long level;

   for(level = 0; print_tree_level(root, level); level++);
}




/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/


