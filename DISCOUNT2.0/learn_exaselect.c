/*-------------------------------------------------------------------------

File        : learn_exaselect.c

Author      : Felix Brandt

Inhalt      : Functions for example selection.

Aenderungen : <1> 9.3.1998 neu

-------------------------------------------------------------------------*/

#include "learn_exaselect.h"

/*-----------------------------------------------------------------------*/
/*                         Globale Variable                              */
/*-----------------------------------------------------------------------*/

bool   no_exasel;
long   maxexamples;
double maxdelta;

double weightNA, weightAD, weightDD, weightGD, weightAF, weightTSM;

/*-----------------------------------------------------------------------*/
/*                     Interne Funktionen                                */
/*-----------------------------------------------------------------------*/

/*-------------------------------------------------------------------------

FUNCTION         : example_p InsertExample(exatree_p tree, example_p example,
                                           double delta)

Beschreibung     : Inserts example into bintree

Globale Variable : -

Seiteneffekte    : Example tree is changed

Aenderungen      : <1> 7.5.1998 new
                   <2> 30.6.1998 bintree

-------------------------------------------------------------------------*/

exampletree_p InsertExample(exampletree_p tree, example_p example, double delta)
{
   exampletree_p handle, newnode;
   handle = tree;

   newnode = AllocTreeCell();
   newnode->example = example;
   newnode->fitness = delta;
   newnode->left = newnode->right = NULL;

   if(!tree) {tree = newnode; return tree;}

   for(;;)
   {
      if(delta<=handle->fitness) 
      {
	 if(!(handle->left)) {handle->left = newnode; return tree;}
         handle = handle->left;
      }
      else 
      {
    	 if(!(handle->right)) {handle->right = newnode; return tree;}
         handle = handle->right;
      }
   }
}

/*-------------------------------------------------------------------------

FUNCTION         : example_p InOrder(exatree_p tree)

Beschreibung     : Scan tree inorder

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 30.6.1998 

-------------------------------------------------------------------------*/

example_p InOrder(exampletree_p tree, example_p list)
{
   if(!tree) return list;

   list = InOrder(tree->left, list);

   list->next = tree->example;
   list = tree->example;

   list = InOrder(tree->right, list);

   return list;
}

/*-------------------------------------------------------------------------

FUNCTION         : double DeltaAF(long arity1[], long arity2[], maxarity1,
                                  maxarity2)

Beschreibung     : Calculates DeltaAF

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 26.5.1998 new

-------------------------------------------------------------------------*/

double DeltaAF(long arity1[], long arity2[], long maxarity1, long maxarity2) 
{
   double res = 0;  
   long maxarity = max(maxarity1, maxarity2), i;
   for(i=0; i<=maxarity; i++)
   {
      if(i>maxarity1 || i>maxarity2) res+=1;
      else res += delta(arity1[i],arity2[i]);
   }
   return res/(maxarity+1);
}
   
/*-----------------------------------------------------------------------*/
/*                  Exportierte Funktionen                               */
/*-----------------------------------------------------------------------*/

/*-------------------------------------------------------------------------

FUNCTION         : example_p GetExaList(char* PathNameHelp, bool select)

Beschreibung     : Gets the list of "useful" examples.
                   up to maxexamples examples which fitness is less or equal
                   than maxfitness are selected if select is TRUE.

Globale Variable : -

Seiteneffekte    : By calling the scanner, Memory allocation

Aenderungen      : <1> 11.3.1998 new
                   <2> 5.8.1998 DeltaMIX

-------------------------------------------------------------------------*/

example_p GetExaList(char* PathNameHelp, bool select)

{
   example_p allexamples, goodexamples, exa_handle, example;   
   exampletree_p exatree;
   char examplepath[MAXPATHLEN];
   long i, d[MAXAXIOMS], fit = 0,
        axiomscount = 0, maxarity = ArityMax, arity[MAXARITY],  goaldepth,
        axiomscount2,    maxarity2,           arity2[MAXARITY], goaldepth2;   
   double averagedepth = 0, deviationdepth = 0,
          averagedepth2,    deviationdepth2,
          delta;
   termpair *handle;
   TSMDesc_p desc = NULL, desc2 = NULL;
   Lpair_p lp;
   TSMData data;
   
   for(i = 0; i<=MAXARITY; i++) arity[i]=0;

     desc  = AllocEmptyTSMDesc(StandardTSMLtermMap,
			     StandardTSMNtermMap);
   handle = SetOfAxioms.first;
   while(handle) 
   {
     lp = TermpairToNormLpair(handle);
     TSMInsertLterm(desc, &(desc->tsm), lp->lside, &data);
     TSMInsertLterm(desc, &(desc->tsm), lp->rside, &data);
     FreeLpair(lp);
     handle = handle->next;
   }

   /* Get Goal Depth */

   goaldepth = depth(SetOfGoals.first->left) + depth(SetOfGoals.first->right);

   /* Get Number of Axioms and Average Depth */
 
   handle = SetOfAxioms.first;
   i = 0;
   while(handle) 
   {
     axiomscount++;
     averagedepth += (d[i++] = depth(handle->left));
     averagedepth += (d[i++] = depth(handle->right));
     handle = handle->next;
   }
   averagedepth /= 2*axiomscount;

   /* Compute Depth Standard Deviation */

   for(i = 0; i < 2*axiomscount; i++) deviationdepth += 
				      (d[i]-averagedepth)*(d[i]-averagedepth);
   deviationdepth = sqrt(deviationdepth/(2*axiomscount));

   /* Count Arity Frequencies */

   for(i = 1; i <= FuncCount; i++) arity[Arity(i)]++;

   /*   printf("Axioms of current problem: %i\n",axiomscount);
   printf("Average Depth: %f\n",averagedepth);
   printf("Depth Standard Deviation: %f\n",deviationdepth);
   printf("Goal Depth: %ld\n",goaldepth);
   printf("Maximum Arity: %i\nArity Frequencies: ",maxarity);
   for(i = 0; i <= ArityMax; i++) printf("%i ",arity[i]);
   printf("\n"); */

   InitScanner(PathNameHelp);
   printf("Reading example names from %s.\n",
	  PathNameHelp);

   while(!(TestIdent(ident, "examples") && ColonFollows)) /* Skip junk */
   {
      NextRealToken();
      if(TestToken(NoToken))
      {
	 ScannerError("Unexpected end of file");
      }
   }

   AcceptIdent(ident, "examples");
   AcceptToken(colon);

   allexamples = AllocExampleCell();
   exa_handle = allexamples;

   if(!TestToken(string)) {FreeExampleCell(allexamples);allexamples = NULL;}
   while(TestToken(string))
   {
      exa_handle->name = SizeMalloc(strlen(AktLiteral)+1);
      strcpy(exa_handle->name, AktLiteral);
      NextToken();
      if(TestToken(comma))
      {
          exa_handle->next = AllocExampleCell();
          exa_handle = exa_handle->next;
          NextToken();
      }
      else exa_handle->next = NULL;
   }
   EndScanner();

   if(!select) return allexamples;

   printf("Reading example files from %s/SELECTIONDATA/.\n",KBFullName());

   exa_handle = allexamples;
   exatree = NULL;

   while(exa_handle)
   {
      strcpy(examplepath, KBFullName());
      strcat(examplepath, "/SELECTIONDATA/");
      strcat(examplepath, exa_handle->name);
      strcat(examplepath, ".sel");
      InitScanner(examplepath);
         
      AcceptIdent(ident, "number_of_axioms");
      AcceptToken(colon);
      axiomscount2 = AktNum;
      NextToken();

      AcceptIdent(ident, "average_depth");
      AcceptToken(colon);
      averagedepth2 = ParseReal();

      AcceptIdent(ident, "depth_standard_deviation");
      AcceptToken(colon);
      deviationdepth2 = ParseReal();

      AcceptIdent(ident, "goal_depth");
      AcceptToken(colon);
      goaldepth2 = AktNum;
      NextToken();

      AcceptIdent(ident, "max_arity");
      AcceptToken(colon);
      maxarity2 = AktNum;
      NextToken();
      /*printf("%s %ld %f %f %ld %ld : ",exa_handle->name,axiomscount2,averagedepth2, deviationdepth2,goaldepth2,maxarity2);*/
      AcceptIdent(ident, "arity_frequencies");
      AcceptToken(colon);
      for(i = 0; i <= maxarity2; i++)
      {
        arity2[i] = AktNum;
        NextToken();
	/*printf("%ld ",arity2[i]);*/
      }

      AcceptIdent(ident, "axioms");
      AcceptToken(colon);
      desc2 = AllocEmptyTSMDesc(StandardTSMLtermMap,
                                StandardTSMNtermMap);
      i = ParsePlainLpairTSM(desc2)/2;
      delta = ( weightNA*delta(axiomscount,axiomscount2) +
                weightAD*delta(averagedepth,averagedepth2) +
                weightDD*delta(deviationdepth,deviationdepth2) +
                weightGD*delta(goaldepth,goaldepth2) +
                weightAF*DeltaAF(arity,arity2,maxarity,maxarity2) +
                weightTSM*DeltaTSM(desc->tsm, desc2->tsm) )
             / (weightNA+weightAD+weightDD+weightGD+weightAF+weightTSM);
      FreeTSMDesc(desc2);

      EndScanner();

      /*printf("Delta: %f\n",delta);*/

      if(delta<=maxdelta) exatree = InsertExample(exatree, exa_handle, delta);
 
      exa_handle = exa_handle->next;
   }  

   goodexamples = AllocExampleCell();

   exa_handle = InOrder(exatree, goodexamples);
   exa_handle->next = NULL;

   exa_handle = goodexamples;
   goodexamples = goodexamples->next;
   FreeExampleCell(exa_handle);
   exa_handle = goodexamples;
   while(exa_handle)
   {
      fit++;
      if(fit>=maxexamples) exa_handle->next = NULL;
      exa_handle = exa_handle->next;
   }

   if(!maxexamples) 
   {
      goodexamples=NULL;
      printf("No Examples selected.\n");      
   }
   else
   {
      printf("%i fitting Example(s): ",fit);
      PrintExampleList(goodexamples);
   }

   return goodexamples;
}

/*-------------------------------------------------------------------------

FUNCTION         : bool IsEmptyIntersection(exmample_p list1, example_p list2)

Beschreibung     : TRUE if the example lists share at least one example.
                   FALSE otherwise.

Globale Variable : -

Seiteneffekte    : -

Aenderungen      : <1> 16.3.1998 new

-------------------------------------------------------------------------*/

bool IsEmptyIntersection(example_p list1, example_p list2)
{
   example_p handle;
   while(list1)
   {
      handle = list2;
      while(handle)
      {
         if (!strcmp(list1->name,handle->name)) return false;
         handle = handle->next;
      }
      list1 = list1->next;
   }
   return true;
}

/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/


