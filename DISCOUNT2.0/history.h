/*
//=============================================================================
//      Projekt:        TEAM-COMPLETION
//
//      Header:         history
//
//      Author:         Werner Pitz, 1991
//
//      Beschreibung:   Verwaltung der Geschichte aller Prozesses.
//-----------------------------------------------------------------------------
//      $Log: history.h,v $
//      Revision 0.0  1991/08/09  08:18:19  pitz
//      Initiale Version
//
//=============================================================================
*/

#ifndef __HISTORY
#define __HISTORY

/*
//-----------------------------------------------------------------------------
//      Prototypen
//-----------------------------------------------------------------------------
*/

#ifdef ANSI

    void    AddReport    ( short process, report *rep );
    void    PrintReport  ( report *rep );
    void    PrintHistory ( void );

#endif

#endif
