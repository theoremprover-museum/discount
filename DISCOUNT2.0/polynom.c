/*
//=============================================================================
//      Projekt:        TEAM-COMPLETION
//
//      Modul:          polynom
//
//      Author:         Werner Pitz, 1991
//
//      Beschreibung:   Verwaltung von Polynomen
//-----------------------------------------------------------------------------
//      $Log: polynom.c,v $
//      Revision 0.1  1991/08/19  09:50:10  pitz
//      Fehlermeldungen mit Dateiangabe.
//
//      Revision 0.0  1991/08/09  08:18:19  pitz
//      Initiale Version
//
//=============================================================================
*/

#include    <stdio.h>
#include    <string.h>
#include    <ctype.h>

#ifdef    ATARI
    #include    <stdlib.h>
    #include    <ext.h>
#endif

#include    "defines.h"
#include    "error.h"
#include    "memory.h"
#include    "vartree.h"
#include    "polynom.h"
#include    "term.h"
#include    "scanner.h"



/*
//-----------------------------------------------------------------------------
//      Datendeklarationen
//-----------------------------------------------------------------------------
*/

#define MAXVAR      20

static  polynom     *freelist   = NULL;

static  char        varname[MAXVAR][IDENTLENGTH];
static  short       varcount;


#define ClearVars()      varcount = 0
#define AddVar(ident)    strcpy (varname[varcount++],ident)


/*
//-----------------------------------------------------------------------------
//      Modulinterne Definitionen
//-----------------------------------------------------------------------------
*/

#ifdef  ANSI

    static void     delete       ( polynom *poly );

    static char     *skip        ( char *ptr );
    static char     *scanident   ( char *ptr, char *ident );
    static char     *getbracket  ( char *ptr );

    static function readfunction ( bool newfunc, char *fptr );

    static long     readexp      ( char **ptr );
    static polynom  *readfactor  ( char **ptr );
    static polynom  *readpolynom ( char **ptr );

#endif


/*
//-----------------------------------------------------------------------------
//  Funktion:       newpoly ( opcode opc, long value )
//
//  Parameter:      opc     Opcode
//                  value   Ganzzahliger Wert
//
//  Rueckgabewert:  Zeiger auf eine Polynomzelle.
//
//  Beschreibung:   Anlegen einer neuen Polynomzelle.
//                  Unbelegte Polynomzellen werden aus der Freiliste
//                  freelist gewonnen.
//                  Sind keine freien Listenelemente vorhanden, so werden
//                  LISTALLOC neue Elemente allociert und in die Freiliste
//                  eingekettet.
//-----------------------------------------------------------------------------
*/

polynom  *newpoly ( opcode opc, long value )
{
    register polynom    *ptr, *next;
    register short      i;
             long       size;

    if (!freelist)
    {
        size = POLYALLOC * sizeof (polynom);
        ptr  = freelist = Malloc ( size );
        if (!ptr)
            Error ( __FILE__ ": "  "polynom:new", "Zuwenig Speicher fuer Listeneintrag." );

        for ( i = 1; i < POLYALLOC; i++ )
        {
            next = ptr;
            ptr  = ptr->left = ++next;
        }
        ptr->left = NULL;
    }

    ptr = freelist;
    freelist = freelist->left;

    ptr->opc   = opc;
    ptr->value = value;
    ptr->left  = NULL;
    ptr->right = NULL;
    return ptr;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       delete ( polynom *poly )
//
//  Parameter:      poly    eine Polynomzelle
//
//  Beschreibung:   Eine Polynomzelle wird in die Freiliste eingefgt.
//-----------------------------------------------------------------------------
*/

static void delete ( polynom *poly )
{
    poly->left = freelist;
    freelist   = poly;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       deletepoly ( polynom *poly )
//
//  Parameter:      poly    Zeiger auf ein Polynom
//
//  Beschreibung:   Ein Polynom wird in die Freiliste umgekettet
//-----------------------------------------------------------------------------
*/

void deletepoly ( polynom *poly )
{
    if (poly)
    {
        deletepoly (poly->left);
        deletepoly (poly->right);
        poly->left = freelist;
        freelist   = poly;
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       skip ( char *ptr )
//
//  Parameter:      ptr     Zeiger auf String
//
//  Rueckgabewert:  Neue Position des Zeigers
//
//  Beschreibung:   Ueberspringe Leerzeichen
//-----------------------------------------------------------------------------
*/

static char     *skip ( char *ptr )
{
    while ((*ptr == ' ') || (*ptr =='\t'))
        ptr++;
    return ptr;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       scanident ( char *ptr, char *ident )
//
//  Parameter:      ptr     Zeiger auf Pufferbereich
//                  ident   Puffer Fuer Identifierer
//
//  Rueckgabewert:  Neue Position des Zeigers
//
//  Beschreibung:   Einlesen eines Identifiers aus String
//-----------------------------------------------------------------------------
*/

static char     *scanident ( char *ptr, char *ident )
{
    ptr = skip (ptr);

    while (isalnum (*ptr) || (*ptr == '_'))
        *ident++ = *ptr++;
    *ident = '\0';

    return skip (ptr);
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       getbracket ( char *ptr )
//
//  Parameter:      ptr     Zeiger auf Pufferbereich
//
//  Rueckgabewert:  Neue Position des Zeigers
//
//  Beschreibung:   Ende eines Klammerblocks finden und mit \0 markieren
//-----------------------------------------------------------------------------
*/

static char  *getbracket ( char *ptr )
{
    short    count = 0;

    do
    {
        if (!*ptr)
            Error ( __FILE__ ": "  "getbracket", "Fehler in Klammerstruktur." );

        if (*ptr == '(')
            count++;
        else if (*ptr == ')')
            count--;

        ptr++;
    }
    while (count);
    ptr--;
    *ptr++ = '\0';

    return ptr;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       readfunction ( bool newfunc, char *ptr )
//
//  Parameter:      newfunc     true:  Funktionen sollen angelegt werden
//                              false: Funktionen existieren bereits
//                  ptr         Vereinbarung des Funktionssymbols
//                              z.B.: 'f (x,y)'
//
//  Rueckgabewert:  Code dess Funktionsymbols
//
//  Beschreibung:   Einlesen einer Funktionsvereinbarung
//-----------------------------------------------------------------------------
*/

static function readfunction ( bool newfunc, char *ptr )
{
    char        func[IDENTLENGTH];
    char        ident[IDENTLENGTH];
    short       i;
    function    fcode;

    ClearVars();
    ptr = scanident ( ptr, func );
    if (!*func)
        Error ( __FILE__ ": "  "readfunction", "Funktionbezeichner erwartet." );

    if (*ptr == '(')
    {
        do
        {
            ptr++;
            ptr = scanident ( ptr, ident );
            if (!*ident)
                Error ( __FILE__ ": "  "readfunction", "Variable erwartet." );

            for (i = 0; i < varcount; i++)
                if (!strcmp (varname[i],ident))
                    Error ( __FILE__ ": "  "readfunction",
                            "Variable mehrfach vereinbart." );
            AddVar (ident);

            if ((*ptr != ',') && (*ptr != ')'))
                Error ( __FILE__ ": "  "readfunction",  ") oder , erwartet." );
        }
        while ((*ptr != ')') && (*ptr));

        if (*ptr != ')')
            Error ( __FILE__ ": "  "readfunction",
                    "Unerwartes Ende eines Funktionsvereinbarung." );
    }
    else
    {
        if (*ptr)
           Error ( __FILE__ ": "  "readfunction", "( erwartet." );
    }

    if (newfunc)  SetArguments ( fcode = NewFunction (func),  varcount );
            else  SetArguments ( fcode = FindFunction (func), varcount );

    return fcode;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       readexp ( char **ptr )
//
//  Parameter:      ptr     Zeiger auf String
//
//  Rueckgabewert:  Wert der eingelesenen Zahl
//
//  Beschreibung:   Einlesen eines Exponenten (positive Ganzzahl !)
//-----------------------------------------------------------------------------
*/

static long readexp ( char **ptr )
{
    char    ident[IDENTLENGTH];
    long    value;

    if (!**ptr)
        Error ( __FILE__ ": "  "readexp", "Unerwartes Ende der Eingabe." );

    *ptr = scanident ( *ptr, ident );

    if (IsNumber (ident))
    {
        value = atoi (ident);
        if (value < 0)
            Error ( __FILE__ ": "  "readexp", "Als Exponenten nur positive Zahlen." );
        return value;
    }
    
    Error ( __FILE__ ": "  "readexp", "Als Exponenten nur ganze Zahlen." );
    return 0;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       readfactor ( char **ptr )
//
//  Parameter:      ptr     Zeiger auf String
//
//  Rueckgabewert:  eingelesener Factor
//
//  Beschreibung:   Einlesen eines Factors.
//-----------------------------------------------------------------------------
*/

static polynom *readfactor ( char **ptr )
{
    polynom *left, *right, *cell;
    char    ident[IDENTLENGTH];
    char    *xptr;
    short   i, v;

    if (!**ptr)
        Error ( __FILE__ ": "  "readfactor", "Unerwartes Ende der Eingabe." );

    if (**ptr == '(')
    {
        xptr = getbracket ( *ptr );
        (*ptr)++;
        left = readpolynom ( ptr );
        *ptr = xptr;
        if (!*xptr)
            return left;
    }
    else
    {
        *ptr = scanident ( *ptr, ident );
        if (IsNumber (ident))
            left = newpoly ( op_const, atoi (ident) );
        else
        {
            v = -1;
            for (i = 0; (i < varcount) && (v < 0); i++)
              if (!strcmp (varname[i],ident))
                  v = i;

            if (v < 0)
                Error ( __FILE__ ": "  "readfactor", "Unbekannte Variable." );

            left = newpoly ( op_var, v );
        }
    }

    if (**ptr == '^')
    {
       (*ptr)++;
       cell = newpoly (op_pot, readexp (ptr));
       cell->left = left;
       left = cell;
    }

    if (**ptr == '^')
        Error ( __FILE__ ": "  "readfactor", "Nur einstufige Exponenten." );

    if (**ptr != '*')
       return left;

    (*ptr)++;
    right = readfactor ( ptr );
    cell  = newpoly ( op_mul, 0 );
    cell->left  = left;
    cell->right = right;

    return cell;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       readpolynom ( char **ptr )
//
//  Parameter:      ptr     Zeiger auf String
//
//  Rueckgabewert:  eingelesenes Polynom
//
//  Beschreibung:   Einlesen eines Polynoms.
//-----------------------------------------------------------------------------
*/

static polynom *readpolynom ( char **ptr )
{
    polynom *left, *right, *cell;
    char    ident[IDENTLENGTH];
    char    *xptr;
    short   i, v;

    if (!**ptr)
        Error ( __FILE__ ": "  "readpolynom", "Unerwartes Ende der Eingabe." );

    if (**ptr == '(')
    {
        xptr = getbracket ( *ptr );
        (*ptr)++;
        left = readpolynom ( ptr );
        *ptr = xptr;
        if (!*xptr)
            return left;
    }
    else
    {
        *ptr = scanident ( *ptr, ident );
        if (IsNumber (ident))
            left = newpoly ( op_const, atoi (ident) );
        else
        {
            v = -1;
            for (i = 0; (i < varcount) && (v < 0); i++)
              if (!strcmp (varname[i],ident))
                  v = i;

            if (v < 0)
                Error ( __FILE__ ": "  "readpolynom", "Unbekannte Variable." );

            left = newpoly ( op_var, v );
        }
    }

next:
    *ptr = skip ( *ptr );

    if (!**ptr)
       return left;

    switch (**ptr)
    {
        case '+': (*ptr)++;
                  right = readpolynom ( ptr );
                  cell  = newpoly ( op_add, 0 );
                  cell->left  = left;
                  cell->right = right;
                  return cell;

        case '-': (*ptr)++;
                  right = readfactor ( ptr );
                  cell  = newpoly ( op_sub, 0 );
                  cell->left  = left;
                  cell->right = right;
                  left = cell;
                  goto next;

        case '*': (*ptr)++;
                  right = readfactor ( ptr );
                  cell  = newpoly ( op_mul, 0 );
                  cell->left  = left;
                  cell->right = right;
                  left = cell;
                  goto next;

        case '^': (*ptr)++;
                  cell  = newpoly ( op_pot, readexp (ptr) );
                  cell->left  = left;
                  left = cell;
                  goto next;

        default:  Error ( __FILE__ ": "  "readpolynom", "Unbekannter Operator." );
    }
    return NULL;
}


/*
//=============================================================================
//      Externe Funktionen
//=============================================================================
*/

/*
//-----------------------------------------------------------------------------
//  Funktion:       polyval ( polynom *poly, long *var )
//
//  Parameter:      poly    Zeiger auf ein Polynom
//                  var     Werte der Variablen
//
//  Rueckgabewert:  Wert des Polynoms
//
//  Beschreibung:   Auswertung eines Polynoms
//-----------------------------------------------------------------------------
*/

long    polyval ( polynom *poly, long *var )
{
    long     v, x, i;
    switch (poly->opc)
    {
       case op_const:   return poly->value;
       case op_var:     return var[poly->value];
       case op_add:     return   polyval (poly->left, var)
                               + polyval (poly->right, var);
       case op_sub:     return   polyval (poly->left, var)
                               - polyval (poly->right, var);
       case op_mul:     return   polyval (poly->left, var)
                               * polyval (poly->right, var);
       case op_pot:     v = 1;
                        x = polyval (poly->left, var);
                        i = poly->value;
                        while (i)
                        {
                            if (i & 1)
                               v *= x;

                            x *= x;
                            i = i >> 1;
                        }
                        return v;
       default:         Error ( __FILE__ ": "  "polyval", "Unbekannter Operator." );
                        break;
    }
    return 0;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       getpolynom ( char *fptr, char *pptr )
//
//  Parameter:      fcode       Puffer fuer Funktionscode
//                  newfunc     true:  Funktionen sollen angelegt werden
//                              false: Funktionen existieren bereits
//                  fptr        Vereinbarung des Funktionssymbols
//                              z.B.: 'f (x,y)'
//                  pptr        Zugehoeriges Polynom
//                              z.B.: 'x^2 + 2*x*y + y^2'
//
//  Beschreibung:   Erzeugen eines Polynoms
//-----------------------------------------------------------------------------
*/

polynom    *getpolynom ( function *fcode, bool newfunc, char *fptr, char *pptr )
{
    *fcode = readfunction ( newfunc, fptr );
    return readpolynom ( &pptr );
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       printpoly ( polynom *poly )
//
//  Parameter:      poly    Zeiger auf Polynom
//
//  Beschreibung:   Ausgabe eines Polynoms.
//                  Variablen werden dabei in der Form <n> ausgegeben.
//-----------------------------------------------------------------------------
*/

void printpoly ( polynom *poly )
{
    switch (poly->opc)
    {
       case op_const:   printf ( "%ld", poly->value );
                        break;
       case op_var:     printf ( "<%ld>", 1+poly->value );
                        break;
       case op_add:     printf ( "(" );
                        printpoly (poly->left);
                        printf ( "+" );
                        printpoly (poly->right);
                        printf ( ")" );
                        break;
       case op_sub:     printf ( "(" );
                        printpoly (poly->left);
                        printf ( "-" );
                        printpoly (poly->right);
                        printf ( ")" );
                        break;
       case op_mul:     printf ( "(" );
                        printpoly (poly->left);
                        printf ( "*" );
                        printpoly (poly->right);
                        printf ( ")" );
                        break;
       case op_pot:     printf ( "(" );
                        printpoly (poly->left);
                        printf ( "^%ld", poly->value );
                        printf ( ")" );
                        break;
    }
}

