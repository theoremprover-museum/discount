/*-------------------------------------------------------------------------

File        : learn_specific.c

Autor       : Stephan Schulz

Inhalt      : Definitions for the goal- and specification dependend
              learning experts. Includes general infrastructure
              (global variables, space for filename construction...)

Aenderungen : <1> 6.1.1995 neu

-------------------------------------------------------------------------*/

#include "learn_specific.h"


/*-----------------------------------------------------------------------*/
/*                     Exportierte Variablen                             */
/*-----------------------------------------------------------------------*/


char          KnowledgeBase[MAXPATHLEN] = "";
char          TmpPathName[MAXPATHLEN];
char          PathNameHelp[MAXPATHLEN];


/*-----------------------------------------------------------------------*/
/*           Forward-Deklaration interner Funktionen                     */
/*-----------------------------------------------------------------------*/


LpairTreeParams read_goaldata(Lpair_p *root, char *name, bool complete);

LpairTreeParams read_specdata(Lpair_p *root, char *name, bool complete);

LpairTreeParams find_fist_compat_specdom(Lpair_p* knowledge_tree,
                                         LearnSig_p* sig, 
                                         DNormSubst_p* subst, 
                                         bool complete);
     
void find_rest_compat_specdom(Lpair_p* knowledge_tree, 
                              LearnSig_p sig, LpairTreeParams_p params,
                              bool complete); 


/*-----------------------------------------------------------------------*/
/*                     Interne Funktionen                                */
/*-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
//
// Function: read_goaldata()
//
//   Reads the pruned or complete knowledge tree from the named
//   goaldomain and inserts it into the tree at *root. Return value is
//   the struct of maximal values  in the tree as set by
//   ParseLpairTree(). 
//
// Global Variables: KnowledgeBase,  PathNameHelp
//
// Side Effects    : Changes the tree and the filename-variables.
//
/----------------------------------------------------------------------*/

LpairTreeParams read_goaldata(Lpair_p *root, char *name, bool complete)
{
   LpairTreeParams max_values = {0,0,0,0,0,0};

   strcpy(PathNameHelp, KBFullName());
   strcat(PathNameHelp, "/GOALDOMS/");
   strcat(PathNameHelp, name);
   strcat(PathNameHelp, ".gdm");
   
   InitScanner(PathNameHelp);
   printf("Reading goal-domain knowledge from %s.\n", PathNameHelp);
   
   while(!(TestIdent(ident, 
                     complete ? "facts" : "lemmas") 
           && ColonFollows)) /* Skip junk */
   {
      NextRealToken();
      if(TestToken(NoToken))
      {
         ScannerError("Unexpected end of file");
      }
   }
   
   AcceptIdent(ident, complete ? "facts" : "lemmas");
   AcceptToken(colon);

   ParseLpairTree(root, &max_values);
   
   EndScanner();
   printf("Knowledge read.\n");

   return max_values;
}


/*-----------------------------------------------------------------------
//
// Function: read_specdata()
//
//   Reads the pruned or complete knowledge tree from the named
//   specdomain and inserts it into the tree at *root. Return value is
//   the struct of maximal values  in the tree as set by
//   ParseLpairTree(). 
//
// Global Variables: KnowledgeBase, PathNameHelp
//
// Side Effects    : Changes the tree and the filename-variables.
//
/----------------------------------------------------------------------*/

LpairTreeParams read_specdata(Lpair_p *root, char *name, bool complete)
{
   LpairTreeParams max_values = {0,0,0,0,0,0};

   strcpy(PathNameHelp, KBFullName());
   strcat(PathNameHelp, "/SPECDOMS/");
   strcat(PathNameHelp, name);
   strcat(PathNameHelp, ".sdm");
   
   InitScanner(PathNameHelp);
   printf("Reading spec-domain knowledge from %s.\n", PathNameHelp);
   
   while(!(TestIdent(ident, 
                     complete ? "facts" : "lemmas") 
           && ColonFollows)) /* Skip junk */
   {
      NextRealToken();
      if(TestToken(NoToken))
      {
         ScannerError("Unexpected end of file");
      }
   }
   
   AcceptIdent(ident, complete ? "facts" : "lemmas");
   AcceptToken(colon);

   ParseLpairTree(root, &max_values);
   
   EndScanner();
   printf("Knowledge read.\n");

   return max_values;
}


/*-----------------------------------------------------------------------
//
// Function: find_fist_compat_specdom()
//
//   Searches for the first valid domain in the opened input
//   stream. If such a domain is found, the DNormSubst matching the
//   DISCOUNT sig onto the Learn-Sig is generated, the knowledge tree
//   is read and the signature is stored. Return value is the struct
//   of parameters desribing the knowledge read. If no domain is
//   found, all memory is freed and the return values are all 0.
//
//   Note that much of the results are returned in
//   DomainGlobalInfo[MAXDOM_ANZ-1], and not via the
//   I/O-parameters. This hack is inherited from MK's code!
//
// Global Variables: DomainGlobalInfo (from domains.h)
//
// Side Effects    : Memory, IO, Ugly stuff happens with
//                   DomainGlobalInfo 
//
/----------------------------------------------------------------------*/
 
LpairTreeParams find_fist_compat_specdom(Lpair_p* knowledge_tree, 
                              LearnSig_p* sig, DNormSubst_p* subst,
                              bool complete)
{
   LpairTreeParams res = {0,0,0,0,0,0};
   char            domain_name[MAXPATHLEN];
   Lpair_p         specification = NULL;
   DomainFrame     *domain = &(DomainGlobalInfo[MAXDOM_ANZ-1]);
   short           f;

   domain->def_anz = 1;
   SuchModus = SFIND_FIRST;
   ClearSet(&(domain->def_gleichungen[0]));

   while(!TestToken(NoToken) && !res.goal_dist) /* Find first
                                                   compatible 
                                                   domain */ 
   {      
      AcceptIdent(ident, "domain");
      AcceptToken(colon);
      strcpy(domain_name, AktLiteral);
      AcceptToken(identifier);
      
      printf("Testing Domain %s.\n", domain_name);

      AcceptIdent(ident, "type");
      AcceptToken(colon);
      AcceptIdent(ident, "specification");
      
      *sig = ParseLearnSig();
            
      /* Skip the Example list... */
      while(!(TestIdent(ident, "specification") && ColonFollows))
      {
         NextRealToken();
         if(TestToken(NoToken))
         {
            ScannerError("Unexpected end of file");
         }
      }
      AcceptIdent(ident, "specification");
      AcceptToken(colon);

      ParseSpecTree(&specification);
      LpairTreeAdd(specification, *sig, &(domain->def_gleichungen[0]));      
      FreeLpairTree(specification);
      specification = NULL;

      for(f=1; f<= (*sig)->entrycount; f++)
      {
         domain->dom_funktion[f].arity = (*sig)->sig_data[f].arity;
         strncpy(domain->dom_funktion[f].ident,
                 (*sig)->sig_data[f].ident, IDENTLENGTH);
      }
      domain->dom_fkt_anz = (*sig)->entrycount;
            
      if(FindAndTestAllMatches())
      {
         res = read_specdata(knowledge_tree, domain_name, complete); 
         printf("Specification domain %s identified.\n",domain_name);
         
         *subst = AllocEmptyDNormSubst();
         
         for(f=1; f<= domain->dom_fkt_anz; f++)
         {
            (*subst)->norm_id[domain->dom_funktion[f].fmatch] = f;
         }
         (*subst)->f_count = domain->dom_fkt_anz;
      }
      else
      {
         FreeLearnSig(*sig);
      }
      ClearSet(&(domain->def_gleichungen[0]));
   }
   return res;
}

/*-----------------------------------------------------------------------
//
// Function:  find_rest_compat_specdom() 
//
//   After find_fist_compat_specdom() has discovered one compatible
//   domain, this function checks, if any other domains _with the same
//   signature_ is also compatible. If it is, it will be read in and
//   added to the tree, and the LearnParams provided will be updated
//   accordingly. find_fist_compat_specdom() has to be called before
//   this function, and it must have found a domain, otherwise the
//   results of this function are definitly undefined!
//
// Global Variables: -
//
// Side Effects    : I/O, Memory, Changes to the tree
//
/----------------------------------------------------------------------*/


void find_rest_compat_specdom(Lpair_p* knowledge_tree, LearnSig_p sig,
                              LpairTreeParams_p params, bool complete) 
{
   LearnSig_p        new_sig;
   char              domain_name[MAXPATHLEN];
   Lpair_p           specification = NULL;
   LpairTreeParams   tmp = {0,0,0,0,0,0};
   DomainFrame       *domain = &(DomainGlobalInfo[MAXDOM_ANZ-1]);

   while(!TestToken(NoToken))
   {      
      AcceptIdent(ident, "domain");
      AcceptToken(colon);
      strcpy(domain_name, AktLiteral);
      AcceptToken(identifier);
      
      printf("Testing Domain %s.\n", domain_name);
      
      AcceptIdent(ident, "type");
      AcceptToken(colon);
      AcceptIdent(ident, "specification");
      
      new_sig = ParseLearnSig();
      
      if(!LearnSigCompare(sig, new_sig))
      {
         while(!(TestIdent(ident, "domain")&&ColonFollows)&&!TestToken(NoToken))
         { /* Skip this domain...*/
            NextRealToken();
         }
      }
      else
      {
         /* Skip the Example list... */
         while(!(TestIdent(ident, "specification") && ColonFollows))
         {
            NextRealToken();
            if(TestToken(NoToken))
            {
               ScannerError("Unexpected end of file");
            }
         }
         AcceptIdent(ident, "specification");
         AcceptToken(colon);
         
         ParseSpecTree(&specification);
         
         LpairTreeAdd(specification, sig, &(domain->def_gleichungen[0]));
         FreeLpairTree(specification);
         specification = NULL;
         
         if(FindRules())
         {
            tmp = read_specdata(knowledge_tree, domain_name, complete);
            params->proofs    = max(params->proofs,    tmp.proofs);
            params->ave_ref   = max(params->ave_ref,   tmp.ave_ref);
            params->tot_ref   = max(params->tot_ref,   tmp.tot_ref);
            params->goal_dist = max(params->goal_dist, tmp.goal_dist);
            printf("Specification domain %s added.\n",domain_name);
         }
         ClearSet(&(domain->def_gleichungen[0]));
      }
   }
}


/*-------------------------------------------------------------------------
//
// Function: get_kb_variable_value()
//
//   Return a pointer to a (constant) string with the textual value of
//   the given KB-Variables (if it exists, otherwise NULL).
//
// Global Variables: -
//
// Side Effect     : May load the kb_variables, issue warnings, and
//                   terminate the program if more than
//                   MAX_KB_VARIABLES are in the file.
//
//-----------------------------------------------------------------------*/

char* get_kb_variable_value(char* var)
{
   static char* variables[MAX_KB_VARIABLES+1];
   static char* values[MAX_KB_VARIABLES+1];
   static bool  load_tried=false;
   int          i=0;

   if(!load_tried)
   {
      strcpy(PathNameHelp, KBFullName());
      strcat(PathNameHelp, "/kb_variables");
      if(FileExists(PathNameHelp,false))
      {
         printf("Loading KB-Variables\n");
         InitScanner(PathNameHelp);      
         while(AktToken!=NoToken)
         {
            if(i==MAX_KB_VARIABLES)
            {
               Error("get_kb_variable_value()", 
                     "Maximum number of KB variables reached");
            }
            variables[i] = SecureStrdup(AktLiteral);
            AcceptToken(identifier);
            AcceptToken(equal_sign);
            values[i] = SecureStrdup(AktLiteral);
            if(!TestToken(string))
            {
               CheckToken(identifier);
            }
            NextRealToken();
            i++;
         }
         EndScanner();
      }
      variables[i] = NULL;
      load_tried = true;
   }
   
   for(i=0; variables[i]; i++)
   {
      if(strcmp(variables[i],var)==0)
      {
         return values[i];
      }
   }
   printf("Warning: knowledge base variable %s not found!\n", var);
   return NULL;
}


/*-----------------------------------------------------------------------*/
/*                  Exportierte Funktionen                               */
/*-----------------------------------------------------------------------*/


/*-------------------------------------------------------------------------
//
// Function: KBFullName()
//
//   Computes the directory name of the knowledge base to use (once)
//   and returns a pointer to it.
//
// Global Variables: PathNameHelp
//
// Side Effects    : May terminate program if path cannot be
//                   resolved. 
//
//-----------------------------------------------------------------------*/

char* KBFullName()
{
   static char kb_name[MAXPATHLEN]="";
   
   if(!kb_name[0])
   {
      if(KnowledgeBase[0])
      {
         strcpy(TmpPathName, KnowledgeBase);
      }
      else
      {
         strcpy(TmpPathName, "KNOWLEDGE");
      }
      if(!realpath(TmpPathName, kb_name))
      {
         printf("Filename '%s' could not be resolved...\n", TmpPathName);
         perror("ERROR: realpath");
         exit(-1);
      }
   }
   return kb_name;
}

/*-------------------------------------------------------------------------
//
// Function: GetBoolKBVariable()
//
//   Returns the boolean value of a given KB variable (if it exists),
//   otherwise returns false
//
// Global Variables: -
//
// Side Effect     : via get_kb_variable_value()
//
//-----------------------------------------------------------------------*/

bool GetBoolKBVariable(char* var)
{
   char* value;

   if((value=get_kb_variable_value(var)))
   {
      if(strcmp(value,"TRUE")==0)
      {
         return true;
      }
      else if(strcmp(value,"FALSE")==0)
      {
         return false;
      }
      printf("Warning: knowledge base variable %s does not "
             "appear to be boolean!\n", var);
   }
   return false;
}


/*-------------------------------------------------------------------------

FUNCTION         : DNormSubst_p FindEquivalentGoal(Lpair_p testgoal)

Beschreibung     : Checks, wether one of DISCOUNTS goals is equivalent
                   to testgoal. In the positive case the DomSubst is
                   returned, NULL otherwise.

Globale Variable : SetOfGoals (from termpair.c)

Seiteneffekte    : Memory operations

Aenderungen      : <1> 5.1.1994 neu

-------------------------------------------------------------------------*/

DNormSubst_p FindEquivalentGoal(Lpair_p testgoal)
{
   termpair*   ptr;
   term        *left, *right;
   DNormSubst_p subst = NULL;

   ptr = SetOfGoals.first;
   while (ptr && !subst)
   { 
      left  = ptr->left;
      right = ptr->right;
      subst = OrderNormTerms(&left, &right);

      if(! ((CmpLtermNterm(testgoal->lside, left, subst)==0)
         && (CmpLtermNterm(testgoal->rside, right, subst)==0)))
      {
         FreeDNormSubst(subst);
         subst = NULL;
      }
            
      ptr = ptr->next;
   }
   return subst;
}
  
/*-----------------------------------------------------------------------
//
// Function: InitGoalDomExpert()
//
//   Initializes the Goal-dependend expert, returns maximal values in
//   found Domain 
//
// Global Variables: PathNameHelp, KnowledgeBase
//
// Side Effects    : Memory Operations, I/O
//
/----------------------------------------------------------------------*/


LpairTreeParams InitGoalDomExpert(Lpair_p *tree, DNormSubst_p
                                  *goal_bound_subst, bool complete)
{
   char            domain_name[MAXPATHLEN];
   Lpair_p         dgoal;
   DNormSubst_p    subst = NULL;
   LpairTreeParams params = {0,0,0,0,0,0};

   printf("GoalDomExpert started...\n");
   NormTermPreserveArity = GetBoolKBVariable("PreserveArity");

   strcpy(PathNameHelp, KBFullName());
   strcat(PathNameHelp, "/goaldoms");

   InitScanner(PathNameHelp);
   printf("Scanning goal domain list %s.\n", PathNameHelp);
   
   while (!TestToken(NoToken) && !subst)
   {      
      while(!(TestIdent(ident, "domain") && ColonFollows))
      {
         NextRealToken();
         if(TestToken(NoToken))
         {
            ScannerError("Unexpected end of file");
         }
      }
      AcceptIdent(ident, "domain");
      AcceptToken(colon);
      strcpy(domain_name, AktLiteral);
      AcceptToken(identifier);
      
      while(!(TestIdent(ident, "goals") && ColonFollows))
      {
         NextRealToken();
         if(TestToken(NoToken))
         {
            ScannerError("Unexpected end of file");
         }
      }
      AcceptIdent(ident, "goals");
      AcceptToken(colon);
      
      dgoal = ParseLpair();
      
      subst = FindEquivalentGoal(dgoal);
      
      FreeLpair(dgoal);
   }
   EndScanner();
   
   if(subst)
   {
      printf("Goal domain %s identified.\n", domain_name);
      
      VTclear(&(subst->variables));
      *goal_bound_subst = subst;
      params = read_goaldata(tree, domain_name, complete);
   } 
   return params;
}

/*-----------------------------------------------------------------------
//
// Function: InitSpecDomExpert()
//
//   Initializes the Specification-dependend expert, returns struct of
//   maximal relevant values in the found Domains
//
// Global Variables: PathNameHelp, KnowledgeBase
//
// Side Effects    : Memory Operations, I/O
//
/----------------------------------------------------------------------*/

LpairTreeParams InitSpecDomExpert(Lpair_p *tree, DNormSubst_p
                                  *spec_bound_subst, bool complete,
                                  bool all_spec_doms)  
{
   LearnSig_p      domain_sig;
   LpairTreeParams max_values;
   short           i;

   printf("Trying to find knowledge about the specification.\n");
   NormTermPreserveArity = GetBoolKBVariable("PreserveArity");

   if(DGICount>=MAXDOM_ANZ-1)
   {
      Error( __FILE__ ": "  "InitSpecDomExpert",
             "No space for domain testing..." );
   }

   AktDomNr = MAXDOM_ANZ-1;

   strcpy(PathNameHelp, KBFullName());
   strcat(PathNameHelp, "/specdoms");
   
   InitScanner(PathNameHelp);
   printf("Scanning specification domain list %s.\n", PathNameHelp);

   max_values = find_fist_compat_specdom(tree, &domain_sig,
                                        spec_bound_subst, 
                                        complete); 
   
   if(max_values.goal_dist)
   {
      if(all_spec_doms)
      {
         find_rest_compat_specdom(tree, domain_sig, &max_values, complete);
      }
      FreeLearnSig(domain_sig);
   }
   

   if( no_rule_equ ) /* Domaenensuche kann CP-Menge in Regeln und */
                     /* Gleichungen verwandeln..diese muss */
                     /* rueckgaengig gemacht werden     */
   {
      TPClearCPCache();
      ForAllRulesDo( InsertCP );
      
      for( i=0; i <= FuncCount; i++ )
      {
         SetOfRules[i].first = SetOfRules[i].last = NULL;
      } /* Ende von for */
      SetOfRules[0].setcount = SetOfRules[0].count = 0;
      
      
      ForAllEquDo(  InsertCP );
      SetOfEquations.first = SetOfEquations.last = NULL;
      SetOfEquations.setcount = SetOfEquations.count = 0;
   } /* Ende von if */
   
   EndScanner();
   
   return max_values;
}

/*-----------------------------------------------------------------------*/
/*                       Ende des Files                                  */
/*-----------------------------------------------------------------------*/


