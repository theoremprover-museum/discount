/*
//=============================================================================
//      Projekt:        TEAM-COMPLETION
//
//      Modul:          buildcp
//
//      Author:         Werner Pitz, 1991
//
//      Beschreibung:   Erzeugung kritischer Paare
//-----------------------------------------------------------------------------
//      $Log: buildcp.c,v $
//      Revision 0.3  1991/09/19  13:05:19  pitz
//      ref_new_cp wird mitgezaehlt.
//
//      Revision 0.2  1991/09/05  14:27:18  pitz
//      special_factor nach cpweight.* ausgelagert.
//      Neue Funktion SpecialWeight in cpweight.* ubernimmt die
//      Bewertung von speziellen kritischen Paaren.
//
//      Revision 0.1  1991/09/05  12:51:28  pitz
//      Unterstuetzung von special_factor implementiert.
//
//      Revision 0.0  1991/08/09  08:18:19  pitz
//      Initiale Version
//
//=============================================================================
*/

#include    <stdio.h>

#include    "defines.h"
#include    "error.h"
#include    "vartree.h"
#include    "polynom.h"
#include    "term.h"
#include    "termpair.h"
#include    "subst.h"
#include    "order.h"
#include    "cpweight.h"
#include    "reduce.h"
#include    "pcl.h"
#include    "buildcp.h"
#include    "complet.h"


/*
//-----------------------------------------------------------------------------
//      Datendeklarationen
//-----------------------------------------------------------------------------
*/

termpair    *prule;
termpair    *pequ;
termpair    *pgoal;

bool        KapurCriteria       = false;


/* In parent1 und parent2 stehen die Termpaare,  aus denen ein kritisches Paar */
/* oder ein kritisches Ziel entstanden ist. Bei kritischen Paarem ist ein El-  */
/* ternteil jeweils entweder eine Regel oder eine Gleichung.                   */
/* Bei kritischen Zielen ist parent1 stets das beteiligte Ziel und parent2     */
/* die beteiligte Regel bzw. Gleichung.                                        */
static  termpair    *parent1    = NULL;
static  termpair    *parent2    = NULL;

static  term        *Left1, *Right1;
static  term        *Left2, *Right2;


/*
//-----------------------------------------------------------------------------
//      Modulinterne Definitionen
//-----------------------------------------------------------------------------
*/

static void    CritPair    ( termpair *cp );

static term    *rplacsubst ( subst *sigma, term *l1, term *pos, term *r2 );
static void    BuildCP     ( term *pos, subst *sigma );

static void    makecp      ( term *pos );
static void    MakeCP      ( term *left1, term *right1, term  *pos,
			     term *left2, term *right2 );

static void    BuildPara   ( term *Left1, term *Right1, term *pos,
			     term *Right2, subst *sigma );
static void    MakePara    ( term *left1, term *right1, term  *pos,
			     term *left2, term *right2 );


/*
//=============================================================================
//      Meldungen des Systems bei Aenderungen durch Paramodulation
//=============================================================================
*/

#define ProvedBy( equ )                                                 \
{                                                                       \
    printf     ( "Dies gilt da:       %4ld  ", equ->number );           \
    printtpair ( equ );                                                 \
}



#define Paramodul( goal )                                               \
{                                                                       \
    printf     ( "Paramoduliere Ziel: %4ld  ", goal->number );          \
    printtpair ( goal );                                                \
}

/*
//-----------------------------------------------------------------------------
//  Funktion:       CritPair ( termpair *cp )
//
//  Parameter:      cp      neues kritisches Paar
//
//  Beschreibung:   cp wird als Kritisches Paar anhand der aktuellen
//                  Gewichtsfunktion gewichtet und in die Menge der
//                  kritischen Paare eingefuegt
//-----------------------------------------------------------------------------
*/

static void CritPair ( termpair *cp )
{
   cp->quality = ErodeQuality((cp->parent1->quality + cp->parent2->quality)/2);
   cp->weight = CPWeight ( cp );
   if (special_factor && (cp->parent1->special_flag || cp->parent2->special_flag))
      SpecialWeight ( cp );
   InsertCP ( cp );
   ref_new_cp++;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       rplacsubst ( subst *sigma,
//                               term *left1, term *pos, term *right2 )
//
//  Parameter:      sigma   Substitution
//                  left1   Term
//                  pos     Position in left1
//                  right2  Term
//
//  Rueckgabewert:  rechte Seite eines kritsichen Paares
//                  sigma (left1)[pos<-sigma (right2)]
//
//  Beschreibung:   Aufbau einer rechten Seite eines kritischen Paares.
//                  Wird auch fuer Paramodulation verwendet.
//-----------------------------------------------------------------------------
*/

static term *rplacsubst ( subst *sigma, term *left1, term *pos, term *right2 )
{
    term    *tptr;
    short   i;

    if (left1 == pos)
        return substitute ( sigma, right2 );

    if (varp(left1))
    {
        tptr = assoc (sigma, left1->fcode);
        return (tptr) ? tptr
                      : newterm (left1->fcode);
    }

    tptr = newterm (left1->fcode);
    for ( i = 0; i < left1->arity; i++ )
        tptr->argument[i] = rplacsubst (sigma, left1->argument[i], pos, right2);

    return tptr;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       BuildCP
//
//  Parameter:      pos             Stelle (Pointer) in Left1
//                  sigma           Match von Left2 in Left1
//
//  globale Werte:  Left1, Right1   erstes Termpaar
//                  Left2, Right2   zweites Termpaar
//
//  Beschreibung:   Aufbau des kritschen Paares, das sich aus der
//                  Ueberlappung an der Stelle pos in Left1 mit Left2
//                  ergibt.
//-----------------------------------------------------------------------------
*/


/* This is the old BuildCP-Function, it does not support PCL. To keep */
/* problems smaller I decided to use a unified Version, the old and */
/* now debugged PCL-Version. The semantics are very nearly identical, */
/* but there is one difference which will influece outside behaviour. */
/* In the PCL-version any critical pair is represented as a pair, */
/* while in the old, non-PCL-version only CP's that could not be */
/* reduced to a trivial Equation had been made Termpairs. This may */
/* lead to minor inefficiencies and it will definitly influence the */
/* termpair-counter (TPcounter) used in newpair() from termpair.h */
 /*          StS          */

/*
    static void BuildCP ( term  *pos, subst *sigma )
    {
        term        *left, *right;
        bool        redcheck;

        if (KapurCriteria)
        {
             getweight (left = substitute ( sigma, Left1 ));
             redcheck = CheckReduce ( &left, parent1, parent2 );
             deleteterm ( left );
             if (redcheck)
                 return;
        }

        getweight ( left  = substitute ( sigma, Right1 ) );
        getweight ( right = rplacsubst ( sigma, Left1, pos, Right2 ) );

        tpnewvars ( left, right );
        reduce ( &left );
        reduce ( &right );

        if (equal (left, right))
        {
            deleteterm ( left );
            deleteterm ( right );
            #ifdef SHOWCP
                putchar ( '-' );
                flush ();
            #endif
            return;
        }

        #ifdef SHOWCP
            putchar ( '+' );
            flush ();
        #endif

        CritPair ( newpair (left, right, parent1, parent2) );

        if (parent1)
            Referee_CountCP ( parent1 );
        if (parent2)
            Referee_CountCP ( parent2 );
    }
*/

static void BuildCP ( term  *pos, subst *sigma )
{
    termpair    *cp;
    term        *left, *right;
    bool        redcheck;

    if (KapurCriteria)
    {
       getweight (left = substitute ( sigma, Left1 ));
       redcheck = CheckReduce ( &left, parent1, parent2 );
       deleteterm ( left );
       if (redcheck)
	  return;
    }

    getweight ( left  = substitute ( sigma, Right1 ) );
    getweight ( right = rplacsubst ( sigma, Left1, pos, Right2 ) );
    cp = newpair ( left, right,
		  parent1, parent2 );
    PCL_NEWCP (cp, Left1, pos, sigma);

    tpnewvars ( cp->left, cp->right );
    PCL_REDUCE_LEFT ( cp );
    reduce ( &(cp->left) );
    PCL_REDUCE_RIGHT ( cp );
    reduce ( &(cp->right) );

    if (equal (cp->left, cp->right))
   {
      deleteterm ( cp->left );
      deleteterm ( cp->right );
      deletepair ( cp );
#ifdef SHOWCP
      putchar ( '-' );
      flush ();
#endif
      return;
   }
#ifdef SHOWCP
    putchar ( '+' );
    flush ();
#endif
    
    CritPair ( cp );
    
    if (parent1)
       Referee_CountCP ( parent1 );
    if (parent2)
       Referee_CountCP ( parent2 );
}



/*
//-----------------------------------------------------------------------------
//  Funktion:       makecp ( term *pos )
//
//  Parameter:      pos             Position in Left1
//
//  globale Werte:  Left1, Right1   erstes Termpaar
//                  Left2, Right2   zweites Termpaar
//
//  Beschreibung:   Ueberprueft alle Stellen in Left1 auf kritische
//                  Ueberelappungen mit Left2 und bildet gegebenenfalls
//                  das kritische Paar.
//                  Hilfsfunktion fuer MakeCP ohne zusaetzliche Parameter
//                  left1, right1, left2, right2.
//-----------------------------------------------------------------------------
*/

static void makecp ( term *pos )
{
    subst   *sigma  = NULL;
    short   i;

    if (varp (pos))
        return;

    if ((pos->fcode == Left2->fcode) && unify (pos, Left2, &sigma))
    {
        BuildCP ( pos, sigma );
        deletesubst ( sigma );
        sigma = NULL;
    }

    for (i = 0; i < pos->arity; i++ )
        makecp ( pos->argument[i] );
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       MakeCP ( term *left1, term *right1, term *pos,
//                           term *left2, term *right2 )
//
//  Parameter:      left1, right1   erstes Termpaar
//                  pos             Position in left1
//                  left2, right2   zweites Termpaar
//
//  Beschreibung:   Ueberprueft alle Stellen in left1 auf kritische
//                  Ueberelappungen mit left2 und bildet gegebenenfalls
//                  das kritische Paar.
//-----------------------------------------------------------------------------
*/

static void MakeCP ( term *left1, term *right1, term *pos,
                     term *left2, term *right2 )
{
    subst   *sigma  = NULL;
    short   i;

    if (varp (pos))
        return;

    Left1 = left1;    Right1 = right1;
    Left2 = left2;    Right2 = right2;

    if ((pos->fcode == Left2->fcode) && unify (pos, left2, &sigma))
    {
        BuildCP ( pos, sigma );
        deletesubst ( sigma );
        sigma = NULL;
    }

    for (i = 0; i < pos->arity; i++ )
        makecp ( pos->argument[i] );
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       CPRuleRule  ( termpair *rule2 )
//
//  Parameter:      rule        1. Regel
//
//  Globaler Wert:  prule       2. Regel
//
//  Beschreibung:   Es werden alle kritischen Paare zwischen rule1 und
//                  rule2 gebildet.
//-----------------------------------------------------------------------------
*/

void CPRuleRule ( termpair *rule )
{
    term    *left, *right;
    short   i;

    if (prule != rule)
    {
        parent1 = prule;
        parent2 = rule;
        PCL_INIT_CP (parent1, parent2, true);
        MakeCP ( prule->left, prule->right, prule->left,
                 rule->left, rule->right );

        parent1 = rule;
        parent2 = prule;
        PCL_INIT_CP (parent1, parent2, true);
        left  = rule->left;
        right = rule->right;
        if (funcp(left))
            for (i = 0; i < left->arity; i++ )
                MakeCP ( left, right, left->argument[i],
                         prule->left, prule->right );
        return;
    }
    else
    if ((prule == rule) && (funcp(prule->left)))
    {
        parent1 = rule;
        parent2 = prule;
        PCL_INIT_CP (parent1, parent2, true);

        tpcopynew ( rule, &left, &right );
        for (i = 0; i < left->arity; i++ )
            MakeCP (left,right, left->argument[i], prule->left,prule->right);

        deleteterm ( left );
        deleteterm ( right );
        return;
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       CPRuleEqu   ( termpair *equ )
//
//  Parameter:      equ         eine Gleichung
//
//  Globaler Wert:  prule       Regel
//
//  Beschreibung:   Es werden alle kritischen Paare zwischen rule und
//                  equ gebildet.
//-----------------------------------------------------------------------------
*/

void CPRuleEqu ( termpair *equ )
{
    term    *left, *right;
    short   i;

    parent1 = equ;
    parent2 = prule;
    PCL_INIT_CP (parent1, parent2, true);
    MakeCP ( equ->left,  equ->right, equ->left,  prule->left, prule->right );
    PCL_INIT_ECP ( false );
    MakeCP ( equ->right, equ->left,  equ->right, prule->left, prule->right );

    parent1 = prule;
    parent2 = equ;
    left  = prule->left;
    right = prule->right;
    if (funcp(left))
        for (i = 0; i < left->arity; i++ )
        {
            PCL_INIT_CP (parent1, parent2, true);
            MakeCP (left, right, left->argument[i], equ->left,  equ->right );
            PCL_INIT_CP (parent1, parent2, false);
            MakeCP (left, right, left->argument[i], equ->right, equ->left  );
        }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       CPEquRule   ( termpair *rule )
//
//  Parameter:      rule        eine Regel
//
//  Globaler Wert:  pequ        eine Gleichung
//
//  Beschreibung:   Es werden alle kritischen Paare zwischen rule und
//                  equ gebildet.
//-----------------------------------------------------------------------------
*/

void CPEquRule ( termpair *rule )
{
    term    *left, *right;
    short   i;

    parent1 = pequ;
    parent2 = rule;
    PCL_INIT_CP (parent1, parent2, true);
    MakeCP ( pequ->left,  pequ->right, pequ->left,  rule->left, rule->right );
    PCL_INIT_ECP ( false );
    MakeCP ( pequ->right, pequ->left,  pequ->right, rule->left, rule->right );

    parent1 = rule;
    parent2 = pequ;
    left  = rule->left;
    right = rule->right;
    if (funcp(left))
        for (i = 0; i < left->arity; i++ )
        {
            PCL_INIT_CP (parent1, parent2, true);
            MakeCP (left, right, left->argument[i], pequ->left,  pequ->right );
            PCL_INIT_CP (parent1, parent2, false);
            MakeCP (left, right, left->argument[i], pequ->right, pequ->left  );
        }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       CPEquEqu  ( termpair *equ  )
//
//  Parameter:      equ         1. Gleichung
//
//  Globaler Wert:  prule       Regel
//
//  Beschreibung:   Es werden alle kritischen Paare zwischen equ1 und
//                  equ2 gebildet.
//-----------------------------------------------------------------------------
*/

void CPEquEqu ( termpair *equ )
{
    term        *left, *right;
    short       i;

    if (pequ != equ)
    {
        parent1 = pequ;
        parent2 = equ;
        PCL_INIT_CP (parent1, parent2, true);
        MakeCP (pequ->left, pequ->right, pequ->left,  equ->left, equ->right);
        PCL_INIT_CP (parent1, parent2, false);
        MakeCP (pequ->left, pequ->right, pequ->left,  equ->right,equ->left );
        PCL_INIT_CP (parent1, parent2, true);
        PCL_INIT_ECP ( false );
        MakeCP (pequ->right,pequ->left,  pequ->right, equ->left, equ->right);
        PCL_INIT_CP (parent1, parent2, false);
        PCL_INIT_ECP ( false );
        MakeCP (pequ->right,pequ->left,  pequ->right, equ->right,equ->left );

        parent1 = equ;
        parent2 = pequ;
        left  = equ->left;
        right = equ->right;

        if (funcp(left))
            for (i = 0; i < left->arity; i++ )
            {
                PCL_INIT_CP (parent1, parent2, true);
                MakeCP (left,right,left->argument[i], pequ->left,pequ->right);
                PCL_INIT_CP (parent1, parent2, false);
                MakeCP (left,right,left->argument[i], pequ->right,pequ->left);
            }

        if (funcp (right))
            for (i = 0; i < right->arity; i++ )
            {
                PCL_INIT_CP (parent1, parent2, true);
                PCL_INIT_ECP ( false );
                MakeCP (right,left,right->argument[i], pequ->left,pequ->right);
                PCL_INIT_CP (parent1, parent2, false);
                PCL_INIT_ECP ( false );
                MakeCP (right,left,right->argument[i], pequ->right,pequ->left);
            }

        return;
    }
    else
    {
        parent1 = pequ;
        parent2 = pequ;

        tpcopynew ( pequ, &left, &right );

        PCL_INIT_CP (parent1, parent2, true);
        MakeCP ( left,right, left,  pequ->left, pequ->right );
        PCL_INIT_CP (parent1, parent2, false);
        MakeCP ( left,right, left,  pequ->right,pequ->left  );

        PCL_INIT_CP (parent1, parent2, true);
        PCL_INIT_ECP ( false );
        MakeCP ( right,left, right, pequ->left, pequ->right );
        PCL_INIT_CP (parent1, parent2, false);
        PCL_INIT_ECP ( false );
        MakeCP ( right,left, right, pequ->right,pequ->left  );

        deleteterm ( left );
        deleteterm ( right );
        return;
    }
}


/*
//=============================================================================
//      Paramodulation
//=============================================================================
*/


/*
//-----------------------------------------------------------------------------
//  Funktion:       BuildPara ( term *Left1, term *Right1, term *pos,
//                              term *Right2, subst *sigma )
//
//  Parameter:      Left1, Right1   erstes Termpaar
//                  pos             Stelle (Pointer) in Left1
//                  Right2          zweites Termpaar - rechte Seite
//                  sigma           Match von Left2 in Left1
//
//  Beschreibung:   Aufbau einer Paramodulatoion, das sich aus der
//                  Ueberlappung an der Stelle pos in Left1 mit Left2
//                  ergibt.
//                  Ablauf:
//                  - krit. Ziel bilden und Gewicht bestimmen mit getweight
//                  - krit. Ziel reduzieren
//                  - auf Gleichheit und Unifizierbarkeit testen -> evtl. Proved
//                  - Gewicht mit CGWeight bestimmen und in die Menge einf"ugen
//-----------------------------------------------------------------------------
*/

static void BuildPara ( term *Left1, term *Right1, term *pos,
                        term *Right2, subst *sigma )
{
    term        *left, *right;
    termpair    *goal;
    subst       *tau = NULL;

    getweight (left = substitute ( sigma, Right1 ));
    getweight (right = rplacsubst ( sigma, Left1, pos, Right2 ));

    tpnewvars ( left, right );
    reduce ( &left );
    reduce ( &right );
    goal = newpair (left, right, parent1, parent2);

    if (equal (left, right))
    {
        AddGoal ( goal );
        Paramodul ( goal );
        Proved ();
    }

    if (unify (left, right, &tau))
    {
        AddGoal ( goal );
        Paramodul ( goal );
        printf ( "Instanziere Ziel:    %4ld  ", goal->number );

        getweight (goal->left  = substitute (tau, left));
        getweight (goal->right = substitute (tau, right));
        deleteterm ( left );
        deleteterm ( right );
        printtpair ( goal );
        Proved ();
    }


    goal->weight = CGWeight ( goal );
    InsertCritGoal ( goal );
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       MakePara ( term *left1, term *right1, term *pos,
//                             term *left2, term *right2 )
//
//  Parameter:      left1, right1   erstes Termpaar  ( Dies ist das Ziel, aus dem
//                                  das krit, Ziel gebildet wird. )
//                  pos             Position in left1  
//                  left2, right2   zweites Termpaar ( Dies ist die Regel/Gleichung,
//                                  aus dem das krit. Ziel gebildet wird. )
//
//  Beschreibung:   Ueberprueft alle Stellen in left1 auf moegliche
//                  Paramodulation mit left2 und bildet gegebenenfalls
//                  das neue Goal.
//-----------------------------------------------------------------------------
*/

static void MakePara ( term *left1, term *right1, term *pos,
                       term *left2, term *right2 )
{
    subst   *sigma  = NULL;
    short   i;

    if (varp (pos))
        return;

    if (unify (pos, left2, &sigma))
    {
        BuildPara ( left1, right1, pos, right2, sigma );
        deletesubst ( sigma );
        sigma = NULL;
    }

    for (i = 0; i < pos->arity; i++ )
        MakePara ( left1, right1, pos->argument[i], left2, right2 );
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       ParaRule ( termpair *rule )
//
//  Parameter:      rule    Mit dieser Gleichung soll paramodularisiert
//                          werden
//
//  Globaler Wert:  pgoal    zu paramodularisierendes Ziel
//
//  Beschreibung:   Paramodulation eines Ziels mit einer Regel
//                  Diese Funktion wird aufgerufen, wenn ein neues Ziel
//                  betrachtet wird - n"amlich pgoal. Dieses wird mit der 
//                  Regel rule paramoduliert.
//                  rule durchl"auft dabei normalerweise die gesamte 
//                  Regelmenge.
//-----------------------------------------------------------------------------
*/

void ParaRule ( termpair *rule )
{
    if (Paramodulation)
    {
        parent1 = pgoal;
        parent2 = rule;
        MakePara (pgoal->left, pgoal->right, pgoal->left,
                  rule->left, rule->right);
        MakePara (pgoal->right,pgoal->left,  pgoal->right,
                  rule->left, rule->right);
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       ParaNewRule ( termpair *rule )
//
//  Parameter:      goal    Dieses Ziel soll paramodularisiert werden
//
//  Globaler Wert:  prule   Mit dieser Regel soll paramoduliert werden
//
//  Beschreibung:   Paramodulation eines Ziels mit einer Regel
//                  Diese Funktion wird aufgerufen, wenn eine neue Regel
//                  betrachtet wird - n"amlich prule. Mit dieser wird 
//                  das Ziel goal paramoduliert.
//                  goal durchl"auft dabei normalerweise die gesamte
//                  Menge von Zielen.
//                  
//-----------------------------------------------------------------------------
*/

void ParaNewRule ( termpair *goal )
{
    if (Paramodulation)
    {
        parent1 = goal;
        parent2 = prule;
        MakePara (goal->left,  goal->right, goal->left,
                  prule->left, prule->right);
        MakePara (goal->right, goal->left,  goal->right,
                  prule->left, prule->right);
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       ParaEqu ( termpair *equ )
//
//  Parameter:      equ     Mit dieser Gleichung soll paramodularisiert
//                          werden
//
//  Globaler Wert:  pgoal    zu paramodularisierendes Ziel
//
//  Beschreibung:   Paramodulation eines Ziels mit einer Gleichung
//                  Diese Funktion wird aufgerufen, wenn ein neues Ziel
//                  betrachtet wird - n"amlich pgoal. Dieses wird mit der 
//                  Gleichung equ paramoduliert.
//                  equ durchl"auft dabei normalerweise die gesamte 
//                  Gleichungsmenge.
//-----------------------------------------------------------------------------
*/

void ParaEqu ( termpair *equ )
{
    if (Paramodulation)
    {
        parent1 = pgoal;
        parent2 = equ;
        MakePara (pgoal->left, pgoal->right, pgoal->left,
                  equ->left,  equ->right);
        MakePara (pgoal->left, pgoal->right, pgoal->left,
                  equ->right, equ->left );
        MakePara (pgoal->right,pgoal->left,  pgoal->right,
                  equ->left,  equ->right);
        MakePara (pgoal->right,pgoal->left,  pgoal->right,
                  equ->right, equ->left );
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       ParaNewEqu ( termpair *goal )
//
//  Parameter:      goal    Dieses Ziel soll paramoduliert werden
//
//  Globaler Wert:  pequ    Mit dieser Gleichung paramoduliert
//
//  Beschreibung:   Paramodulation eines Ziels mit einer Gleichung
//                  Diese Funktion wird aufgerufen, wenn eine neue Gleichung
//                  betrachtet wird - n"amlich pequ. Mit dieser wird 
//                  das Ziel goal paramoduliert.
//                  goal durchl"auft dabei normalerweise die gesamte
//                  Menge von Zielen.
//-----------------------------------------------------------------------------
*/

void ParaNewEqu ( termpair *goal )
{
    if (Paramodulation)
    {
        parent1 = goal;
        parent2 = pequ;
        MakePara (goal->left,  goal->right, goal->left,
                  pequ->left,  pequ->right);
        MakePara (goal->left,  goal->right, goal->left,
                  pequ->right, pequ->left );
        MakePara (goal->right, goal->left,  goal->right,
                  pequ->left,  pequ->right);
        MakePara (goal->right, goal->left,  goal->right,
                  pequ->right, pequ->left );
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       ParaGoal ( termpair *goal )
//
//  Parameter:      goal    neues Ziel
//
//  Beschreibung:   Paramodulation eines Ziels mit Regeln und Gleichungen
//-----------------------------------------------------------------------------
*/

void ParaGoal ( termpair *goal )
{
    if (Paramodulation)
    {
        pgoal = goal;
        ForAllRulesDo (ParaRule);
        ForAllEquDo   (ParaEqu);
    }
}



/*
//-----------------------------------------------------------------------------
//  Funktion:       UnifyGoal ( termpair *goal )
//
//  Parameter:      goal    ein Ziel
//
//  Beschreibung:   Diese Funtion versucht durch Unifikation der beiden
//                  Seiten der Zielgleichung eine Instanz des Ziels
//                  zu finden, so dass dies beiden Seiten uebersinstimmen.
//-----------------------------------------------------------------------------
*/

bool UnifyGoal ( termpair *goal )
{
    subst   *tau     = NULL;
    term    *left    = goal->left;
    term    *right   = goal->right;

    if (unify (goal->left, goal->right, &tau))
    {
        getweight (goal->left  = substitute (tau, left));
        getweight (goal->right = substitute (tau, right));
        deleteterm ( left );
        deleteterm ( right );
        printf ( "Instanziere Ziel:    %4ld  ", goal->number );
        printtpair ( goal );
        printsubst ( tau );
        Proved ();
        return true;
    }
    return false;
}
