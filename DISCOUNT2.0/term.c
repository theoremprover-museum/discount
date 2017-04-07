/*
//=============================================================================
//      Projekt:        TEAM-COMPLETION
//
//      Modul:          term
//
//      Author:         Werner Pitz, 1991
//
//      Beschreibung:   Alle Funktionen zur Verwaltung von Termen
//-----------------------------------------------------------------------------
//      $Log: term.c,v $
//      Revision 0.3 1998/03/26   19:08:43  brandt
//      depth: Termtiefe bestimmen
//
//      Revision 0.2  1991/08/19  09:50:37  pitz
//      Fehlermeldungen mit Dateiangabe.
//
//      Revision 0.1  1991/08/09  10:00:20  pitz
//      varoccur: Test, ob in einem Term eine (beliebige) Variable vorkommt.
//
//      Revision 0.0  1991/08/09  08:18:19  pitz
//      Initiale Version
//
//=============================================================================
*/


#include    "term.h"


/*
//-----------------------------------------------------------------------------
//      Modulinterne Datenbestaende
//-----------------------------------------------------------------------------
*/

function         FuncCount   = 0;
variable         VarCount    = 0;
short            ArityMax    = -1;
FunctionInfo     Function [MAXFUNCTION] = { { "X", -2, -1, -2, 0, 0L, NULL } };

short            OrderCount  = 0;
OrderInfo        Order    [MAXORDER];

long             Bit[32] = { 0x00000001L,0x00000002L,0x00000004L,0x00000008L,
                             0x00000010L,0x00000020L,0x00000040L,0x00000080L,
                             0x00000100L,0x00000200L,0x00000400L,0x00000800L,
                             0x00001000L,0x00002000L,0x00004000L,0x00008000L,
                             0x00010000L,0x00020000L,0x00040000L,0x00080000L,
                             0x00100000L,0x00200000L,0x00400000L,0x00800000L,
                             0x01000000L,0x02000000L,0x04000000L,0x08000000L,
                             0x10000000L,0x20000000L,0x40000000L,0x80000000L };

#ifdef BERLIN
static char      varstring[] = "VXYZUVW";
#else
static char      varstring[] = "Vxyzuvw";   /* Variablen fuer Pretty-Print  */
#endif

#define          varstrlen      6           /* Anzahl der Variablen         */


#ifdef  ANSI

    bool    ClearCPs ( bool msg );          /* Upcall in complet.c          */

#endif


/*
//-----------------------------------------------------------------------------
//  Funktion:       initterm
//
//  Parameter:      -keine-
//
//  Beschreibung:   Loeschen der Freilisten.
//-----------------------------------------------------------------------------
*/

void    initterm ( void )
{
    int    i;

    for (i = 0; i <= FuncCount; 
        Function[i++].freelist = NULL);
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       FindFunction ( char *ident )
//
//  Parameter:      ident       Identifier der Funktion
//
//  Rueckgabewert:  >0      Funktion mit Codenummer gefunden
//                  0       keine Funktion ident gefunden
//
//  Beschreibung:   Suchen einer Funktion anhand des Identifiers.
//-----------------------------------------------------------------------------
*/

function  FindFunction ( char *ident )
{
    short   i = 1;

    while (i <= FuncCount)
    {
        if (!strcmp (ident, Function[i].ident))
            return i;
        i++;
    }
    return 0;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       NewFunction ( char *ident )
//
//  Parameter:      ident       Identifier der Funktion
//
//  Rueckgabewert:  Codenummer der Funktion
//
//  Beschreibung:   Es wird eine neue Funktion vereinbart.
//-----------------------------------------------------------------------------
*/

function  NewFunction ( char *ident )
{
    if (FindFunction ( ident ))
        Error ( __FILE__ ": "  "NewFunction", "Funktion bereits definiert." );

    if (++FuncCount >= MAXFUNCTION)
        Error ( __FILE__ ": "  "NewFunction", "Zuviele Funktionssymbole." );

    strcpy ( Function[FuncCount].ident, ident );
    Function[FuncCount].weight       = -1;
    Function[FuncCount].preorder     = -1;
    Function[FuncCount].arity        = -1;
    Function[FuncCount].cancellation =  0;
    Function[FuncCount].freelist     = NULL;

    return FuncCount;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       SetArguments ( function fn, short arity )
//
//  Parameter:      fn          Codenummer der Funktion
//                  arity       Anzahl der Argumente
//
//  Beschreibung:   Anzahl der Argumente einer Funktion festlegen
//-----------------------------------------------------------------------------
*/

void    SetArguments ( function fn, short arity )
{
    if ((fn <= 0) || (fn > FuncCount))
        Error ( __FILE__ ": "  "SetArguments", "Funktionscode ausserhalb des Bereichs." );

    if (Function[fn].arity == -1)
    {
       Function[fn].arity = arity;
       ArityMax = max(arity, ArityMax);
    }
    else
    if (Function[fn].arity != arity)
    {
        printf ( "******  Funktion: %s\n", Function[fn].ident );
        Error ( __FILE__ ": "  "SetArguments",
                "Funktion mit unterschiedlicher Stelligkeit definiert." );
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       SetCancellation ( function fn, short cancellcation )
//
//  Parameter:      fn            Codenummer der Funktion
//                  cancellation  Argument fuer Cancelation
//
//  Beschreibung:   Legt ein Funktionssymbol als kuerzbar fest, falls
//                  Terme sich nur im Argument cancellation unterscheidet.
//-----------------------------------------------------------------------------
*/

void    SetCancellation ( function fn, short cancellation )
{
    cancellation--;
    if ((fn <= 0) || (fn > FuncCount))
        Error ( __FILE__ ": "  "SetCancellation", "Funktionscode ausserhalb des Bereichs." );

    if ((cancellation < 0) || (cancellation > 31))
        Error ( __FILE__ ": "  "SetCancellation", "Cancellation bis max. 32. Argument." );

    if (CheckCancel (fn, cancellation))
          printf ( "******  Warnung: Cancellation %s fuer %d mehrfach.\n",
                   Function[fn].ident, cancellation );

    Function[fn].cancellation |= Bit[cancellation];
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       SetOrdertype ( short number, short type )
//
//  Parameter:      number      Nummer der Ordnung
//                  type        Art der Ordnung
//
//  Beschreibung:   Auswahl der Ordnung
//-----------------------------------------------------------------------------
*/

void    SetOrdertype ( short number, short type )
{
    short   i;

    Order[number].type = type;
    for (i = 1; i < MAXFUNCTION; i++)
    {
        Order[number].weight[i]   = -1;
        Order[number].preorder[i] = -1;
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       SetWeight ( function fn,
//                              short number, short preorder, short weight )
//
//  Parameter:      fn          Codenummer der Funktion
//                  number      Nummer der Ordnung
//                  preorder    Code fuer Praezedenz auf Funktionssymbolen
//                  weight      Gewicht des Funktionssymbols
//
//  Beschreibung:   Anzahl der Argumente einer Funktion festlegen
//-----------------------------------------------------------------------------
*/

void    SetWeight ( function fn, short number, short preorder, short weight )
{
    if ((number < 0) || (number >= OrderCount))
        Error ( __FILE__ ": "  "SetWeight",
                "Nummer der Ordnung ausserhalb des zugelassenen Bereichs." );

    if ((fn <= 0) || (fn > FuncCount))
        Error ( __FILE__ ": "  "SetWeight", "Funktionscode ausserhalb des Bereichs." );

    if (Order[number].weight[fn] == -1)
    {
        Order[number].preorder[fn] = preorder;
        Order[number].weight[fn]   = weight;
    }
    else
        Error ( __FILE__ ": "  "SetWeight", "Gewicht der Funktion bereits definiert." );
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       CheckFunctions
//
//  Parameter:      -keine-
//
//  Rueckgabewert:  true    Alle Informationen vollstaendig
//                  false   Fehlende Informationen
//
//  Beschreibung:   Pruefen ob die Argumentzahl bei allen Funktionen vorhanden.
//-----------------------------------------------------------------------------
*/

bool    CheckFunctions ()
{
    short   i;
    bool    result  = true;

    for (i = 1; i <= FuncCount; i++ )
        if (Function[i].arity < 0)
        {
          printf ( "******  Warnung: %s wird in den Gleichungen nicht benutzt.\n",
                   Function[i].ident );
          result    = false;
        }

    return result;
}



/*
//-----------------------------------------------------------------------------
//  Funktion:       PrintFunctions
//
//  Parameter:      -keine-
//
//  Beschreibung:   Ausgabe aller Infomationen zu den Funktionen
//-----------------------------------------------------------------------------
*/

void    PrintFunctions ()
{
    short   i;

    printf ( "\n" );
    printf ( "----------------------------------------" );
    printf ( "---------------------------------------\n" );
    for (i = 1; i <= FuncCount; i++ )
    {
        printf ( "    %-26s\targs = %4d\tweight = %4d\n",
                 Function[i].ident,
                 Function[i].arity,
                 Function[i].weight );
    }
    printf ( "----------------------------------------" );
    printf ( "---------------------------------------\n\n" );
}



/*
//-----------------------------------------------------------------------------
//  Funktion:       newterm  ( function fcode )
//
//  Parameter:      fcode   Codenummer der Funktion/Variable
//                          > 0     Funktion
//                          < 0     Variable
//
//  Rueckgabewert:  Pointer auf Termzelle
//
//  Beschreibung:   Pointer auf Term fuer Funktion fcode.
//                  (fcode und arity werden initialisiert)
//                  Allokiert werden immer meherer Zellen, die dann
//                  in eine Freiliste an der Funktionsinformation
//                  angekettet werden.
//                  Dadurch wird eine Vielzahl von malloc-Aufrufen
//                  vermieden.
//-----------------------------------------------------------------------------
*/

term    *newterm ( function fcode )
{
    register term       *ptr, *next;
    register short      i;
             function   acode;
             long       acount;
             long       asize;

    acode = max(0, fcode);

    if (!FreeList(acode))
    {
        if (fcode <= 0)
        {
            acount = VARALLOC;
            asize  =   sizeof (term)
                     + sizeof (term *);
        }
        else
        {
            if (Arity(fcode) < 0)
                Error ( __FILE__ ": "  "newterm",
                        "Termallocate: Weniger als 0 Argumente" );

            acount = TERMALLOC;
            asize  =   sizeof (term)
                     + max(1, Arity(fcode)) * sizeof (term *);
        }
        asize  = ((asize+ALIGN-1)/ALIGN) * ALIGN;
        ptr = Function[acode].freelist = Allocate ( (long)(acount) * asize );
        if (!ptr)
        {
            if (ClearCPs ( true ))
                return newterm ( fcode );

            Error ( __FILE__ ": "  "newterm", "Zuwenig Speicher." );
        }

        for ( i = 1; i < acount; i++ )
        {
            next = (term *)((long)ptr + asize);
            #ifdef MEMDEBUG
               ptr->debug = 0;
            #endif
            ptr  = ptr->argument[0] = next;
        }
        ptr->argument[0] = NULL;
    }

    ptr = FreeList(acode);
    FreeList(acode) = ptr->argument[0];
    ptr->fcode      = fcode;
    ptr->arity      = Arity(acode);

    #ifdef MEMDEBUG
       if (ptr->debug)
       {
           printf ( "debug-value newterm (%ld) %ld\n", acode, ptr->debug );
           Error ( __FILE__ ": "  "newterm", "MEMORY-ERROR." );
       }
       ptr->debug++;
    #endif

    return ptr;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       deletet  ( term *t )
//
//  Parameter:      t       Pointer auf TermZelle
//
//  Beschreibung:   EINE Termzelle wird in die Freiliste zurueckgekettet.
//-----------------------------------------------------------------------------
*/

void    deletet ( term *t )
{
    short   acode;

    #ifdef MEMDEBUG
       t->debug--;
       if (t->debug)
           Error ( __FILE__ ": "  "deletet", "MEMORY-ERROR." );
    #endif

    acode   = max(t->fcode, 0);
    t->argument[0] = FreeList(acode);
    FreeList(acode) = t;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       deleteterm  ( term *t )
//
//  Parameter:      t       Pointer auf Term
//
//  Beschreibung:   Alle Termzellen eines Terms werden rekursiv freigegeben.
//-----------------------------------------------------------------------------
*/

void    deleteterm ( term *t )
{
    short   i;

    if (t->arity > 0)
        for ( i = 0; i < t->arity; deleteterm (t->argument[i++]) );
    deletet ( t );
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       copyterm  ( term *t )
//
//  Parameter:      t       Pointer auf Term
//
//  Rueckgabewert:  Zeiger auf echte Kopie des Terms
//
//  Beschreibung:   Es wird eine echte Kopie eines Terms aufgebaut.
//-----------------------------------------------------------------------------
*/

term    *copyterm ( term *t )
{
    term        *ptr;
    short       i;
    variable    fcode;

    ptr = newterm (fcode = t->fcode);

    if  (fcode > 0)
        for ( i = 0; i < t->arity; i++ )
            ptr->argument[i] = copyterm (t->argument[i]);

    return ptr;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       occur ( variable var, term *t )
//
//  Parameter:      var         Kennummer der Variablen
//                  t           Pointer auf zu untersuchenden Term
//
//  Rueckgabewert:  true    Variable var kommt in t vor
//                  false   Variable var kommt nicht in t vor
//
//  Beschreibung:   Occurence-Check
//-----------------------------------------------------------------------------
*/

bool  occur  ( variable var, term *t )
{
    short   i;

    if (t->fcode == var)
        return true;

    for (i = 0; i < t->arity; i++ )
        if (occur (var, t->argument[i]))
            return true;

    return false;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       foccur ( function f, term *t )
//
//  Parameter:      f           Kennummer der Funktion
//                  t           Pointer auf zu untersuchenden Term
//
//  Rueckgabewert:  true    Variable var kommt in t vor
//                  false   Variable var kommt nicht in t vor
//
//  Beschreibung:   Occurence-Check
//-----------------------------------------------------------------------------
*/

bool  foccur  ( function f, term *t )
{
    short   i;

    if (t->fcode == f)
        return true;

    for (i = 0; i < t->arity; i++ )
        if (foccur (f, t->argument[i]))
            return true;

    return false;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       equal ( term *t1, term *t2 )
//
//  Parameter:      t1, t2  zu vergleichende Terme
//
//  Rueckgabewert:  true    Terme stimmen ueberein
//                  false   Terme stimmen nicht ueberein
//
//  Beschreibung:   Ueberpruefung auf Termgleichheit
//-----------------------------------------------------------------------------
*/

bool    equal ( term *t1, term *t2 )
{
    short   i;

    if ((t1->fcode != t2->fcode) || (t1->weight != t2->weight))
        return false;

    if (funcp (t1))
    {
        for (i = 0; i < t1->arity; i++ )
            if (!equal (t1->argument[i], t2->argument[i]))
                return false;
    }

    return true;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       member ( term *t1, term *t2 )
//
//  Parameter:      t1, t2  zu ueberpruefende Terme
//
//  Rueckgabewert:  true    t1 ist Teilterm von t2
//                  false   sonst
//
//  Beschreibung:   Ueberprueft, ob t1 ein Teilterm von t2 ist
//-----------------------------------------------------------------------------
*/

bool    member ( term *t1, term *t2 )
{
    short   i;

    if (t2->weight < t1->weight)
        return false;

    if (t2->weight == t1->weight)
        return equal (t2,t1);

    for (i = 0; i < t2->arity; i++)
        if (member (t1, t2->argument[i]))
           return true;

    return false;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       printterm  ( term *t )
//
//  Parameter:      t       Pointer auf Term
//
//  Beschreibung:   einfache Ausgabe eines Terms.
//-----------------------------------------------------------------------------
*/

void    printterm ( term *t )
{
    short   i = 0;

    if (varp(t))
    {
        printf ( "X%ld", (long)(-t->fcode) );
    }
    else
    {
        printf ( Function[t->fcode].ident );
        if (t->arity > 0)
        {
            printf ( " (" );
            while ( i+1 < t->arity )
            {
                printterm ( t->argument[i++] );
                printf ( "," );
            }
            printterm (t->argument[i]);
            printf ( ")" );
        }
    }
}

/*
//-----------------------------------------------------------------------------
//  Funktion:       depth ( term *t )
//
//  Parameter:      t1 Term
//
//  Rueckgabewert:  Tiefe des Terms
//
//  Beschreibung:   Liefert die max. Tiefe eines Terms
//-----------------------------------------------------------------------------
*/

long    depth ( term *t )
{
    long   i, d, max = -1;

    if (varp(t)) return 0; /* Kann weggelassen werden, wenn garantiert ist,
                              dass arity bei Variablen immer <= 0 ist */

    for (i = 0; i < t->arity; i++ )
    { 
         d = depth(t->argument[i]);
         if  (d > max) max = d;
    }

    return 1 + max;
}



/*
//=============================================================================
//      Pretty-Print
//=============================================================================
*/

/*
//-----------------------------------------------------------------------------
//  Funktion:       prettyprint ( term *t, vartree **vars, variable *counter )
//
//  Parameter:      t       Pointer auf Term
//                  vars    Pointer auf den aktuellen Variablenbaum
//                  counter Zaehler fuer Variablen
//
//  Beschreibung:   Verbesserte Ausgabe eines Terms.
//                  Hilsfunktion fuer prettyterm
//                  Wird auch fuer Ausgabe von Termpaaren exportiert
//-----------------------------------------------------------------------------
*/

void    prettyprint ( term *t, vartree **vars, variable *counter )
{
    short       i = 0;
    variable    v;

    if (varp(t))
    {
        if ((v = VTfind (*vars, t->fcode)) == 0)
            VTadd ( vars, t->fcode, v = ++*counter );


        if (v > varstrlen)
            printf ( "x%d", v-varstrlen );
        else
            printf ( "%c", varstring[v] );
    }
    else
    {
        printf ( Function[t->fcode].ident );
        if (t->arity > 0)
        {
            printf ( " (" );
            while ( i+1 < t->arity )
            {
                prettyprint ( t->argument[i++], vars, counter );
                printf ( "," );
            }
            prettyprint ( t->argument[i], vars, counter );
            printf ( ")" );
        }
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       fprettyprint ( FILE *stream,
//                                 term *t, vartree **vars, variable *counter )
//
//  Parameter:      stream  Ausgabestream
//                  t       Pointer auf Term
//                  vars    Pointer auf den aktuellen Variablenbaum
//                  counter Zaehler fuer Variablen
//
//  Beschreibung:   Verbesserte Ausgabe eines Terms.
//                  Hilsfunktion fuer prettyterm
//                  Wird auch fuer Ausgabe von Termpaaren exportiert
//                  Ausgabe aus Stream
//-----------------------------------------------------------------------------
*/

void    fprettyprint ( FILE *stream, term *t, vartree **vars, variable *counter )
{
    short       i = 0;
    variable    v;

    if (varp(t))
    {
        if ((v = VTfind (*vars, t->fcode)) == 0)
            VTadd ( vars, t->fcode, v = ++*counter );

        if (v > varstrlen)
#ifdef BERLIN
            fprintf ( stream, "X%d", v-varstrlen );
#else
            fprintf ( stream, "x%d", v-varstrlen );
#endif
        else
            fprintf ( stream, "%c", varstring[v] );
    }
    else
    {
        fprintf ( stream, Function[t->fcode].ident );
        if (t->arity > 0)
        {
            fprintf ( stream, " (" );
            while ( i+1 < t->arity )
            {
                fprettyprint ( stream, t->argument[i++], vars, counter );
                fprintf ( stream, "," );
            }
            fprettyprint ( stream, t->argument[i], vars, counter );
            fprintf ( stream, ")" );
        }
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       sprettylen   ( term *t, vartree **vars, variable *counter )
//
//  Parameter:      t       Pointer auf Term
//                  vars    Variablenbaum
//                  counter Zaehler fuer Variablen
//
//  Beschreibung:   Bestimmt die Laenge einer Ausgabe.
//-----------------------------------------------------------------------------
*/

long    sprettylen   ( term *t, vartree **vars, variable *counter )
{
    short       i       = 0;
    long        size    = 3;
    variable    v;

    if (varp(t))
    {
        if ((v = VTfind (*vars, t->fcode)) == 0)
            VTadd ( vars, t->fcode, v = ++*counter );

        size++;
        if (v > 9+varstrlen)
            size++;
        if (v > 99+varstrlen)
            size++;
    }
    else
    {
        size += strlen ( Function[t->fcode].ident ) + 1;
        for (i = 0; i < t->arity; i++)
            size += sprettylen ( t->argument[i], vars, counter )+1;
    }
    return size;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       sprettylen   ( term *t, vartree **vars, variable *counter )
//
//  Parameter:      ptr     Pointer auf Pufferbereich
//                  t       Pointer auf Term
//                  vars    Variablenbaum
//
//  Beschreibung:   Ausgabe eines Terms in einen Pufferbereich.
//-----------------------------------------------------------------------------
*/

void    sprettyprint ( char **ptr, term *t, vartree **vars )
{
    short       i = 0;
    variable    v;

    if (varp(t))
    {
        v = VTfind (*vars, t->fcode);

        if (v > varstrlen)
        {
            sprintf ( *ptr, "x%d", v-varstrlen );
            (*ptr)++;
        }
        else
        {
            sprintf ( *ptr, "%c", varstring[v] );
            (*ptr)++;
            if (v > 9+varstrlen)
                (*ptr)++;
            if (v > 99+varstrlen)
                (*ptr)++;
        }
    }
    else
    {
        sprintf ( *ptr, "%s",Function[t->fcode].ident );
        (*ptr) += strlen ( Function[t->fcode].ident );
        if (t->arity > 0)
        {
            sprintf ( *ptr, " (" );
            (*ptr) += 2;
            while ( i+1 < t->arity )
            {
                sprettyprint ( ptr, t->argument[i++], vars );
                sprintf ( *ptr, "," );
                (*ptr)++;
            }
            sprettyprint ( ptr, t->argument[i], vars );
            sprintf ( *ptr, ")" );
            (*ptr)++;
        }
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       prettyterm  ( term *t )
//
//  Parameter:      t       Pointer auf Term
//
//  Beschreibung:   Verbesserte Ausgabe eines Terms.
//-----------------------------------------------------------------------------
*/

void    prettyterm ( term *t )
{
    vartree     *vars    = NULL;
    variable    counter  = 0;

    prettyprint ( t, &vars, &counter );
    VTclear ( &vars );
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       fprettyterm  ( term *t )
//
//  Parameter:      stream  Ausgabestream
//                  t       Pointer auf Term
//
//  Beschreibung:   Verbesserte Ausgabe eines Terms.
//-----------------------------------------------------------------------------
*/

void    fprettyterm ( FILE *stream, term *t )
{
    vartree     *vars    = NULL;
    variable    counter  = 0;

    fprettyprint ( stream, t, &vars, &counter );
    VTclear ( &vars );
}


/*
//=============================================================================
//      Variablenumbenennung
//=============================================================================
*/

/*
//-----------------------------------------------------------------------------
//  Funktion:       varoccur ( term *t )
//
//  Parameter:      t       Pointer auf Term
//
//  Rueckgabe:      true    falls in t eine Variable vorkommt
//                  false   sonst
//
//  Beschreibung:   Ueberprueft, ob im Term t eine Variable vorkommt.
//-----------------------------------------------------------------------------
*/

bool    varoccur  ( term *t )
{
    short   i;

    if (t->fcode < 0)
        return true;

    for (i = 0; i < t->arity; i++)
        if (varoccur (t->argument[i]))
            return true;

    return false;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       newvars ( term *t )
//
//  Parameter:      t       Pointer auf Term
//                  vars    Pointer auf den aktuellen Variablenbaum
//
//  Beschreibung:   Variablenumbennenung in einem Term.
//                  Alle Variablen in einem Term werden durch neu erzeugte
//                  Variablen ersetzt.
//-----------------------------------------------------------------------------
*/

void    newvars ( term *t, vartree **vars )
{
    short       i = 0;
    variable    v;

    if (varp(t))
    {
        if ((v = VTfind (*vars, t->fcode)) == 0)
            VTadd ( vars, t->fcode, v = NewVariable );

        t->fcode = v;
    }
    else
    {
        for ( i = 0; i < t->arity; newvars (t->argument[i++], vars) );
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       tnewvars  ( term *t )
//
//  Parameter:      t       Pointer auf Term
//
//  Beschreibung:   Variablenumbennenung in einem Term.
//                  Alle Variablen in einem Term werden durch neu erzeugte
//                  Variablen ersetzt.
//-----------------------------------------------------------------------------
*/

void    tnewvars ( term *t )
{
    vartree     *vars    = NULL;

    newvars ( t, &vars );
    VTclear ( &vars );
}


/*
//=============================================================================
//      Variablenerfassung
//=============================================================================
*/

/*
//-----------------------------------------------------------------------------
//  Funktion:       getvars ( term *t )
//
//  Parameter:      t       Pointer auf Term
//                  vars    Pointer auf den aktuellen Variablenbaum
//
//  Beschreibung:   Alle Variablen in einem Term werden ermittelt
//-----------------------------------------------------------------------------
*/

void    getvars ( term *t, vartree **vars )
{
    short   i = 0;

    if (varp(t))
    {
        if (!VTfind (*vars, t->fcode))
            VTadd ( vars, t->fcode, 1 );
    }
    else
    {
        for ( i = 0; i < t->arity; getvars (t->argument[i++], vars) );
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       tgetvars  ( term *t )
//
//  Parameter:      t       Pointer auf Term
//
//  Rueckgabewert:  Variablenbaum mit den Variablen in t
//
//  Beschreibung:   Alle Variablen in einem Term werden ermittelt
//-----------------------------------------------------------------------------
*/

vartree *tgetvars ( term *t )
{
    vartree     *vars    = NULL;

    getvars ( t, &vars );
    return vars;
}



/*
//=============================================================================
//      Termkopie mit Variablenumbenennung
//=============================================================================
*/

/*
//-----------------------------------------------------------------------------
//  Funktion:       copynew ( term *t, vartree **vars )
//
//  Parameter:      t       Pointer auf Term
//                  vars    Pointer auf den aktuellen Variablenbaum
//
//  Beschreibung:   Es wird eine Kopie eines Terms ausgebaut, die neue
//                  Variablen enthaelt.
//-----------------------------------------------------------------------------
*/

term    *copynew ( term *t, vartree **vars )
{
    term        *ptr;
    short       i;
    variable    v;

    ptr = newterm (t->fcode);

    if (varp(ptr))
    {
        if ((v = VTfind (*vars, ptr->fcode)) == 0)
            VTadd ( vars, ptr->fcode, v = NewVariable );

        ptr->fcode = v;
    }
    else
        for ( i = 0; i < t->arity; i++ )
            ptr->argument[i] = copynew (t->argument[i], vars);

    return ptr;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       tcopynew  ( term *t )
//
//  Parameter:      t       Pointer auf Term
//
//  Beschreibung:   Es wird eine Kopie eines Terms ausgebaut, die neue
//                  Variablen enthaelt.
//-----------------------------------------------------------------------------
*/

term    *tcopynew ( term *t )
{
    term        *ptr;
    vartree     *vars    = NULL;

    ptr = copynew ( t, &vars );
    VTclear ( &vars );

    return ptr;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       getweight  ( term *t )
//
//  Parameter:      t       Pointer auf Term
//
//  Beschreibung:   Bestimmt das Gewicht eines Termes an allen Stellen.
//-----------------------------------------------------------------------------
*/

long    getweight  ( term *t )
{
    short   i;
    long    weight = 1;

    if (funcp(t))
    {
        for ( i = 0; i < t->arity; 
              weight += getweight (t->argument[i++]) );
        weight++;
    }

    return t->weight = weight;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       lgetweight  ( term *t )
//
//  Parameter:      t       Pointer auf Term
//
//  Beschreibung:   Bestimmt das Gewicht eines Termes an einer Stelle.
//-----------------------------------------------------------------------------
*/

long    lgetweight  ( term *t )
{
    short   i;
    long    weight = 1;

    if (funcp(t))
    {
        for ( i = 0; i < t->arity; 
              weight += t->argument[i++]->weight );
        weight++;
    }

    return t->weight = weight;
}



long     testweight ( term *t, bool *flag )
{
    short   i;
    long    weight = 1;

    if (funcp(t))
    {
        for ( i = 0; flag && (i < t->arity); 
              weight += testweight (t->argument[i++], flag) );
        weight++;
    }

    *flag &= (t->weight == weight);
    return weight;
}



bool    checkweight  ( term *t )
{
    bool    result = true;
    
    testweight ( t, &result );

    return !result;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       count_func, count_var
//
//  Parameter:      t       Pointer auf Term
//
//  Beschreibung:   Bestimmt die Anzahl der Funktionen/Variablen in einem Term
//-----------------------------------------------------------------------------
*/

short   count_func ( term *t )
{
    short   res = 1;
    short   i;

    if (varp(t))
        return 0;

    for ( i = 0; i < t->arity; 
          res += count_func (t->argument[i++]) );

    return res;
}


short   count_var ( term *t )
{
    short   res = 0;
    short   i;

    if (varp(t))
        return 1;

    for ( i = 0; i < t->arity;
          res += count_var (t->argument[i++]) );

    return res;
}


/*
//=============================================================================
//      Reorganisation
//=============================================================================
*/

REORG_FUNCTION(reorg,term,argument[0])


void ReorgTerm ( bool silence )
{
    short   i;

    if (!silence)
        printf ( "Reorganisiere Termzellen: " );
    for (i =0; i <= FuncCount; i++)
    {
        if (!silence)
            printf ( "%s ", Function[i].ident );
        Function[i].freelist = reorg ( Function[i].freelist, STARTSHIFT );
    }
    if (!silence)
    putchar ('\n');
}



