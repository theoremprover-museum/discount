/*
//=============================================================================
//      Projekt:        TEAM-COMPLETION
//
//      Header:         error
//
//      Author:         Werner Pitz, 1991
//
//      Beschreibung:   Dieses Modul uebernimmt die Behandlung
//                      von schwerwiegenden Fehlern.
//
//      Funktionen:     Error       Ausgabe einer Fehlermeldung
//                                  Termination des Programms
//-----------------------------------------------------------------------------
//      $Log: error.h,v $
//      Revision 0.0  1991/08/09  08:18:19  pitz
//      Initiale Version
//
//=============================================================================
*/

#ifndef     __ERROR
#define     __ERROR

#include "defines.h"

/*
//-----------------------------------------------------------------------------
//      Prototypen
//-----------------------------------------------------------------------------
*/

extern bool     ParallelMode;
extern bool     SilentMode;
extern bool     NullMode;


#ifdef __GNUC__
volatile
#endif
    void    Error ( char *funcname, char *errormsg );

#endif
