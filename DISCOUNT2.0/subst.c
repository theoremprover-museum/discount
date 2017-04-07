/*
//=============================================================================
//      Projekt:        TEAM-COMPLETION
//
//      Modul:          subst
//
//      Author:         Werner Pitz, 1991
//
//      Beschreibung:   Substitutionen
//-----------------------------------------------------------------------------
//      $Log: subst.c,v $
//      Revision 0.2  1991/08/21  07:40:28  pitz
//      Kopieren von Substitutionen durch copysubst.
//
//      Revision 0.1  1991/08/19  09:50:26  pitz
//      Fehlermeldungen mit Dateiangabe.
//
//      Revision 0.0  1991/08/09  08:18:19  pitz
//      Initiale Version
//
//=============================================================================
*/

#include    <stdio.h>

#include    "memory.h"
#include    "subst.h"



/*
//-----------------------------------------------------------------------------
//      lokale Funktionsdefinitionen
//-----------------------------------------------------------------------------
*/

#ifdef  ANSI

    static subst    *newmatch ( variable code, term *t );

    static void     deletes ( subst *cell );
    static void     deletem ( subst *cell );

    static bool     hcombine ( subst *sigma, subst *tau );

    static void     sadd    ( subst **sigma, subst *cell );
    static void     sunion  ( subst **sigma, subst *tau  );

    static void     addmatch ( subst **sigma, variable vcode, term *t );

    static bool     cunify  ( term *t1, term *t2 );
    static bool     dunify  ( term *t1, term *t2, subst **sigma );
    static bool     cmatch  ( term *t1, term *t2 );
    static bool     dmatch  ( term *t1, term *t2, subst **sigma );

#endif


/*
//-----------------------------------------------------------------------------
//      Speicherverwaltung fuer Substitutionen
//-----------------------------------------------------------------------------
*/

static subst *freelist = NULL;


/*
//-----------------------------------------------------------------------------
//  Funktion:       initsubst
//
//  Parametert:     -keine-
//
//  Beschreibung:   Loeschen der Freiliste
//-----------------------------------------------------------------------------
*/

void    initsubst (void)
{
    freelist = NULL;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       newsubst ( variable code, term *t )
//
//  Parameter:      code    Code fuer die Variable
//                  t       Zeiger auf assozierten Term
//
//  Rueckgabewert:  Zeiger auf einen neuen Knoten fuer subst
//
//  Beschreibung:   Anlegen eines neuen Substitutionsknoten.
//                  Unbelegte Knotenelemente werden aus freelist gewonnen.
//                  Sind keine freien Knotenelemente vorhanden, so werden
//                  SUBSTALLOC neue Elemente allociert.
//-----------------------------------------------------------------------------
*/

subst *newsubst ( variable code, term *t )
{
    register subst      *ptr, *next;
    register short      i;
             long       size;

    if (!freelist)
    {
        size = SUBSTALLOC * sizeof (subst);
        ptr = freelist = Allocate ( size );
        if (!ptr)
            Error ( __FILE__ ": "  "newsubst", "Zuwenig Speicher fuer Substitution." );

        for ( i = 1; i < SUBSTALLOC; i++ )
        {
            next = ptr;
            #ifdef MEMDEBUG
               ptr->debug = 0;
            #endif
            ptr = ptr->lson = ++next;
        }
        ptr->lson = NULL;
    }

    ptr = freelist;
    freelist = freelist->lson;
    ptr->code = code;
    ptr->tptr = copyterm ( t );
    ptr->lson = NULL;
    ptr->rson = NULL;

    #ifdef MEMDEBUG
       if (ptr->debug)
           Error ( __FILE__ ": "  "newsubst", "MEMORY-ERROR." );
       ptr->debug++;
    #endif 

    return ptr;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       newmatch ( variable code, term *t )
//
//  Parameter:      code    Code fuer die Variable
//                  t       Zeiger auf assozierten Term
//
//  Rueckgabewert:  Zeiger auf einen neuen Knoten fuer subst
//
//  Beschreibung:   Anlegen eines neuen Substitutionsknoten.
//                  Unbelegte Knotenelemente werden aus freelist gewonnen.
//                  Sind keine freien Knotenelemente vorhanden, so werden
//                  SUBSTALLOC neue Elemente allociert.
//
//  Beachte:        Im Gegensatz zu newsubst wird KEINE Kopie des Terms 
//                  assoziert.
//-----------------------------------------------------------------------------
*/

static subst *newmatch ( variable code, term *t )
{
    register subst      *ptr, *next;
    register short      i;
             long       size;

    if (!freelist)
    {
        size = SUBSTALLOC * sizeof (subst);
        ptr = freelist = Allocate ( size );
        if (!ptr)
            Error ( __FILE__ ": "  "newmatch", "Zuwenig Speicher fuer Substitution." );

        for ( i = 1; i < SUBSTALLOC; i++ )
        {
            next = ptr;
            #ifdef MEMDEBUG
               ptr->debug = 0;
            #endif
            ptr = ptr->lson = ++next;
        }
        ptr->lson = NULL;
    }

    ptr = freelist;
    freelist = freelist->lson;
    ptr->code = code;
    ptr->tptr = t;
    ptr->lson = NULL;
    ptr->rson = NULL;

    #ifdef MEMDEBUG
       if (ptr->debug)
           Error ( __FILE__ ": "  "newsubst", "MEMORY-ERROR." );
       ptr->debug++;
    #endif 

    return ptr;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       deletes ( subst *cell )
//
//  Parameter:      cell    ein Knoten
//
//  Beschreibung:   Das Knotenelement knot wird in die Freiliste elist
//                  umgekettet.
//  Seiteneffekt:   Der assozierte Term wird freigegeben.
//-----------------------------------------------------------------------------
*/

static void deletes ( subst *cell )
{
    #ifdef MEMDEBUG
       cell->debug--;
       if (cell->debug)
           Error ( __FILE__ ": "  "deletes", "MEMORY-ERROR." );
    #endif

    deleteterm ( cell->tptr );
    cell->lson = freelist;
    freelist = cell;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       deletem ( subst *cell )
//
//  Parameter:      cell    ein Knoten
//
//  Beschreibung:   Das Knotenelement knot wird in die Freiliste elist
//                  umgekettet. 
//                  Der angekettete Term wird dabei NICHT gelscht !
//
//  Seiteneffekt:   Der assozierte Term wird NICHT freigegeben.
//-----------------------------------------------------------------------------
*/

static void deletem ( subst *cell )
{
    #ifdef MEMDEBUG
       cell->debug--;
       if (cell->debug)
           Error ( __FILE__ ": "  "deletes", "MEMORY-ERROR." );
    #endif

    cell->lson = freelist;
    freelist = cell;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       deletesubst ( subst *cell )
//
//  Parameter:      cell    ein Knoten
//
//  Beschreibung:   Rekusives freigeben einer Substitution
//-----------------------------------------------------------------------------
*/

void deletesubst ( subst *cell )
{
    if (cell)
    {
        deletesubst (cell->lson);
        deletesubst (cell->rson);
        deletes ( cell );
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       deletematch ( subst *cell )
//
//  Parameter:      cell    ein Knoten
//
//  Beschreibung:   Rekusives freigeben eines Matchs
//-----------------------------------------------------------------------------
*/

void deletematch ( subst *cell )
{
    if (cell)
    {
        deletematch (cell->lson);
        deletematch (cell->rson);
        deletem ( cell );
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       dassoc ( subst *sigma, variable code )
//
//  Parameter:      sigma   Substitution
//                  code    Nummer der Variablen
//
//  Rueckgabewert:  assozierter Term    falls vorhanden
//                  NULL                sonst
//
//  Beschreibung:   Die Substitution wird nach einer moeglichen Substitution
//                  der Variablen var durchsucht.
//                  Es wird der ECHTE assozierte Term geliefert -
//                                                      keine Kopie !!
//-----------------------------------------------------------------------------
*/

term    *dassoc ( subst *sigma, variable code )
{
    while (sigma)
    {
        if (sigma->code == code)
            return sigma->tptr;
        sigma = (code < sigma->code) ? sigma->lson
                                     : sigma->rson;
    }
    return NULL;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       assoc ( subst *sigma, variable code )
//
//  Parameter:      sigma   Substitution
//                  code    Nummer der Variablen
//
//  Rueckgabewert:  assozierter Term    falls vorhanden
//                  NULL                sonst
//
//  Beschreibung:   Die Substitution wird nach einer moeglichen Substitution
//                  der Variablen var durchsucht.
//-----------------------------------------------------------------------------
*/

term    *assoc ( subst *sigma, variable code )
{
    while (sigma)
    {
        if (sigma->code == code)
            return copyterm (sigma->tptr);
        sigma = (code < sigma->code) ? sigma->lson
                                     : sigma->rson;
    }
    return NULL;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       substitute ( subst *sigma, term *t )
//
//  Parameter:      sigma   Substitution
//                  t       Term
//
//  Rueckgabewert:  NEUER substituierter Term sigma(t)
//
//  Beschreibung:   Es wird rekursiv eine Termkopie angelegt,
//                  wobei alle assozierten Variablen substituiert werden.
//-----------------------------------------------------------------------------
*/

term    *substitute ( subst *sigma, term *t )
{
    term    *tptr;
    short   i;

    if (varp(t))
    {
        tptr = assoc (sigma, t->fcode);
        return (tptr) ? tptr
                      : newterm (t->fcode);
    }

    tptr = newterm (t->fcode);
    for ( i = 0; i < t->arity; i++ )
        tptr->argument[i] = substitute (sigma, t->argument[i]);
    return tptr;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       dsubstitute ( subst *sigma, term **t )
//
//  Parameter:      sigma   Substitution
//                  t       Term
//
//  Rueckgabewert:  substituierter Term sigma(t)
//
//  Beschreibung:   Es wird rekursiv eine Termsubstitution durchgefuehrt.
//                  Beachte, dass dies destruktiv durchgefuehrt wird.
//
//  Seiteneffekt:   t wird veraendert!
//-----------------------------------------------------------------------------
*/

term    *dsubstitute ( subst *sigma, term **t )
{
    term    *tptr;
    short   i;

    if (varp((*t)))
    {
        if ((tptr = assoc (sigma, (*t)->fcode)) != NULL)
        {
            deleteterm ( *t );
            *t = tptr;
        }
        return *t;
    }

    for ( i = 0; i < (*t)->arity;
        dsubstitute (sigma, &((*t)->argument[i++])));

    return *t;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       hcombine ( subst *sigma, subst *tau )
//
//  Parameter:      sigma   bisherige Substitution
//                  tau     zuzufuegende Substitution
//
//  Rueckgabewert:  true    Kombination ist moeglich
//                  false   Kombination ist nicht moeglich
//
//  Beschreibung:   Die Substitution tau wird auf sigma angewendet
//                  und hinzugefuegt.
//-----------------------------------------------------------------------------
*/

static bool hcombine ( subst *sigma, subst *tau )
{
    if (sigma)
    {
        dsubstitute ( tau, &(sigma->tptr) );
        if (occur (sigma->code, sigma->tptr) )
            return false;
        if (hcombine (sigma->lson, tau))
            return hcombine ( sigma->rson, tau );

        return false;
    }
    return true;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       sadd ( subst **sigma, subst *cell )
//
//  Parameter:      sigma   Substitution
//                  cell    Substitutionszelle
//
//  Beschreibung:   Die Zelle cell wird in sigma eingefuegt
//-----------------------------------------------------------------------------
*/

static void  sadd ( subst **sigma, subst *cell )
{
    variable    vcode;
    subst       **ptr;

    cell->lson = NULL;
    cell->rson = NULL;
    vcode = cell->code;
    ptr   = sigma;
    while (*ptr)
        ptr = (vcode < (*ptr)->code) ? &(*ptr)->lson
                                     : &(*ptr)->rson;
    *ptr = cell;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       sunion ( subst **sigma, subst *tau  )
//
//  Parameter:      sigma   Referenz auf Substitutionspointer
//                  tau     zuzufuegende Substitution
//
//  Beschreibung:   Vereinigt die Substitutionen sigma und tau in sigma
//
//  Seiteneffekt:   tau wird zerstoert
//-----------------------------------------------------------------------------
*/

static void  sunion ( subst **sigma, subst *tau  )
{
    if (tau)
    {
        sunion ( sigma, tau->lson );
        sunion ( sigma, tau->rson );
        sadd ( sigma, tau );
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       combine ( subst *sigma, subst *tau )
//
//  Parameter:      sigma   Referenz auf Substitutionspointer
//                  tau     zuzufuegende Substitution
//
//  Rueckgabewert:  true    Kombination ist moeglich
//                  false   Kombination ist nicht moeglich
//
//  Beschreibung:   Die Substitution tau wird auf sigma angewendet.
//-----------------------------------------------------------------------------
*/

bool    combine ( subst **sigma, subst *tau )
{
    if (tau == NULL)
        return true;

    if (!hcombine (*sigma, tau))
        return false;

    sunion ( sigma, tau );

    return true;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       addsubst ( subst **sigma, variable vcode, term *t )
//
//  Parameter:      sigma   Referenz auf Substitutionspointer
//                  vcode   neue Variable
//                  t       zugehoeriger Term
//
//  Beschreibung:   Die Substitution sigma wird um (vcode <- t) erweitert.
//-----------------------------------------------------------------------------
*/

void    addsubst ( subst **sigma, variable vcode, term *t )
{
    subst       **ptr;

    ptr = sigma;
    while (*ptr)
        ptr = (vcode < (*ptr)->code) ? &(*ptr)->lson
                                     : &(*ptr)->rson;
    *ptr = newsubst (vcode, t);
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       addmatch ( subst **sigma, variable vcode, term *t )
//
//  Parameter:      sigma   Referenz auf Substitutionspointer
//                  vcode   neue Variable
//                  t       zugehoeriger Term
//
//  Beschreibung:   Die Substitution sigma wird um (vcode <- t) erweitert.
//-----------------------------------------------------------------------------
*/

static void  addmatch ( subst **sigma, variable vcode, term *t )
{
    subst       **ptr;

    ptr = sigma;
    while (*ptr)
        ptr = (vcode < (*ptr)->code) ? &(*ptr)->lson
                                     : &(*ptr)->rson;
    *ptr = newmatch (vcode, t);
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       printsubst ( subst *sigma )
//
//  Parameter:      sigma   Substitutionspointer
//
//  Beschreibung:   Ausgabe einer Substitution.
//-----------------------------------------------------------------------------
*/

void    printsubst ( subst *sigma )
{
    if (sigma)
    {
        printsubst ( sigma->lson );
        printf ( "X%d <- ", -sigma->code );
        printterm ( sigma->tptr );
        printf ( "\n" );
        printsubst ( sigma->rson );
    }
}


/*
//=============================================================================
//      Unifikation
//=============================================================================
*/

/*
//-----------------------------------------------------------------------------
//  Funktion:       cunify ( term *t1, term *t2 )
//
//  Parameter:      t1, t2      Pointer auf zwei Terme
//
//  Rueckgabewert:  true    Unifikation von t1 in t2 ist moeglich.
//                  false   Unifikation ist nicht moeglich,
//                          da ein Clash auftritt
//
//  Beschreibung:   Hilfsfunktion fuer Unifikation
//----------------------------------------------------------------------------
*/

static bool cunify (term *t1, term *t2)
{
    short   i;

    if (varp (t1) || varp (t2))
        return true;

    if (t1->fcode != t2->fcode)
        return false;

    for (i = 0; i < t1->arity; i++ )
        if (!cunify (t1->argument[i],t2->argument[i]))
            return false;

    return true;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       dunify ( term *t1, term *t2, subst **sigma )
//
//  Parameter:      t1, t2      Pointer auf zwei Terme
//                  sigma       Pointer auf Substitution
//                              (muss NULL sein !!)
//
//  Rueckgabewert:  true    Unifikation war erfolgreich.
//                  false   Unifikation konnte nicht durchgefuehrt werden.
//
//  Beschreibung:   Destruktive Unifikation
//
//  Seiteneffekt:   Die Terme t1 und t2 werden moeglicherweise veraendert.
//----------------------------------------------------------------------------
*/

static bool  dunify ( term *t1, term *t2, subst **sigma )
{
    subst   *tau    = NULL;
    short   i       = 0;
    short   j       = 0;
    bool    result;

    if (varp (t1))
    {
        if (t1->fcode == t2->fcode)
            return true;

        if (occur ( t1->fcode, t2 ))
            return false;

        if (t1->fcode < t2->fcode)
            *sigma = newsubst ( t1->fcode, t2 );
        else
            *sigma = newsubst ( t2->fcode, t1 );

        return true;
    }

    if (varp (t2))
    {
        if (occur ( t2->fcode, t1 ))
            return false;

        *sigma = newsubst ( t2->fcode, t1 );
        return true;
    }

    if (t1->fcode != t2->fcode)
        return false;

    if (constp (t1))
        return true;

    result = true;
    i = 0;
    while ((i < t1->arity) && (result))
    {
        if ((result = dunify (t1->argument[i], t2->argument[i], &tau)) != 0)
        {
            if (tau)
            {
                for (j = i+1; j < t1->arity; j++ )
                {
                    dsubstitute ( tau, &(t1->argument[j]));
                    dsubstitute ( tau, &(t2->argument[j]));
                }
                if (!combine ( sigma, tau ))
                {
                    deletesubst ( tau );
                    return false;
                }
                tau = NULL;
            }
            i++;
        }
        else
        {
            deletesubst ( tau );
            tau = NULL;
        }
    }

    return result;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       unify ( term *t1, term *t2, subst *sigma )
//
//  Parameter:      t1, t2      Pointer auf zwei Terme
//                  sigma       Pointer auf Substitution
//
//  Rueckgabewert:  true    Unifikation war erfolgreich.
//                  false   Unifikation konnte nicht durchgefuehrt werden.
//
//  Beschreibung:   Unifikation
//                  Es werden Kopien der Terme erzeugt, und dann die
//                  destruktive Unifikations dunify aufgerufen
//-----------------------------------------------------------------------------
*/

bool    unify ( term *t1, term *t2, subst **sigma )
{
    term    *ct1, *ct2;
    bool    result;

    if (!cunify (t1,t2))
    {
        *sigma = NULL;
        return false;
    }

    ct1 = copyterm ( t1 );
    ct2 = copyterm ( t2 );

    result = dunify ( ct1, ct2, sigma );
    if (!result)
    {
        deletesubst ( *sigma );
        *sigma = NULL;
    }

    deleteterm ( ct1 );
    deleteterm ( ct2 );

    return result;
}


/*
//=============================================================================
//      Match
//=============================================================================
*/

/*
//-----------------------------------------------------------------------------
//  Funktion:       cmatch ( term *t1, term *t2 )
//
//  Parameter:      t1, t2      Pointer auf zwei Terme
//
//  Rueckgabewert:  true    Match von t1 in t2 ist moeglich.
//                  false   Match ist nicht moeglich,
//                          da ein Clash auftritt
//
//  Beschreibung:   Hilfsfunktion fuer Match
//                  Es wird ueberprueft, ob alle Stellen mit Funktionsymbolen
//                  in t1 mit denen in t2 uebereinstimme
//----------------------------------------------------------------------------
*/

static bool cmatch (term *t1, term *t2)
{
    short   i;

    if varp (t1)
        return true;

    if ((t1->fcode != t2->fcode) ||(t1->weight > t2->weight))
        return false;

    for (i = 0; i < t1->arity; i++ )
        if (!cmatch (t1->argument[i],t2->argument[i]))
            return false;

    return true;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       dmatch ( term *t1, term *t2, subst **sigma )
//
//  Parameter:      t1, t2      Pointer auf zwei Terme
//                  sigma       Bisherige Substitution
//
//  Rueckgabewert:  true    Match von t1 in t2 war erfolgreich, d.h. sigma(t1)=t2
//                  false   Match konnte nicht durchgefuehrt werden.
//
//  Beschreibung:   Hilfsfunktion fuer Match
//
//  Beachte:        Korrekter Match nur dann gesichert,
//                  falls t1 und t2 variablendisjunkt.
//----------------------------------------------------------------------------
*/

static bool  dmatch ( term *t1, term *t2, subst **sigma )
{
    term    *tx;
    short   i;

    if (varp (t1))
    {
        if (t1->fcode == t2->fcode)
            return true;

        if ((tx = dassoc (*sigma, t1->fcode)) != NULL)
            return equal ( tx, t2 );

        addmatch ( sigma, t1->fcode, t2 );
        return true;
    }
/* -------------------------------------- Ueberfluessig aufgrund von cmatch
    if (varp (t2))
        return false;

    if (t1->fcode != t2->fcode)
        return false;

    if (constp (t1))
        return true;
*/
    for ( i = 0; i < t1->arity; i++ )
        if (!dmatch (t1->argument[i], t2->argument[i], sigma) )
            return false;

    return true;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       match ( term *t1, term *t2, subst **sigma )
//
//  Parameter:      t1, t2      Pointer auf zwei Terme
//                  sigma       Pointer auf Substitution
//
//  Rueckgabewert:  true    Match von t1 in t2 war erfolgreich; d.h. sigma(t1)=t2
//                  false   Match konnte nicht durchgefuehrt werden.
//
//  Beschreibung:   Match
/----------------------------------------------------------------------------
*/

bool    match ( term *t1, term *t2, subst **sigma )
{
    bool    result;

    if (!cmatch (t1,t2))
    {
        deletematch ( *sigma );
        *sigma = NULL;
        return false;
    }

    result = dmatch ( t1, t2, sigma );
    if (!result)
    {
        deletematch ( *sigma );
        *sigma = NULL;
    }

    return result;
}


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  goal_in_goal_match                                       */
/*                                                                           */
/*  Parameter    :  zwei Ziele, die zu matchen versucht werden               */
/*                                                                           */
/*  Returnwert   :  true, falls ein Match vom ersten Ziel auf das zweite     */
/*                        existiert                                          */
/*                  false, falls nicht                                       */
/*                                                                           */
/*  Beschreibung :  Ziel1 u1 = v1 matcht Ziel2 u2 = v2 falls es eine Sub-    */
/*                  stitution sigma gibt mit                                 */
/*                     sigma( u1 ) = u2  und  sigma( v1 ) = v2               */
/*                                   oder                                    */
/*                     sigma( v1 ) = u2  und  sigma( u1 ) = v2               */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  SetOfGoals.first                                         */
/*                                                                           */
/*****************************************************************************/

bool goal_in_goal_match( termpair *goal1, termpair *goal2 )
{
  subst  *sigma = NULL;

  if( match( goal1->left, goal2->left, &sigma ) && 
      match( goal1->right, goal2->right, &sigma ) )
  {
    deletematch ( sigma );
    return true;
  }

  if( match( goal1->right, goal2->left, &sigma ) &&
      match( goal1->left, goal2->right, &sigma ) )
  {
    deletematch ( sigma );
    return true;
  }

  return false;
} /* Ende von goal_in_goal_match */


/*
//-----------------------------------------------------------------------------
//  Funktion:       copysubst ( subst *sigma )
//
//  Parameter:      sigma       Pointer auf Substitution
//
//  Rueckgabewert:  Kopie von sigma (inclusive Termkopien !)
//
//  Beschreibung:   Kopie einer SUbstitution erzeugen.
/----------------------------------------------------------------------------
*/

subst   *copysubst ( subst *sigma )
{
    subst    *tau;

    if (!sigma)
        return NULL;

    tau       = newsubst (sigma->code, sigma->tptr);
    tau->lson = copysubst (sigma->lson);
    tau->rson = copysubst (sigma->rson);

    return tau;
}



/*
//=============================================================================
//      Reorganisation
//=============================================================================
*/

REORG_FUNCTION(reorg,subst,lson)

void ReorgSubst ( bool silence )
{
    if (!silence)
        printf ( "Reorganisiere Substitutionen\n" );
    freelist = reorg ( freelist, STARTSHIFT );
}

