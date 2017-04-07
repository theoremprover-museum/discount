/*
//=============================================================================
//      Projekt:        TEAM-COMPLETION
//
//      Header:         order
//
//      Author:         Werner Pitz, 1991
//
//      Beschreibung:   Dieses Modul enthaelt die Implementierungen der
//                      Ordnungen.
//
//      Funktionen:     SetOrdering     Einstellen der Ordnung
//                      GetOrdering     Ermitteln der Ordnung
//                      Compare         Vergleich von zwei Termen gemaess der
//                                      aktuellen Ordnung
//                      GreaterThan     Vergleich von zwei Termen gemaess der
//                                      aktuellen Ordnung
//-----------------------------------------------------------------------------
//      $Log: order.h,v $
//      Revision 0.0  1991/08/09  08:17:41  pitz
//      Initiale Version
//
//=============================================================================
*/

#ifndef     __ORDER
#define     __ORDER


#include "scanner.h"
#include "term.h"

/*
//-----------------------------------------------------------------------------
//      Prototypen
//-----------------------------------------------------------------------------
*/

extern short   (*Compare)     ( term *t1, term *t2 );
extern bool    (*GreaterThan) ( term *t1, term *t2 );
extern short   Ordering;


#ifdef  ANSI

    void    SetOrdering ( short ord );
    short   GetOrdering ( void );

#endif

#endif
