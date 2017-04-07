/*
//=============================================================================
//      Projekt:        TEAM-COMPLETION
//
//      Modul:          vartree
//
//      Author:         Werner Pitz, 1991
//
//      Beschreibung:   Funktionen zur Verwaltung von Variablenbaeumen.
//
//      Datentypen:     vartree
//
//      Funktionen:     VTclear     Loeschen eines Variablenbaums
//                      VTadd       Hinzufuegen einer Variablen in einen
//                                  Variablenbaum
//                      VTfind      Suchen einer Variablen in einem
//                                  Variablenbaum -> info liefern
//                      VTfindinc   Suchen einer Variablen in einem
//                                  Variablenbaum -> count++
//                      VTfindcount Suchen einer Variablen in einem
//                                  Variablenbaum -> count liefern
//                      VTless      Ueberpruefen, ob alle Variablen eines
//                                  Baums mindestens genausooft in einem
//                                  anderen Baum vorkommen
//                      VTpartof    Ueberpruefen, ob alle Variablen eines
//                                  Baums auch in einem anderen Baum
//                                  vorkommen (Anzahl nebensaechlich)
//                      VTprint     Ausgabe eines Baums (fuer Testzwecke)
//-----------------------------------------------------------------------------
//      $Log: vartree.h,v $
//      Revision 0.0  1991/08/09  08:18:19  pitz
//      Initiale Version
//
//=============================================================================
*/

#ifndef __VARTREE
#define __VARTREE


#include  "error.h"


/*
//-----------------------------------------------------------------------------
//      Typdefinitionen
//-----------------------------------------------------------------------------
*/

typedef struct vtree { variable      code[VTSIZE];
                       variable      info[VTSIZE];
                       short         count[VTSIZE];
                       struct vtree* son;

                       #ifdef MEMDEBUG
                         short       debug;
                       #endif
                                                        } vartree;


/*
//-----------------------------------------------------------------------------
//      Funktionsvereinbarungen
//-----------------------------------------------------------------------------
*/

void        VTinit      ( void );

void        VTclear     ( vartree **root );
void        VTadd       ( vartree **root, variable code, variable info );
variable    VTfind      ( vartree *root,  variable code );
variable    VTfindinc   ( vartree *root,  variable code );
short       VTfindcount ( vartree *root,  variable code );
bool        VTless      ( vartree *vt1, vartree *vt2 );
bool        VTpartof    ( vartree *vt1, vartree *vt2 );
void        VTprint     ( vartree *root );


#endif
