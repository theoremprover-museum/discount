/*
//=============================================================================
//      Projekt:        TEAM-COMPLETION
//
//      Header:         systime
//
//      Author:         Werner Pitz, 1991
//
//      Beschreibung:   Bestimmung der Systemzeit
//-----------------------------------------------------------------------------
//      $Log: systime.h,v $
//      Revision 0.0  1991/08/09  08:18:19  pitz
//      Initiale Version
//
//=============================================================================
*/

#ifndef __SYSTIME
#define __SYSTIME

#include <sys/time.h>
#include "error.h"

/*
//-----------------------------------------------------------------------------
//      Funktionsvereinbarungen
//-----------------------------------------------------------------------------
*/

extern long     cputime;


void    InitKillHandler ( void );

void    InitInterruptHandler ( void );

long    systime ( void );

void    settimer ( long sec, bool *bptr );

void    fprinttime ( FILE *log_f );


#endif
