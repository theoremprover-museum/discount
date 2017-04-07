/*
//=============================================================================
//      Projekt:        TEAM-COMPLETION
//
//      Modul:          parser
//
//      Author:         Werner Pitz, 1991
//
//      Beschreibung:   Einlesen einer Aufgabenstellung
//-----------------------------------------------------------------------------
//      $Log: parser.c,v $
//      Revision 0.9  1992/03/25  13:11:13  pitz
//      weight_flag standartmaessig als true initialisiert.
//
//      Revision 0.8  1992/03/25  13:06:35  pitz
//      Bewertung erfolgt nach einlesen des Ziels !
//      GOAL***
//
//      Revision 0.7  1991/10/30  09:29:38  pitz
//      WKBO einlesen.
//
//      Revision 0.6  1991/09/24  12:18:39  pitz
//      weight_flag ermoeglicht die Bewertung der initialen Gleichungen.
//
//      Revision 0.5  1991/09/11  08:28:51  pitz
//      An neues SetScanMode angepasst.
//
//      Revision 0.4  1991/08/26  15:29:23  pitz
//      Einlesen von SPECIAL Gleichungen.
//
//      Revision 0.3  1991/08/19  10:32:47  pitz
//      Skolemgoal wird richtig gesetzt!
//
//      Revision 0.2  1991/08/19  09:50:01  pitz
//      Fehlermeldungen mit Dateiangabe.
//
//      Revision 0.1  1991/08/09  10:08:01  pitz
//      Die Variable SkolemGoal wird beim Einlesen eines Ziels gesetzt.
//      Sie wird true, falls keine Variable im Ziel vorkommt.
//
//      Revision 0.0  1991/08/09  08:18:19  pitz
//      Initiale Version
//
//=============================================================================
*/

#include    <stdio.h>
#include    <string.h>
#include    <sys/param.h>

#ifdef ATARI
    #include    <stdlib.h>
#endif

#include    "defines.h"
#include    "error.h"
#include    "memory.h"
#include    "vartree.h"
#include    "polynom.h"
#include    "term.h"
#include    "termpair.h"
#include    "order.h"
#include    "scanner.h"
#include    "parser.h"
#include    "cpweight.h"
#include    "complet.h"

/****************************************************************************/
/*                                                                          */
/*                          Lokale Variablen                                */
/*                                                                          */
/****************************************************************************/

static short f_descr; /* Interner Filedescriptor des Parsers */

static char     Filename[MAXPATHLEN];

/*
//=============================================================================
//      Listenverwaltung
//=============================================================================
*/

typedef struct lc   { term      *info;
                      struct lc *next; } listcell;

typedef struct      { listcell  *first;
                      listcell  *last;
                      listcell  *now;
                      short     length; } list;


#define EmptyList   { NULL, NULL, NULL, 0 }


static listcell *freelist = NULL;

extern bool     SkolemGoal;             /* Variable aus cpweight */
bool            weight_flag = true;     /* Bewertung der initialen Gleichungen */


#ifdef ANSI

    static listcell *new       ( term     *pointer );
    static void     deletelist ( listcell *cell );

    static void     DeleteList ( list *alist );
    static short    Length     ( list *alist );
    static void     AddList    ( list *alist, term *ptr );
    static term     *First     ( list *alist );
    static term     *Next      ( list *alist );


           void    pcl_fprinttpair ( FILE *stream, 
                                     termpair *pair, char *string );

#endif


/*
//-----------------------------------------------------------------------------
//  Funktion:       new ( term *pointer )
//
//  Parameter:      pointer     Zeiger auf irgendwas
//
//  Rueckgabewert:  Zeiger auf ein neues Listenelement, das die Information
//                  Pointer enthaelt.
//
//  Beschreibung:   Anlegen einer neuen Listenzelle.
//                  Unbelegte Listenelemente werden aus der Freiliste
//                  freelist gewonnen.
//                  Sind keine freien Listenelemente vorhanden, so werden
//                  LISTALLOC neue Elemente allociert und in die Freiliste
//                  eingekettet.
//-----------------------------------------------------------------------------
*/

static listcell     *new ( term *pointer )
{
    register listcell   *ptr, *next;
    register short      i;
             long       size;

    if (!freelist)
    {
        size = LISTALLOC * sizeof (listcell);
        ptr  = freelist = Allocate ( size );
        if (!ptr)
            Error ( __FILE__ ": "  "new", "Zuwenig Speicher fuer Listeneintrag." );

        for ( i = 1; i < LISTALLOC; i++ )
        {
            next = ptr;
            ptr  = ptr->next = ++next;
        }
        ptr->next = NULL;
    }

    ptr = freelist;
    freelist = freelist->next;
    ptr->info = pointer;
    ptr->next = NULL;
    return ptr;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       deletelist ( listcell *cell )
//
//  Parameter:      cell    ein Listenelement
//
//  Beschreibung:   Das Listenelement cell wird in die Freiliste freelist
//                  eingekettet.
//-----------------------------------------------------------------------------
*/

static void     deletelist ( listcell *cell )
{
    cell->next = freelist;
    freelist = cell;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       DeleteList ( list alist )
//
//  Parameter:      alist   Liste, auf der die Operation durchgefuehrt
//                          werden soll.
//
//  Beschreibung:   Die Liste alist wird geloescht,
//                  d.h. sie wird in die Freiliste umgekettet.
//-----------------------------------------------------------------------------
*/

static void    DeleteList ( list *alist )
{
    if (alist->last)
    {
        alist->last->next = freelist;
        freelist = alist->first;

        alist->first  = alist->last = alist->now = NULL;
        alist->length = 0;
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       Length ( list alist )
//
//  Parameter:      alist   Liste, auf der die Operation durchgefuehrt
//                          werden soll.
//
//  Rueckgabewert:  Laenge der Liste alist
//
//  Beschreibung:   Die Laenge der Liste alist wird ermittelt.
//-----------------------------------------------------------------------------
*/

static short   Length ( list *alist )
{
    return  alist->length;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       AddList ( list alist, term *ptr )
//
//  Parameter:      alist   Liste, auf der die Operation durchgefuehrt
//                          werden soll.
//                  ptr     Einzufuegender Pointer
//
//  Beschreibung:   Ein Element an eine Liste anfuegen.
//-----------------------------------------------------------------------------
*/

static void    AddList ( list *alist, term *ptr )
{
    if (alist->last)  alist->last  = alist->last->next = new (ptr);
                else  alist->first = alist->last = new (ptr);
    alist->length++;
}


/*
//-----------------------------------------------------------------------------
//  Funktionen:     First ( list alist )
//                  Next  ( list alist )
//
//  Parameter:      alist   Liste, auf der die Operation durchgefuehrt
//                          werden soll.
//
//  Rueckgabewert:  die entsprechende Information (Zeiger).
//
//  Beschreibung:   Zugriffsfunktionen
//-----------------------------------------------------------------------------
*/

static term    *First ( list *alist )
{
    alist->now = alist->first;
    return (alist->now) ? alist->now->info : NULL;
}

static term    *Next ( list *alist )
{
    alist->now = (alist->now) ? alist->now->next : NULL;
    return (alist->now) ? alist->now->info : NULL;
}



/*
//=============================================================================
//      Unterstuetzung zum Einlesen von Variablen
//=============================================================================
*/

/* Datenstruktur zum Aufnehmen einer Variablen :               */
/* ident : externe Darstellung                                 */
/* code  : interne Kodierung -> negative Zahl                  */
typedef struct varcell { void            *info;
                         struct varcell  *next;
                         char            ident[IDENTLENGTH];
                         variable        code;               } VarCell;

/* Datenstruktur fuer eine Liste von Variablen */
typedef struct  { VarCell    *first;
                  VarCell    *last;  } varlist;


#ifdef  ANSI
    static VarCell      *newvar   ( char *ident, variable code );
    static void         deletevar ( VarCell *cell );
#endif

static VarCell *vfreelist = NULL;
static varlist variables = { NULL, NULL };


/*
//-----------------------------------------------------------------------------
//  Funktion:       newvar ( char *ident, variable code )
//
//  Parameter:      ident       Externe Repraesentation der Variable
//                  code        (interner) Code fuer die Variable
//
//  Rueckgabewert:  Zeiger auf ein neues Listenelement, das die Informationen
//                  ident und code enthaelt.
//
//  Beschreibung:   Anlegen einer neuen Listenelments.
//                  Unbelegte Listenelemente werden aus vfreelist gewonnen.
//                  Sind keine freien Listenelemente vorhanden, so werden
//                  LISTALLOC neue Elemente allociert.
//-----------------------------------------------------------------------------
*/

static VarCell *newvar ( char *ident, variable code )
{
    register VarCell    *ptr, *next;
    register short      i;
             long       size;

    if (!vfreelist)
    {
        size = LISTALLOC * sizeof (VarCell);
        ptr = vfreelist = Allocate ( size );
        if (!ptr)
            Error ( __FILE__ ": "  "newvar", "Zuwenig Speicher." );

        for ( i = 1; i < LISTALLOC; i++ )
        {
            next = ptr;
            ptr = ptr->next = ++next;
        }
        ptr->next = NULL;
    }

    ptr = vfreelist;
    vfreelist = vfreelist->next;
    ptr->code = code;
    strcpy ( ptr->ident, ident );
    ptr->next = NULL;
    return ptr;
}

/*
//-----------------------------------------------------------------------------
//  Funktion:       deletevar ( ListCell *cell )
//
//  Parameter:      cell    ein Listenelement
//
//  Beschreibung:   Das Listenelement cell wird in die Freiliste 
//                  vfreelist umgekettet.
//                  Beachte : diesen Loeschen bedeutet also lediglich,
//                            dass der Speicherplatz der Variablenzelle
//                            wieder vergeben werden kann; es wird also
//                            nur der Status des Speicherplatzes geaen-
//                            dert. (MK)
//-----------------------------------------------------------------------------
*/

static void deletevar ( VarCell *cell )
{
    cell->next = vfreelist;
    vfreelist = cell;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       ClearVariables ( void )
//
//  Parameter:      -keine-
//
//  Beschreibung:   Die Liste variables wird geloescht, d.h. der Spei-
//                  cherplatz bekommt INTERN den Status "frei"(MK)
//
//  Globale Variablen: variables  (MK)
//-----------------------------------------------------------------------------
*/

void     ClearVariables ( void )
{
    VarCell *ptr, *dptr;

    ptr = variables.first;
    while (ptr)
    {
        dptr = ptr;
        ptr = ptr->next;
        deletevar ( dptr );
    }

    variables.first = variables.last = NULL;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       FindVariable   ( char *ident )
//
//  Parameter:      ident   Identifier der gesuchten Variable
//
//  Rueckgabewert:  0       Variable existiert nicht
//                  <> 0    codenummer der Variable
//
//  Beschreibung:   Bestimmung der Codenummer einer Variablen.
//                  Dies geschieht, indem ueberprueft wird, ob in der
//                  globalen Variablen variables ein Eintrag fuer
//                  diesen Identifier vorliegt (MK)
//
//  Globale Variablen: variables
//
//-----------------------------------------------------------------------------
*/

variable     FindVariable   ( char *ident )
{
    VarCell     *ptr;

    ptr = variables.first;
    while (ptr)
    {
        if (!strcmp (ptr->ident, ident))
            return ptr->code;
        ptr = ptr->next;
    }
    return 0;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       CreateVariable ( char *ident )
//
//  Parameter:      ident   Identifier der neuen Variable
//
//  Rueckgabewert:  Codenummer der Variablen.
//
//  Beschreibung:   Testet, ob eine Variable ident bereits existiert.
//                  Falls nicht, wird eine neue Variable angelegt.
//
//  Globale Variable: variables
//
//  Externe Variablen: VarCount ( in term.c )
//
//-----------------------------------------------------------------------------
*/

variable     CreateVariable ( char *ident )
{
    variable code;

    if ((code = FindVariable (ident)) != 0)
        return code;

    code = NewVariable;
    if (variables.last)  variables.last =
                         variables.last->next = newvar (ident, code);
                   else  variables.first =
                         variables.last = newvar (ident, code);
    return code;
}



/*
//=============================================================================
//      Eigentlicher Parser
//=============================================================================
*/

/*
//-----------------------------------------------------------------------------
//      Lokale Funktionsdefinitionen
//-----------------------------------------------------------------------------
*/

#ifdef  ANSI
    static void  SyntaxError     ( char *error );
    static void  CheckError      ( bool cond, char *msg );
    static short readordering    ( short number );
    static short readaltordering ( short number );
    static term  *readterm       ( char *ident, Symbol *sym );
#ifdef BERLIN
    static bool  prologVar       ( char *ident );
#endif
#endif


/*
//-----------------------------------------------------------------------------
//  Funktion:       SyntaxError
//
//  Parameter:      error       Fehlermeldung
//
//  Beschreibung:   Ausgabe einer Fehlermeldung
//-----------------------------------------------------------------------------
*/

static void    SyntaxError ( char *error )
{
    PrintScanText ( f_descr );
    printf ( "****  Fehler in der eingelesenen Datei : %s.\n", Filename );
    printf ( "****  %s\n", error );
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       CheckError
//
//  Parameter:      cond    Bedingung fuer Fehlerfall
//                  msg     Fehlermeldung
//
//  Beschreibung:   Ausgabe einer Fehlermeldung, falls cond erfuellt.
//-----------------------------------------------------------------------------
*/

static void CheckError ( bool cond, char *msg )
{
    if (cond)
    {
        SyntaxError ( msg );
        CloseInput ( f_descr );
	getchar ();
        exit ( 1 );
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       readordering ( short number )
//
//  Parameter:      number   Nummer der Ordnung
//
//  Beschreibung:   Einlesen einer Ordnung
//-----------------------------------------------------------------------------
*/

static short readordering ( short number )
{
    char        ident[IDENTLENGTH];
    char        ident2[IDENTLENGTH];
    Symbol      sym, order;
    short       counter = 1;

    order = GetSymbol ( f_descr, ident);

    CheckError ( ((order != SKBO) && (order != SXKBO) && (order != SWKBO) &&
                  (order != SRPO) && (order != SLPO)),
                 "KBO, LPO, RPO, XKBO oder WKBO erwartet." );

    SetOrdertype ( number, order );

    CheckError ( (GetSymbol ( f_descr, ident) != SCRSYM), "Neue Zeile erwartet." );
    while ((sym = GetSymbol ( f_descr, ident)) == SCRSYM)
        CheckError ( EOI( f_descr ), "Unerwartetes Dateiende." );

    if (order == SKBO)
    {
        while (sym == SIDENT)
        {
            CheckError ( (GetSymbol ( f_descr, ident2) != SCOLLON),
                         "':' erwartet." );

            CheckError ( (GetSymbol ( f_descr, ident2) != SIDENT)
                         && !IsNumber (ident2),
                         "Ganzzahl erwartet." );

            SetWeight ( NewFunction ( ident ), number, 0, atoi ( ident2 ) );

            CheckError ( (GetSymbol ( f_descr, ident) != SCRSYM),
                         "Neue Zeile erwartet." );

            while ((sym = GetSymbol ( f_descr, ident)) == SCRSYM)
                CheckError ( EOI( f_descr ), "Unerwartetes Dateiende." );
        }
    }
    else if ((order == SLPO) || (order == SRPO))
    {
        while (sym == SIDENT)
        {
            SetWeight ( NewFunction ( ident ), number, counter++, 0 );
            if ((sym = GetSymbol ( f_descr, ident)) == SCRSYM)
                break;
            CheckError ( (sym != SGREATER), "Neue Zeile oder > erwartet." );
            while ((sym = GetSymbol ( f_descr, ident)) == SCRSYM)
                CheckError ( EOI( f_descr ), "Unerwartetes Dateiende." );
            CheckError ( (sym != SIDENT), "Identifier erwartet." );
        }
        while ((sym = GetSymbol ( f_descr, ident)) == SCRSYM)
            CheckError ( EOI( f_descr ), "Unerwartetes Dateiende." );
    }
    else if ((order == SXKBO) || (order == SWKBO))
    {
        while (sym == SIDENT)
        {
            CheckError ( (GetSymbol ( f_descr, ident2) != SCOLLON),
                         "':' erwartet." );
            CheckError ( (GetSymbol ( f_descr, ident2) !=  SIDENT)
                         && !IsNumber (ident2),
                         "Ganzzahl erwartet." );
            SetWeight ( NewFunction (ident), number, counter++, atoi (ident2) );

            if ((sym = GetSymbol ( f_descr, ident)) == SCRSYM)
                break;
            CheckError ( (sym != SGREATER), "Neue Zeile oder > erwartet." );
            while ((sym = GetSymbol ( f_descr, ident)) == SCRSYM)
                CheckError ( EOI( f_descr ), "Unerwartetes Dateiende." );
            CheckError ( (sym != SIDENT), "Identifier erwartet." );
        }
        while ((sym = GetSymbol ( f_descr, ident)) == SCRSYM)
            CheckError ( EOI( f_descr ), "Unerwartetes Dateiende." );
    }
    else
    {
        CheckError ( true, "Ordnung noch nicht implementiert." );
    }

    return sym;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       readaltordering ( short number )
//
//  Parameter:      number   Nummer der Ordnung
//
//  Beschreibung:   Einlesen einer alternativen Ordnung
//-----------------------------------------------------------------------------
*/

static short readaltordering ( short number )
{
    char        ident[IDENTLENGTH];
    char        ident2[IDENTLENGTH];
    Symbol      sym, order;
    short       fn, i;
    short       counter = 1;

    order = GetSymbol ( f_descr, ident);

    CheckError ( ((order != SKBO) && (order != SXKBO) && (order != SWKBO) &&
                  (order != SRPO) && (order != SLPO)),
                 "KBO, LPO, RPO, XKBO oder WKBO erwartet." );

    SetOrdertype ( number, order );

    CheckError ( (GetSymbol ( f_descr, ident) != SCRSYM), "Neue Zeile erwartet." );
    while ((sym = GetSymbol ( f_descr, ident)) == SCRSYM)
        CheckError ( EOI( f_descr ), "Unerwartetes Dateiende." );

    if (order == SKBO)
    {
        while (sym == SIDENT)
        {
            CheckError ( (GetSymbol ( f_descr, ident2) != SCOLLON),
                         "':' erwartet." );

            CheckError ( (GetSymbol ( f_descr, ident2) != SIDENT)
                         && !IsNumber (ident2),
                         "Ganzzahl erwartet." );

            CheckError (((fn = FindFunction (ident)) == 0),
                        "Neues Funktionssymbol in alternativer Ordnung." );
            SetWeight ( fn, number, 0, atoi ( ident2 ) );

            CheckError ( (GetSymbol ( f_descr, ident) != SCRSYM),
                         "Neue Zeile erwartet." );

            while ((sym = GetSymbol ( f_descr, ident)) == SCRSYM)
                CheckError ( EOI( f_descr ), "Unerwartetes Dateiende." );
        }
    }
    else if ((order == SLPO) || (order == SRPO))
    {
        while (sym == SIDENT)
        {
            CheckError (((fn = FindFunction (ident)) == 0),
                        "Neues Funktionssymbol in alternativer Ordnung." );
            SetWeight ( fn, number, counter++, 0 );
            if ((sym = GetSymbol ( f_descr, ident)) == SCRSYM)
                break;
            CheckError ( (sym != SGREATER), "Neue Zeile oder > erwartet." );
            while ((sym = GetSymbol ( f_descr, ident)) == SCRSYM)
                CheckError ( EOI( f_descr ), "Unerwartetes Dateiende." );
            CheckError ( (sym != SIDENT), "Identifier erwartet." );
        }
        while ((sym = GetSymbol ( f_descr, ident)) == SCRSYM)
            CheckError ( EOI( f_descr ), "Unerwartetes Dateiende." );
    }
    else if ((order == SXKBO) || (order == SWKBO))
    {
        while (sym == SIDENT)
        {
            CheckError ( (GetSymbol ( f_descr, ident2) != SCOLLON),
                         "':' erwartet." );
            CheckError ( (GetSymbol ( f_descr, ident2) !=  SIDENT)
                         && !IsNumber (ident2),
                         "Ganzzahl erwartet." );
            CheckError (((fn = FindFunction (ident)) == 0),
                        "Neues Funktionssymbol in alternativer Ordnung." );
            SetWeight ( fn, number, counter++, atoi (ident2) );

            if ((sym = GetSymbol ( f_descr, ident)) == SCRSYM)
                break;
            CheckError ( (sym != SGREATER), "Neue Zeile oder > erwartet." );
            while ((sym = GetSymbol ( f_descr, ident)) == SCRSYM)
                CheckError ( EOI( f_descr ), "Unerwartetes Dateiende." );
            CheckError ( (sym != SIDENT), "Identifier erwartet." );
        }
        while ((sym = GetSymbol ( f_descr, ident)) == SCRSYM)
            CheckError ( EOI( f_descr ), "Unerwartetes Dateiende." );
    }
    else
    {
        CheckError ( true, "Ordnung noch nicht implementiert." );
    }

    for (i = 1; i <= FuncCount; i++ )
        if (Order[number].weight[i] < 0)
        {
            printf ( "******  %s wird in %ld. Ordnung nicht beruecksichtigt.\n",
                     Function[i].ident, (long)(number+1) );
            Error ( __FILE__ ": "  "readaltordering", "Unvollstaendige Ordnung." );
        }

    return sym;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       readterm ( char *first, Symbol *sym )
//
//  Parameter:      first    Startidentifier eines Terms
//                  sym      Endsymbol
//
//  Rueckgabewert:  der eingelesene Term
//
//  Beschreibung:   Einlesen eines Terms
//-----------------------------------------------------------------------------
*/

static term    *readterm ( char *ident, Symbol *sym )
{
    function    func;
    list        arglist = EmptyList;
    term        *t;
    short       i = 1;

    if ((func = FindFunction (ident)) == 0)
    {
#ifdef BERLIN
        if (!prologVar(ident))
        {
            func=NewFunction(ident);
            for (i=0; i < OrderCount; SetWeight(func,i++,FuncCount,2));
            SetArguments(func,0);
            *sym = GetSymbol ( f_descr, ident);
            return newterm(func);
        }
#endif
        *sym = GetSymbol ( f_descr, ident);
        return newterm ( CreateVariable (ident) );
    }

    if ((*sym = GetSymbol ( f_descr, ident)) != SBRACKET_L)
    {
        SetArguments ( func, 0 );
        return newterm ( func );
    }

    while ((*sym = GetSymbol ( f_descr, ident)) == SIDENT)
    {
        AddList ( &arglist, readterm ( ident, sym ));
        if (*sym != SCOMMA)
            break;
    }
    CheckError ( (*sym != SBRACKET_R), ", oder ) erwartet." );

    SetArguments ( func, Length (&arglist) );
    t = newterm ( func );

    if (Arity(func) > 0)
        t->argument[0] = First (&arglist);
    while (i < Arity(func))
        t->argument[i++] = Next (&arglist);
    DeleteList ( &arglist );

    *sym = GetSymbol ( f_descr, ident);
    return t;
}


#ifdef BERLIN
static bool prologVar ( char *ident )
{
    if (*ident=='_' || (*ident>='A' && *ident<='Z'))
        return true;
    return false;
}
#endif

/*
//-----------------------------------------------------------------------------
//  Funktion:       Parse ( char *filename, char *example )
//
//  Parameter:      filename    Dateiname
//                  example     Name des Beispiels
//
//  Beschreibung:   Einlesen einer Aufgabenstellung
//-----------------------------------------------------------------------------
*/

void    Parse ( char *filename, char *example )
{
    char        ident[IDENTLENGTH];
    short       mode;
    Symbol      sym;
    function    func;
    term        *left, *right;
    termpair    *pair;
    pairset     set = EmptySet;

    strcpy( Filename, filename );
    freelist = NULL;
    f_descr = OpenInput ( filename );
    SetScanMode ( f_descr, FILE_MODE );

    while ((sym = GetSymbol ( f_descr, ident)) == SCRSYM)
        CheckError ( EOI( f_descr ), "Unerwartetes Dateiende." );

    CheckError ( (sym != SMODE), "MODE erwartet." );
    mode = GetSymbol ( f_descr, ident);
    CheckError ( ((mode != SPROOF) && (mode != SCOMPLETION)),
                 "PROOF oder COMPLETION erwartet." );
    sym = GetSymbol ( f_descr, ident);

    Paramodulation = false;
    ParaCount = 0;

    CheckError ((mode == SCOMPLETION) && (sym != SCRSYM),
                "Neue Zeile erwartet." );

    if (sym != SCRSYM)
    {
        CheckError ((sym != SCOMMA), ", erwartet." );
        CheckError ((GetSymbol ( f_descr, ident) != SPARAMOD),
                    "PARAMOD erwartet" );
        Paramodulation = true;
        sym = GetSymbol ( f_descr, ident);
	CheckError ((sym != SBRACKET_L), " ( erwartet.");

	CheckError ( (GetSymbol ( f_descr, ident) != SIDENT) && !IsNumber (ident),
		     "Ganzzahl erwartet." );

	ParaCount = atoi (ident);
	CheckError ( ParaCount == 0,
		     "0 darf nicht als Parameter von PARAMOD angegeben werden.");

	CheckError ( (GetSymbol ( f_descr, ident) != SBRACKET_R),
		     ") erwartet." );
	CheckError ( (GetSymbol ( f_descr, ident) != SCRSYM),
		     "Neue Zeile erwartet." );
    }

    while ((sym = GetSymbol ( f_descr, ident)) == SCRSYM)
        CheckError ( EOI( f_descr ), "Unerwartetes Dateiende." );

    CheckError ( (sym != SNAME), "NAME erwartet." );
    CheckError ( (GetSymbol ( f_descr, example) != SIDENT), "Identifier erwartet." );
    CheckError ( (GetSymbol ( f_descr, ident) != SCRSYM), "Neue Zeile erwartet." );

    while ((sym = GetSymbol ( f_descr, ident)) == SCRSYM)
        CheckError ( EOI( f_descr ), "Unerwartetes Dateiende." );

    CheckError ( (sym != SORDERING), "ORDERING erwartet." );

    sym = readordering ( OrderCount++ );
    while (sym == SORDERING)
       sym = readaltordering ( OrderCount++ );

    SetOrdering ( 0 );

    CheckError ( (sym != SEQUATIONS) && (sym != SCANCELLATION),
                 "CANCELLATION oder EQUATIONS erwartet." );

    if (sym == SCANCELLATION)
    {
        CancelActive = true;
        while ((sym = GetSymbol ( f_descr, ident)) == SCRSYM)
            CheckError ( EOI( f_descr ), "Unerwartetes Dateiende." );

        while (sym == SIDENT)
        {
            func = FindFunction (ident);
            CheckError ( (!func), "Funktionssymbol erwartet." );

            CheckError ( (GetSymbol ( f_descr, ident) != SCOLLON),
                         "':' erwartet." );

            sym = GetSymbol ( f_descr, ident);
            CheckError ( (sym != SIDENT) && !IsNumber (ident),
                         "Ganzzahl erwartet." );

            while (sym == SIDENT)
            {
                SetCancellation ( func, atoi (ident) );

                sym = GetSymbol ( f_descr, ident);
                CheckError ( ((sym != SCOMMA) && (sym != SCRSYM)),
                             "Komma oder neue Zeile erwartet." );

                if (sym == SCOMMA)
                {
                    sym = GetSymbol ( f_descr, ident);
                    CheckError ( (sym != SIDENT) && !IsNumber (ident),
                                 "Ganzzahl erwartet." );
                }
            }

            while ((sym = GetSymbol ( f_descr, ident)) == SCRSYM)
                CheckError ( EOI( f_descr ), "Unerwartetes Dateiende." );
        }
    }

    CheckError ( (sym != SEQUATIONS), "EQUATIONS erwartet." );

    while ((sym = GetSymbol ( f_descr, ident)) == SCRSYM)
        CheckError ( EOI( f_descr ), "Unerwartetes Dateiende." );

    while (sym == SIDENT)
    {
        ClearVariables ();
        getweight (left = readterm (ident, &sym) );

        CheckError ( sym != SEQUAL, "= erwartet." );

        while ((sym = GetSymbol ( f_descr, ident)) == SCRSYM)
            CheckError ( EOI( f_descr ), "Unerwartetes Dateiende." );
        CheckError ( (sym != SIDENT), "Term erwartet." );

        getweight (right = readterm (ident, &sym) );

        pair = newpair (left, right, NULL, NULL);
        Add ( &set, pair );
        Add ( &SetOfAxioms, pair); /* Axioms for Example Selection */

        CheckError ( sym != SCRSYM, "Neue Zeile erwartet." );
        while ((sym = GetSymbol ( f_descr, ident)) == SCRSYM)
            if (EOI( f_descr ))
                break;
    }

    if (sym == SSPECIAL)
    {
        while ((sym = GetSymbol ( f_descr, ident)) == SCRSYM)
            CheckError ( EOI( f_descr ), "Unerwartetes Dateiende." );

        while (sym == SIDENT)
        {
            ClearVariables ();
            getweight (left = readterm (ident, &sym) );

            CheckError ( sym != SEQUAL, "= erwartet." );

            while ((sym = GetSymbol ( f_descr, ident)) == SCRSYM)
                CheckError ( EOI( f_descr ), "Unerwartetes Dateiende." );
            CheckError ( (sym != SIDENT), "Term erwartet." );

            getweight (right = readterm (ident, &sym) );

            pair = newpair (left, right, NULL, NULL);
            pair->special_flag = true;
            Add ( &set, pair );

            CheckError ( sym != SCRSYM, "Neue Zeile erwartet." );
            while ((sym = GetSymbol ( f_descr, ident)) == SCRSYM)
                if (EOI( f_descr ))
                    break;
        }
    }

    if (mode == SPROOF)
    {
        CheckError ( (sym != SCONCLUSION), "SPECIAL oder CONCLUSION erwartet." );

        while ((sym = GetSymbol ( f_descr, ident)) == SCRSYM)
            CheckError ( EOI( f_descr ), "Unerwartetes Dateiende." );

        CheckError ( (sym != SIDENT), "Term erwartet." );

        ClearVariables ();
        getweight (left = readterm (ident, &sym) );

        CheckError ( sym != SEQUAL, "= erwartet." );

        while ((sym = GetSymbol ( f_descr, ident)) == SCRSYM)
            CheckError ( EOI( f_descr ), "Unerwartetes Dateiende." );
        CheckError ( (sym != SIDENT), "Term erwartet." );

        getweight (right = readterm (ident, &sym) );

        pair = newpair (left, right, NULL, NULL);
        AddGoal ( pair );
        SkolemGoal = !varoccur (left) && !varoccur (right);

        Proofmode = true;
    }
    else
    {
        Proofmode = false;
    }

    while (!EOI( f_descr ))
        CheckError ((GetSymbol ( f_descr, ident) != SCRSYM),
                    "Dateiende erwartet." );
    CloseInput ( f_descr );

    CheckFunctions ();

    if (CPWeight==occnest)
        init_goal_fn_m();

    if (weight_flag)
    {
        while ((pair = DelFirst ( &set )))
        {
            pair->weight = CPWeight ( pair );
            if (special_factor && pair->special_flag)
                SpecialWeight ( pair );
            InsertCP ( pair );
        }
    }
    else
    {
        while ((pair = DelFirst ( &set )))
            AddCP ( pair );
    }
}



static void ptp ( termpair *tp )
{
    pcl_fprinttpair ( stdout, tp, "=" );
}


void PrintFile ( char *name )
{
    short   i;

    printf ( "mode %s\n", (Proofmode) ? "proof"
                                      : "completion" );
    printf ( "name %s\n", name );
    if ((Ordering == SXKBO) || (Ordering == SLPO))
    {
        printf ( "ordering lpo\n" );
        for (i = 1; i < FuncCount; i++)
            printf ( "%s>", Function[i].ident );
        printf ( "%s\n", Function[i].ident );
    }
    else 
    {
        printf ( "---------------\n\n" );
        exit ( 0 );
    }

    printf ( "equations\n" );
    ForAllCPsDo ( ptp );
    if (!Proofmode)
        return;
    ForAllGoalsDo ( ptp );
}
