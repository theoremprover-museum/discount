/*
//=============================================================================
//      Projekt:        TEAM-COMPLETION
//
//      Modul:          order
//
//      Author:         Werner Pitz, 1991
//
//      Beschreibung:   Dieses Modul enthaelt die Implementierungen der
//                      Ordnungen.
//-----------------------------------------------------------------------------
//      $Log: order.c,v $
//      Revision 0.5  1992/01/28  08:45:00  pitz
//      kleine Fehlerkorrektur der letzten Fehlerkorrektur.
//
//      Revision 0.4  1992/01/28  08:05:12  pitz
//      Variablenbedingung bei XKBO korrigiert.
//
//      Revision 0.3  1991/10/29  09:46:26  pitz
//      xkbo ver"andert, Alte xkbo ist jetz wxkbo
//
//      Revision 0.2  1991/09/26  10:18:28  pitz
//      LPO mit Teiltermueberpruefung.
//      Dies beschleunigt den Termvergleich bei grossen Termen.
//
//      Revision 0.1  1991/08/19  09:49:54  pitz
//      Fehlermeldungen mit Dateiangabe.
//
//      Revision 0.0  1991/08/09  08:17:41  pitz
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
#include    "scanner.h"
#include    "order.h"


/*
//=============================================================================
//      Pointer auf aktuelle Bewertungsfunktion
//=============================================================================
*/

/*
//-----------------------------------------------------------------------------
//  Funktion-Ptr:   Compare ( term *t1, term *t1 )
//
//  Parameter:      t1, t2      zu vergleichende Terme
//
//  Rueckgabewert:  TLESS       falls t1 < t2
//                  TGREATER    falls t1 > t2
//                  TEQUAL      sonst
//
//  Beschreibung:   Vergleicht zwei Terme nach der aktuellen Ordnung
//-----------------------------------------------------------------------------
*/

short   (*Compare)     ( term *t1, term *t2 ) = NULL;


/*
//-----------------------------------------------------------------------------
//  Funktion-Ptr:   GreaterThan ( term *t1, term *t1 )
//
//  Parameter:      t1, t2      zu vergleichende Terme
//
//  Rueckgabewert:  true        falls t1 > t2
//                  false       sonst
//
//  Beschreibung:   Vergleicht zwei Terme nach der aktuellen Ordnung
//-----------------------------------------------------------------------------
*/

bool    (*GreaterThan) ( term *t1, term *t2 ) = NULL;


/*
//=============================================================================
//      Modulinterne Daten
//=============================================================================
*/

short   Ordering       = SUNKNOWN;


/*
//=============================================================================
//      Knuth Bendix Ordnung            - KBO
//=============================================================================
*/

#ifdef  ANSI

    static long     kbo_weight  ( term *t, vartree **vars );
    static bool     kbo_greater ( term *t1, term *t2 );
    static short    kbo_cmp     ( term *t1, term *t2 );

#endif


/*
//-----------------------------------------------------------------------------
//  Funktion:       kbo_weight ( term *t, vartree **vars )
//
//  Parameter:      t       zu untersuchender Term
//                  vars    bisheriger Variablenbaum
//
//  Rueckgabewert:  Gewicht des Terms nach KBO
//
//  Beschreibung:   Rekusives durchlaufen eines Terms zur Bestimmung
//                  des KBO-Gewichtes. Gleichzeitig werden die Vorkommen der
//                  Variablen gezaehlt.
//-----------------------------------------------------------------------------
*/

static long kbo_weight ( term *t, vartree **vars )
{
    short   i;
    long    sum;

    if (varp(t))
    {
        if ((VTfindinc (*vars, t->fcode)) == 0)
            VTadd ( vars, t->fcode, 1 );

        return KBO_ALPHA;
    }
    else
    {
        sum = Weight ( t->fcode );
        for ( i = 0; i < t->arity;
                     sum += kbo_weight (t->argument[i++], vars) );

        return sum;
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       kbo_greater ( term *t, vartree **vars )
//
//  Parameter:      t       zu untersuchender Term
//                  vars    bisheriger Variablenbaum
//
//  Rueckgabewert:  true    falls t1 >>kbo>> t2
//                  false   sonst
//-----------------------------------------------------------------------------
*/

static bool kbo_greater ( term *t1, term *t2 )
{
    vartree     *vt1 = NULL;
    vartree     *vt2 = NULL;
    long        w1, w2;
    bool        result;

    w1 = kbo_weight ( t1, &vt1 );
    w2 = kbo_weight ( t2, &vt2 );

    result = (w1 > w2) && (VTless (vt2, vt1));

    VTclear ( &vt1 );
    VTclear ( &vt2 );

    return result;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       kbo_cmp ( term *t1, erm *t1 )
//
//  Parameter:      t1, t2      zu vergleichende Terme
//
//  Rueckgabewert:  TLESS       falls t1 <<kbo<< t2
//                  TGREATER    falls t1 >>kbo>> t2
//                  TEQUAL      sonst
//
//  Beschreibung:   Vergleich zweier Terme nach Knuth-Bendix-Ordnung
//-----------------------------------------------------------------------------
*/

static short    kbo_cmp ( term *t1, term *t2 )
{
    vartree     *vt1 = NULL;
    vartree     *vt2 = NULL;
    long        w1, w2;
    short       result;

    #ifdef SHOWCP
        putchar ( 'o' );
        flush ();
    #endif

    w1 = kbo_weight ( t1, &vt1 );
    w2 = kbo_weight ( t2, &vt2 );

    if (w1 == w2)
        result = TEQUAL;
    else
    if (w1 > w2)
        result = VTless ( vt2, vt1 ) ? TGREATER
                                     : TEQUAL;
    else
        result = VTless ( vt1, vt2 ) ? TLESS
                                     : TEQUAL;

    VTclear ( &vt1 );
    VTclear ( &vt2 );

    return result;
}


/*
//=============================================================================
//      Lexikographische Pfad Ordnung   - LPO
//=============================================================================
*/

#ifdef  ANSI

    static bool     lpo_greater ( term *t1, term *t2 );
    static short    lpo_cmp     ( term *t1, term *t2 );
    static function MaxFunc     ( term *t );

#endif


/*
//-----------------------------------------------------------------------------
//  Funktion:       MaxFunc ( term *t )
//
//  Parameter:      t       Term
//
//  Rueckgabewert:  Maximales Funktionssymbol bzgl. LPO-Praezedenz
//
//  Beschreibung:   Suche den max. Funktionssymbol bzgl. LPO-Praezedenz
//-----------------------------------------------------------------------------
*/

static function  MaxFunc ( term *t )
{
    function    m  = MAXFUNCTION+1;
    function    mn = 0;
    short       i;

    if (funcp (t))
    {
        m = Function[t->fcode].preorder;
        for (i = 0; i < t->arity; i++ )
        {
            mn = MaxFunc ( t->argument[i] );
            m  = min (m, mn);
        }
    }
    return m;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       lpo_greater ( term *t1, term *t2 );
//
//  Parameter:      t1, t2  zu vergleichende Terme
//
//  Rueckgabewert:  true    falls t1  >>lpo>>  t2
//                  false   sonst
//
//  Beschreibung:   Ueberprueft, ob t1 groesser ist als t2
//
//-----------------------------------------------------------------------------
*/

bool    lpo_greater ( term *t1, term *t2 )
{
    short   i, j;

    if (varp (t1))
        return false;

    if (varp (t2))
        return occur ( t2->fcode, t1 );

    for (i = 0; i < t1->arity; i++)
        if (member (t2, t1->argument[i]))
            return true;


    if (LPOFGreater(t1, t2))
    {
        for ( j = 0; j < t2->arity; j++ )
            if (!lpo_greater (t1, t2->argument[j]))
                return false;
        return true;
    }
    else if (t1->fcode == t2->fcode)
    {
        for ( i = 0;
              ((i < t1->arity) && equal (t1->argument[i], t2->argument[i]));
              i++ );

        if (i >= t1->arity)
            return false;

        if (!lpo_greater (t1->argument[i], t2->argument[i]))
        {
            for ( i = 0; i < t1->arity; i++ )
                if (lpo_greater (t1->argument[i], t2) ||
                          equal (t1->argument[i], t2) )
                    return true;

            return false;
        }

        for ( j = 0; j < t2->arity; j++ )
            if (!lpo_greater (t1, t2->argument[j]))
            {
                for ( i = 0; i < t1->arity; i++ )
                    if (lpo_greater (t1->argument[i], t2) ||
                              equal (t1->argument[i], t2) )
                        return true;

                return false;
            }
        return true;
    }

    for ( i = 0; i < t1->arity; i++ )
        if (lpo_greater (t1->argument[i], t2) ||
                  equal (t1->argument[i], t2) )
            return true;

    return false;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       lpo_cmp ( term *t1, term *t1 )
//
//  Parameter:      t1, t2      zu vergleichende Terme
//
//  Rueckgabewert:  TLESS       falls t1 <<lpo<< t2
//                  TGREATER    falls t1 >>lpo>> t2
//                  TEQUAL      sonst
//
//  Beschreibung:   Vergleich zweier Terme nach LPO
//-----------------------------------------------------------------------------
*/

short   lpo_cmp ( term *t1, term *t2 )
{
/*
    function    ft1 = 0;
    function    ft2 = 0;

    ft1 = -MaxFunc (t1);
    ft2 = -MaxFunc (t2);

    if (ft1 > ft2)
        return (lpo_greater (t1, t2)) ? TGREATER
                                      : TEQUAL;

    if (ft2 > ft1)
        return (lpo_greater (t2, t1)) ? TLESS
                                      : TEQUAL;
*/
    #ifdef SHOWCP
        putchar ( 'o' );
        flush ();
    #endif

    return (lpo_greater (t1, t2)) ? TGREATER
                                  : (lpo_greater (t2, t1)) ? TLESS
                                                           : TEQUAL;
}


/*
//=============================================================================
//      Rekursive Pfad Ordnung   - RPO
//=============================================================================
*/

#ifdef  ANSI

    static bool     rpo_greater ( term *t1, term *t2 );
    static short    rpo_cmp     ( term *t1, term *t2 );

#endif



/*
//-----------------------------------------------------------------------------
//  Funktion:       rpo_greater ( term *t1, term *t2 );
//
//  Parameter:      t1, t2  zu vergleichende Terme
//
//  Rueckgabewert:  true    falls t1  >>rpo>>  t2
//                  false   sonst
//
//  Beschreibung:   Ueberprueft, ob t1 groesser ist als t2
//
//-----------------------------------------------------------------------------
*/

bool    rpo_greater ( term *t1, term *t2 )
{
    short   i, j;
    term    *left, *right;

    if (varp (t1))
        return false;

    if (varp (t2))
        return occur ( t2->fcode, t1 );

    for ( i = 0; i < t1->arity; i++ )
        if (rpo_greater (t1->argument[i], t2) ||
                  equal (t1->argument[i], t2) )
            return true;

    if (RPOFGreater(t2, t1))
        return false;

    if (RPOFGreater(t1, t2))
    {
        for ( j = 0; j < t2->arity; j++ )
            if (!rpo_greater (t1, t2->argument[j]))
                return false;

        return true;
    }

    if (t1->arity == 0)
        return false;

    if (t1->arity == 1)
        return rpo_greater (t1->argument[1], t2->argument[1]);


    left  = newterm (t1->fcode);
    right = newterm (t1->fcode);
    for (i = 0; i < t1->arity; i++ )
    {
        left->argument[i]  = t1->argument[i];
        right->argument[i] = t2->argument[i];
    }

    /*-------------------------------------*\
    |   Hier Multiset-Ordnung einfuegen !   |
    \*-------------------------------------*/

    deletet ( left );
    deletet ( right );
    return false;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       rpo_cmp ( term *t1, term *t1 )
//
//  Parameter:      t1, t2      zu vergleichende Terme
//
//  Rueckgabewert:  TLESS       falls t1 <<rpo<< t2
//                  TGREATER    falls t1 >>rpo>> t2
//                  TEQUAL      sonst
//
//  Beschreibung:   Vergleich zweier Terme nach rpo
//-----------------------------------------------------------------------------
*/

short   rpo_cmp ( term *t1, term *t2 )
{
    #ifdef SHOWCP
        putchar ( 'o' );
        flush ();
    #endif

    return (rpo_greater (t1, t2)) ? TGREATER
                                  : (rpo_greater (t2, t1)) ? TLESS
                                                           : TEQUAL;
}


/*
//=============================================================================
//      Erweiterte Knuth Bendix Ordnung - XKBO
//=============================================================================
*/

#ifdef  ANSI

    static bool     xkbo_gt      ( term *t1, term *t2 );
    static bool     xkbo_greater ( term *t1, term *t2 );
    static short    xkbo_cmp     ( term *t1, term *t2 );

#endif



/*
//-----------------------------------------------------------------------------
//  Funktion:       xkbo_gt ( term *t1, term *t2 );
//
//  Parameter:      t1, t2  zu vergleichende Terme
//
//  Rueckgabewert:  true    falls die lexikographische Anordnung in t1
//                          der Argumente groesser ist als in t2
//                  false   sonst
//
//  Beschreibung:   siehe Rueckgabewert
//-----------------------------------------------------------------------------
*/

static bool     xkbo_gt ( term *t1, term *t2 )
{
    short    i;

    if (LPOFGreater(t2, t1))
        return false;

    if (LPOFGreater(t1, t2))
        return true;

    for ( i = 0;
          ((i < t1->arity) && equal (t1->argument[i], t2->argument[i]));
          i++ );

    return (i < t1->arity) ? xkbo_greater (t1->argument[i], t2->argument[i])
                           : false;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       xkbo_greater ( term *t1, term *t2 );
//
//  Parameter:      t1, t2  zu vergleichende Terme
//
//  Rueckgabewert:  true    falls t1  >>xkbo>>  t2
//                  false   sonst
//
//  Beschreibung:   Ueberprueft, ob t1 groesser ist als t2
//
//-----------------------------------------------------------------------------
*/

bool   xkbo_greater ( term *t1, term *t2 )
{
    vartree     *vt1 = NULL;
    vartree     *vt2 = NULL;
    long        w1, w2;

    w1 = kbo_weight ( t1, &vt1 );
    w2 = kbo_weight ( t2, &vt2 );

    if (!VTless (vt2, vt1))
    {
        VTclear ( &vt1 );
        VTclear ( &vt2 );
        return false;
    }

    VTclear ( &vt1 );
    VTclear ( &vt2 );

    if (w1 > w2)
        return true;

    if ((w1 < w2) || (varp (t1)))
        return false;

    return xkbo_gt ( t1, t2 );
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       xkbo_cmp ( term *t1, term *t1 )
//
//  Parameter:      t1, t2      zu vergleichende Terme
//
//  Rueckgabewert:  TLESS       falls t1 <<xkbo<< t2
//                  TGREATER    falls t1 >>xkbo>> t2
//                  TEQUAL      sonst
//
//  Beschreibung:   Vergleich zweier Terme nach xkbo
//-----------------------------------------------------------------------------
*/

short   xkbo_cmp ( term *t1, term *t2 )
{
    vartree     *vt1 = NULL;
    vartree     *vt2 = NULL;
    long        w1, w2;
    short       result, res12, res21;

    #ifdef SHOWCP
        putchar ( 'o' );
        flush ();
    #endif

    w1 = kbo_weight ( t1, &vt1 );
    w2 = kbo_weight ( t2, &vt2 );

    if (w1 != w2)
    {
        if (w1 > w2)
            result = VTless ( vt2, vt1 ) ? TGREATER
                                         : TEQUAL;
        else
            result = VTless ( vt1, vt2 ) ? TLESS
                                         : TEQUAL;
        VTclear ( &vt1 );
        VTclear ( &vt2 );

        return result;
    }

    res12 = VTless ( vt1, vt2 );
    res21 = VTless ( vt2, vt1 );

    VTclear ( &vt1 );
    VTclear ( &vt2 );

    if (!(res12 || res21))
        return TEQUAL;

    if (res21 && xkbo_gt (t1, t2))
         return TGREATER;

    if (res12 && xkbo_gt (t2, t1))
         return TLESS;

    return TEQUAL;
}



/*
//=============================================================================
//      Erweiterte Knuth Bendix Ordnung - W-XKBO
//=============================================================================
*/

#ifdef  ANSI

    static bool     wxkbo_greater ( term *t1, term *t2 );
    static short    wxkbo_cmp     ( term *t1, term *t2 );

#endif



/*
//-----------------------------------------------------------------------------
//  Funktion:       wxkbo_greater ( term *t1, term *t2 );
//
//  Parameter:      t1, t2  zu vergleichende Terme
//
//  Rueckgabewert:  true    falls t1  >>wxkbo>>  t2
//                  false   sonst
//
//  Beschreibung:   Ueberprueft, ob t1 groesser ist als t2
//
//-----------------------------------------------------------------------------
*/

bool   wxkbo_greater ( term *t1, term *t2 )
{
    vartree     *vt1 = NULL;
    vartree     *vt2 = NULL;
    long        w1, w2;
    short       i;

    w1 = kbo_weight ( t1, &vt1 );
    w2 = kbo_weight ( t2, &vt2 );

    if (!VTless (vt2, vt1))
    {
        VTclear ( &vt1 );
        VTclear ( &vt2 );
        return false;
    }

    VTclear ( &vt1 );
    VTclear ( &vt2 );

    if (w1 > w2)
        return true;

    if ((w1 < w2) || (varp (t1)))
        return false;

    if (LPOFGreater(t2, t1))
        return false;

    if (LPOFGreater(t1, t2))
        return true;

    for ( i = 0;
          ((i < t1->arity) && equal (t1->argument[i], t2->argument[i]));
          i++ );

    return (i < t1->arity) ? wxkbo_greater (t1->argument[i], t2->argument[i])
                           : false;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       wxkbo_cmp ( term *t1, term *t1 )
//
//  Parameter:      t1, t2      zu vergleichende Terme
//
//  Rueckgabewert:  TLESS       falls t1 <<wxkbo<< t2
//                  TGREATER    falls t1 >>wxkbo>> t2
//                  TEQUAL      sonst
//
//  Beschreibung:   Vergleich zweier Terme nach wxkbo
//-----------------------------------------------------------------------------
*/

short   wxkbo_cmp ( term *t1, term *t2 )
{
    vartree     *vt1 = NULL;
    vartree     *vt2 = NULL;
    long        w1, w2;
    short       result;

    #ifdef SHOWCP
        putchar ( 'o' );
        flush ();
    #endif

    w1 = kbo_weight ( t1, &vt1 );
    w2 = kbo_weight ( t2, &vt2 );

    if (w1 != w2)
    {
        if (w1 > w2)
            result = VTless ( vt2, vt1 ) ? TGREATER
                                         : TEQUAL;
        else
            result = VTless ( vt1, vt2 ) ? TLESS
                                         : TEQUAL;
        VTclear ( &vt1 );
        VTclear ( &vt2 );

        return result;
    }

    VTclear ( &vt1 );
    VTclear ( &vt2 );

    return (wxkbo_greater (t1, t2)) ? TGREATER
                                    : (wxkbo_greater (t2, t1)) ? TLESS
                                                               : TEQUAL;
}


/*
//=============================================================================
//      Exportierte Funktionen
//=============================================================================
*/

/*
//-----------------------------------------------------------------------------
//  Funktion:       SetOrdering ( short ord )
//
//  Parameter:      ord     aktuelle Ordnung
//
//  Beschreibung:   Festlegung der aktuellen Ordnung
//-----------------------------------------------------------------------------
*/

void    SetOrdering ( short ord )
{
    short   i;

    if ((ord < 0) || (ord >= OrderCount))
        Error ( __FILE__ ": "  "SetOrdering",
                "Nummer der Ordnung ausserhalb des zugelassenen Bereichs." );

    Ordering = Order[ord].type;
    for (i = 0; i <= FuncCount; i++)
    {
        Function[i].weight   = Order[ord].weight[i];
        Function[i].preorder = Order[ord].preorder[i];
    }

    switch (Ordering)
    {
    case SKBO:  Compare     = kbo_cmp;
                GreaterThan = kbo_greater;
                break;

    case SLPO:  Compare     = lpo_cmp;
                GreaterThan = lpo_greater;
                break;

    case SXKBO: Compare     = xkbo_cmp;
                GreaterThan = xkbo_greater;
                break;
/*
    case SRPO:  Compare     = rpo_cmp;
                GreaterThan = rpo_greater;
                break;
*/
    default:    Error ( __FILE__ ": "  "SetOrdering", "Ordnung nicht implementiert." );
                break;
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       GetOrdering
//
//  Parameter:      -keine-
//
//  Rueckgabewert:  aktuelle Ordnung
//
//  Beschreibung:   Ermitteln der aktuellen Ordnung
//-----------------------------------------------------------------------------
*/

short   GetOrdering ( void )
{
    return Ordering;
}
