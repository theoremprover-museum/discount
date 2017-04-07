/*
//=============================================================================
//      Projekt:        TEAM-COMPLETION
//
//      Modul:          reduce
//
//      Author:         Werner Pitz, 1991
//
//      Beschreibung:   Reduktion und Normalformbildung
//-----------------------------------------------------------------------------
//      $Log: reduce.c,v $
//      Revision 0.1  1992/03/25  12:56:17  pitz
//      reduce_pair eingefuehrt.
//
//      Revision 0.0  1991/08/09  08:18:19  pitz
//      Initiale Version
//
//=============================================================================
*/

#include    <stdio.h>


#include    "reduce.h"
#include    "pcl.h"



/*
//-----------------------------------------------------------------------------
//      Sichtbare Daten
//-----------------------------------------------------------------------------
*/

long    RedCount;


/*
//-----------------------------------------------------------------------------
//      Modulinterne Definitionen
//-----------------------------------------------------------------------------
*/

#ifdef  ANSI

    static bool    creduce ( termpair *rule, term **t );
    static bool    rreduce ( termpair *rule, term **t );
    static bool    ereduce ( termpair *equ,  term **t );

#endif


/*
//-----------------------------------------------------------------------------
//  Funktion:       creduce ( termpair *rule, term **t )
//
//  Parameter:      rule    Regel mit der reduziert werden soll
//                  t       Referenz auf Pointer zu dem zu reduzierenden Term
//
//  Rueckgabewert:  true    Term konnte vereinfacht werden.
//                  false   Term konnte nicht vereinfacht werden.
//
//  Beschreibung:   Anwendung der Regel rule im Term t an Topstelle.
//-----------------------------------------------------------------------------
*/

static bool    creduce ( termpair *rule, term **t )
{
    subst   *sigma = NULL;

    if (match (rule->left, (*t), &sigma))
    {
        deletematch ( sigma );
        sigma = NULL;

        return true;
    }
    return false;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       rreduce ( termpair *rule, term **t )
//
//  Parameter:      rule    Regel mit der reduziert werden soll
//                  t       Referenz auf Pointer zu dem zu reduzierenden Term
//
//  Rueckgabewert:  true    Term konnte vereinfacht werden.
//                  false   Term konnte nicht vereinfacht werden.
//
//  Beschreibung:   Anwendung der Regel rule im Term t an Topstelle.
//-----------------------------------------------------------------------------
*/

static bool    rreduce ( termpair *rule, term **t )
{
    subst   *sigma = NULL;
    term    *save;

    if (match (rule->left, (*t), &sigma))
    {
        Referee_Red_Count ( rule );
        RedCount++;

        save = *t;
        getweight ( *t = substitute (sigma, rule->right) );
        deletematch ( sigma );
        sigma = NULL;
        deleteterm ( save );

        PCL_R_REDUCE (rule, *t);

        return true;
    }
    return false;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       ereduce ( termpair *equ, term **t )
//
//  Parameter:      equ     Gleichung mit der reduziert werden soll
//                  t       Referenz auf Pointer zu dem zu reduzierenden Term
//
//  Rueckgabewert:  true    Term konnte vereinfacht werden.
//                  false   Term konnte nicht vereinfacht werden.
//
//  Beschreibung:   Anwendung der Gleichung equ im Term t an Topstelle.
//-----------------------------------------------------------------------------
*/

static bool    ereduce ( termpair *equ, term **t )
{
    subst   *sigma = NULL;
    bool    result = false;
    term    *left, *right;

    if (match (equ->left, (*t), &sigma))
    {
        getweight (left  = substitute (sigma, equ->left));
        getweight (right = substitute (sigma, equ->right));
        deletematch ( sigma );
        sigma = NULL;
        if (GreaterThan (left, right))
        {
            Referee_Red_Count ( equ );
            RedCount++;

            deleteterm ( *t );
            getweight (*t = right);
            deleteterm ( left );
            PCL_E_REDUCE (equ, *t, true);
            return true;
        }
        else
        {
            deleteterm ( left );
            deleteterm ( right );
        }
    }

    if (match (equ->right, (*t), &sigma))
    {
        getweight (left  = substitute (sigma, equ->left));
        getweight (right = substitute (sigma, equ->right));
        deletematch ( sigma );
        sigma = NULL;
        if (GreaterThan (right, left))
        {
            Referee_Red_Count ( equ );
            RedCount++;

            deleteterm ( *t );
            getweight (*t = left);
            deleteterm ( right );
            PCL_E_REDUCE (equ, *t, false);
            result = true;
        }
        else
        {
            deleteterm ( left );
            deleteterm ( right );
        }
    }

    return result;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       reduce ( term **t )
//
//  Parameter:      t       Referenz auf Pointer zu dem zu reduzierenden Term
//
//  Rueckgabewert:  true    Term konnte vereinfacht werden.
//                  false   Term konnte nicht vereinfacht werden.
//
//  Beschreibung:   eigentliche Normalformbildung.
//                  inner-most Strategie.
//-----------------------------------------------------------------------------
*/

bool    reduce ( term **t )
{
    termpair    *rule, *equ;
    short       i;
    function    fcode;
    bool        result = false;

    if (funcp((*t)))
        for ( i = 0; i < (*t)->arity; i++ )
            result = reduce ( &((*t)->argument[i]) ) || result;

    if (result)
        lgetweight ( *t );

    fcode = (*t)->fcode;
    fcode = (fcode < 0) ? 0 : fcode;

    rule = SetOfRules[fcode].first;
    if (rule && (rule->weight > (*t)->weight))
       rule = NULL;

    equ  = SetOfEquations.first;
    if (equ && (equ->weight > (*t)->weight))
       equ = NULL;

    while ((rule) || (equ))
    {
        if (rule)
        {
            if (rreduce (rule, t))
            {
                if (funcp((*t)))
                {
                    for ( i = 0; i < (*t)->arity; i++ )
                        reduce ( &((*t)->argument[i]) );
                    lgetweight ( *t );
                }

                fcode = (*t)->fcode;
                fcode = (fcode < 0) ? 0 : fcode;
                rule   = SetOfRules[fcode].first;
                equ    = SetOfEquations.first;
                result = true;
            }
            else
            {
                rule = rule->next;
            }
            if (rule && (rule->weight > (*t)->weight))
                rule = NULL;
        }

        if (equ)
        {
            if (ereduce (equ, t))
            {
                if (funcp((*t)))
                {
                    for ( i = 0; i < (*t)->arity; i++ )
                        reduce ( &((*t)->argument[i]) );
                    lgetweight ( *t );
                }

                fcode = (*t)->fcode;
                fcode = (fcode < 0) ? 0 : fcode;
                rule   = SetOfRules[fcode].first;
                equ    = SetOfEquations.first;
                result = true;
            }
            else
            {
                equ = equ->next;
            }
            if (equ && (equ->weight > (*t)->weight))
                equ = NULL;
        }
    }
    return result;
}


/*----------------------------------------------------------------------------
**      Versuch einer anderen Reduktionsstrategie:
**      Regeln werden bevorzugt behandelt.

bool    reduce ( term **t )
{
    termpair    *rule, *equ;
    short       i;
    function    fcode;
    bool        result = false;
    bool        rflag;

    repeat:
    if (funcp((*t)))
        for ( i = 0; i < (*t)->arity; i++ )
            rflag = reduce ( &((*t)->argument[i]) ) || rflag;

    if (rflag)
    {
        lgetweight ( *t );
        result = true;
    }

    fcode = (*t)->fcode;
    fcode = (fcode < 0) ? 0 : fcode;

    rule = SetOfRules[fcode].first;
    while (rule && (rule->weight <= (*t)->weight))
    {
        if (rreduce (rule, t))
        {
            result = true;
            goto repeat;
        }
        rule = rule->next;
    }

    equ  = SetOfEquations.first;
    while (equ && (equ->weight <= (*t)->weight))
    {
        if (ereduce (equ, t))
        {
            result = true;
            goto repeat;
        }
        equ = equ->next;
    }

    return result;
}
*/
/*
//-----------------------------------------------------------------------------
//  Funktion:       Rreduce ( termpair *rule, term **t )
//
//  Parameter:      rule    Regel mit der reduziert werden soll
//                  t       Referenz auf Pointer zu dem zu reduzierenden Term
//
//  Rueckgabewert:  true    Term konnte vereinfacht werden.
//                  false   Term konnte nicht vereinfacht werden.
//
//  Beschreibung:   Reduziert den term t mit der Regel rule.
//                  inner-most Strategie.
//-----------------------------------------------------------------------------
*/

bool    Rreduce ( termpair *rule, term **t )
{
    short       i;
    bool        result = false;

    if (rule->weight > (*t)->weight)
        return false;

    if (funcp((*t)))
        for ( i = 0; i < (*t)->arity; i++ )
            result = Rreduce (rule,  &((*t)->argument[i])) || result;

    if (result)
        lgetweight ( *t );

    return rreduce (rule, t) || result;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       Ereduce ( termpair *equ, term **t )
//
//  Parameter:      equ     Gleichung mit der reduziert werden soll
//                  t       Referenz auf Pointer zu dem zu reduzierenden Term
//
//  Rueckgabewert:  true    Term konnte vereinfacht werden.
//                  false   Term konnte nicht vereinfacht werden.
//
//  Beschreibung:   Reduziert den term t mit der Gleichung equ.
//                  inner-most Strategie.
//-----------------------------------------------------------------------------
*/

bool    Ereduce ( termpair *equ, term **t )
{
    short       i;
    bool        result = false;

    if (equ->weight > (*t)->weight)
        return false;

    if (funcp((*t)))
        for ( i = 0; i < (*t)->arity; i++ )
            result = Ereduce ( equ,  &((*t)->argument[i]) ) || result;
    if (result)
        lgetweight ( *t );

    return result || ereduce (equ, t);
}

/*-----------------------------------------------------------------------------
                                                                           
Funktion        : bool    reduce_pair ( termpair *pair )

Autor           : Werner, stark ge"andert von StS

Beschreibung    : Bringt ein Termpaar auf Normalform, meldet, ob eine
Reduktion stattgefunden hat (?)

Globale Variable: viele (?)

Seiteneffekte   : ditto (?)

-----------------------------------------------------------------------------*/


bool    reduce_pair ( termpair *pair )
{
   bool ret=false;

   PCL_REDUCE_LEFT(pair);
   ret = ret | reduce (&(pair->left));
   PCL_REDUCE_RIGHT (pair);
   ret = ret | reduce (&(pair->right));
   return ret;
}



/*
//-----------------------------------------------------------------------------
//  Funktion:       CheckReduce ( term **t, termpair *pair1, termpair *pair2 )
//
//  Parameter:      t       zu testender Term
//                  pair1   bildendes Termpaar 1
//                  pair1   bildendes Termpaar 2
//
//  Rueckgabewert:  true    Term koennte vereinfacht werden.
//                  false   Term koennte nicht vereinfacht werden.
//
//  Beschreibung:   Diese Funtion versucht den Term t ohne die Regeln (Gl.)
//                  pair1 und pair2 zu reduziern.
//                  (Kapur-Kriterium!)
//-----------------------------------------------------------------------------
*/

bool    CheckReduce ( term **t, termpair *pair1, termpair *pair2 )
{
    termpair    *rule, *equ;
    short       i;
    function    fcode;

    fcode = (*t)->fcode;
    fcode = (fcode < 0) ? 0 : fcode;

    rule = SetOfRules[fcode].first;
    if (rule && (rule->weight > (*t)->weight))
       rule = NULL;
    while (rule)
    {
        if ((rule != pair1) && (rule != pair2) && (creduce (rule,t)))
            return true;

        rule = rule->next;
        if (rule && (rule->weight > (*t)->weight))
            rule = NULL;
    }

    if (funcp((*t)))
        for ( i = 0; i < (*t)->arity; i++ )
            if (CheckReduce ( &((*t)->argument[i]), pair1, pair2 ))
                return true;

    equ  = SetOfEquations.first;
    if (equ && (equ->weight > (*t)->weight))
       equ = NULL;
    while (equ)
    {
        if ((equ != pair1) && (equ != pair2) && (ereduce (equ,t)))
            return true;

        equ = equ->next;
        if (equ && (equ->weight > (*t)->weight))
            equ = NULL;
    }
    return false;
}

