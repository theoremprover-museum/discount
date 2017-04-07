/*
//=============================================================================
//      Projekt:        TEAM-COMPLETION
//
//      Header:         reduce
//
//      Author:         Werner Pitz, 1991
//
//      Beschreibung:   Reduktion und Normalformbildung
//
//      Funktionen:     reduce      Normalformbildung
//                      Rreduce     Reduktion mit einer Regel
//                      Ereduce     Reduktion mit einer Gleichung
//-----------------------------------------------------------------------------
//      $Log: reduce.h,v $
//      Revision 0.1  1992/03/25  12:56:17  pitz
//      reduce_pair eingefuehrt.
//
//      Revision 0.0  1991/08/09  08:18:19  pitz
//      Initiale Version
//
//=============================================================================
*/

#ifndef     __REDUCE
#define     __REDUCE

#include    "termpair.h"
#include    "order.h"


/*
//-----------------------------------------------------------------------------
//      Sichtbare Daten
//-----------------------------------------------------------------------------
*/

long    RedCount;


/*
//-----------------------------------------------------------------------------
//      Funktionsvereinbarungen
//-----------------------------------------------------------------------------
*/

#ifdef  ANSI

    bool    reduce ( term **t );

    bool    Rreduce ( termpair *rule, term **t );
    bool    Ereduce ( termpair *equ,  term **t );

    bool    reduce_pair ( termpair *pair );

    bool    CheckReduce ( term **t, termpair *pair1, termpair *pair2 );

#endif

#endif
