/*
//=============================================================================
//      Projekt:        TEAM-COMPLETION
//
//      Header:         term
//
//      Author:         Werner Pitz, 1991
//
//      Beschreibung:   Alle Funktionen zur Verwaltung von Termen.
//
//      Datentypen:     term            Termstruktur
//                      FunctionInfo    Informationen ueber Funktionen
//
//      Funktionen:     FindFunction    Suche eine Funktion anhand des
//                                      Identifiers
//                      NewFunction     Lege neue Funktion an
//                      SetOrdertype    Festlegen einer Ordnung
//                      SetWeight       Setze ein Gewicht fuer eine Funktion
//                      CheckFunctions  Ueberpruefe die Vollstaendigkeit der
//                                      Definition
//                      PrintFunctions  Ausgabe der Funktionen (Test)
//
//                      newterm         Anlegen einer Termzelle
//                      deletet         Loeschen einer Ternzelle
//                      deleteterm      Loeschen eines Terms
//
//                      copyterm        Termkopie erzeugen
//                      occur           Occurence-Check
//                      foccur          Occurence-Check fuer Funktionssymbol
//                      equal           sytaktische Termgleichheit
//                      member          Teiltermeigenschaft
//
//                      printterm       Ausgabe der internen Termstruktur
//
//                      prettyprint     Hilfsfunktion fuer verbesserte Ausgabe
//                                      (Variablenbaum erforderlich)
//                      fprettyprint    Hilfsfunktion fuer verbesserte Ausgabe
//                                      Ausgabe auf Stream
//                                      (Variablenbaum erforderlich)
//                      prettyterm      Verbesserte Ausgabe
//                      fprettyterm     Verbesserte Ausgabe
//                                      Ausgabe auf Stream
//
//                      varoccur        Ueberprueft, ob in einem Term Variablen
//                                      vorkommen.
//                      newvars         Hilfsfunktion fuer Variablenumbennenung
//                                      (Variablenbaum erforderlich)
//                      tnewvars        Variablenumbenennung
//
//                      getvars         Hilfsfunktion fuer Variablenbestimmung
//                                      (Variablenbaum erforderlich)
//                      tgetvars        Variablenbestimmung
//
//                      copynew         Hilfsfunktion fuer Termkopie mit neuen
//                                      Variablen
//                                      (Variablenbaum erforderlich)
//                      tcopynew        Termkopie mit neuen Variablen
//                      tweight         einfache Gewichtsfunktion
//-----------------------------------------------------------------------------
//      $Log: term.h,v $
//      Revision 0.1  1991/08/09  10:00:20  pitz
//      varoccur: Test, ob in einem Term eine (beliebige) Variable vorkommt.
//
//      Revision 0.0  1991/08/09  08:18:19  pitz
//      Initiale Version
//
//=============================================================================
*/

#ifndef __TERM
#define __TERM

#include "error.h"
#include "memory.h"
#include "polynom.h"
#include "vartree.h"


/*
//-----------------------------------------------------------------------------
//      Typendefinitionen
//-----------------------------------------------------------------------------
*/

typedef struct termcell { function          fcode;
                          long              weight;
                          short             arity;
                          #ifdef MEMDEBUG
                            short           debug;
                          #endif
                          struct termcell   *argument[0]; } term;

typedef struct { char       ident[IDENTLENGTH];
                 short      weight;
                 short      preorder;
                 short      arity;
                 short      cpweight;
                 long       cancellation;
                 polynom    *poly;
                 term       *freelist;              } FunctionInfo;

typedef struct { short      type;
                 short      weight[MAXFUNCTION];
                 short      preorder[MAXFUNCTION];  } OrderInfo;


/*
//-----------------------------------------------------------------------------
//      Datendeklaration
//-----------------------------------------------------------------------------
*/

extern  function        FuncCount;
extern  variable        VarCount;
extern short            ArityMax;
extern  FunctionInfo    Function [MAXFUNCTION];
extern  short           OrderCount;
extern  OrderInfo       Order    [MAXORDER];

extern  long            Bit[32];


/*
//-----------------------------------------------------------------------------
//      Makrodefinitionen
//-----------------------------------------------------------------------------
*/

#define varp(term)      (term->fcode < 0)
#define funcp(term)     (term->fcode > 0)
#define constp(term)    (term->arity == 0)

#define Arity(fn)       Function[fn].arity
#define Weight(fn)      Function[fn].weight
#define FreeList(fn)    Function[fn].freelist

#define CheckCancel(fn, x)  (Function[fn].cancellation & Bit[x])

#define LPOFGreater(t1, t2) (Function[t1->fcode].preorder < Function[t2->fcode].preorder)
#define RPOFGreater(t1, t2) (Function[t1->fcode].preorder < Function[t2->fcode].preorder)

#define NewVariable        --VarCount
#define IncVarCount(inc)   VarCount -= inc


/*
//-----------------------------------------------------------------------------
//      Prototypen
//-----------------------------------------------------------------------------
*/

void        initterm ( void );

function    FindFunction ( char *ident );
function    NewFunction  ( char *ident );

void        SetOrdertype    ( short number, short type );
void        SetArguments    ( function fn, short arguments );
void        SetCancellation ( function fn, short cancelation );
void        SetWeight       ( function fn,
			      short number, short preorder, short weight );

bool        CheckFunctions ( void );
void        PrintFunctions ( void );

term        *newterm   ( function fcode );
void        deletet    ( term *t );
void        deleteterm ( term *t );

term        *copyterm ( term *t );
bool        occur     ( variable var, term *t );
bool        foccur    ( function f, term *t );

bool        equal  ( term *t1, term *t2 );
bool        member ( term *t1, term *t2 );

long        depth  ( term *t );

void        printterm ( term *t );

void        prettyprint  ( term *t, vartree **vars, variable *counter );
void        fprettyprint ( FILE *stream, term *t, vartree **vars, variable *counter );

long        sprettylen   ( term *t, vartree **vars, variable *counter );
void        sprettyprint ( char **ptr, term *t, vartree **vars );

void        prettyterm ( term *t );
void        fprettyterm ( FILE *stream, term *t );

bool        varoccur  ( term *t );
void        newvars   ( term *t, vartree **vars );
void        tnewvars  ( term *t );

void        getvars   ( term *t, vartree **vars );
vartree     *tgetvars ( term *t );

term        *copynew  ( term *t, vartree **vars );
term        *tcopynew ( term *t );

long        getweight  ( term *t );
long        lgetweight ( term *t );
bool        checkweight( term *t );

short       count_func  ( term *t );
short       count_var   ( term *t );

void        ReorgTerm   ( bool silence );

#endif
