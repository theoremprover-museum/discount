/*
//=============================================================================
//      Projekt:        TEAM-COMPLETION
//
//      Modul:          transfer
//
//      Author:         Werner Pitz, 1991
//
//      Beschreibung:   Datenuebertragung zwischen Experten
//-----------------------------------------------------------------------------
//      $Log: transfer.h,v $
//      Revision 0.0  1991/08/09  08:18:19  pitz
//      Initiale Version
//
//=============================================================================
*/


#ifndef    __TRANSFER
#define    __TRANSFER

/*
//-----------------------------------------------------------------------------
//      Makros
//-----------------------------------------------------------------------------
*/

#define    BUFSIZE      10240


/*
//-----------------------------------------------------------------------------
//      Funktionsvereinbarungen
//-----------------------------------------------------------------------------
*/

#ifdef  ANSI

    void      SendPolynom     ( int sock, polynom *poly );
    polynom   *ReceivePolynom ( int sock );

    void      SendPair     ( int sock, termpair *tp );
    termpair  *ReceivePair ( int sock );

    void      SendRules         ( int sock );
    void      SendEquations     ( int sock );
    void      SendCriticalPairs ( int sock );
    void      SendGoals         ( int sock );
    void      SendCriticalGoals ( int sock );

    void      ReceiveRules         ( int sock, bool neworder );
    void      ReceiveEquations     ( int sock, bool neworder );
    void      ReceiveCriticalPairs ( int sock );
    void      ReceiveGoals         ( int sock );
    void      ReceiveCriticalGoals ( int sock );

    void      SendAllPairs     ( int sock );
    void      ReceiveAllPairs  ( int sock, bool neworder );

    void      GetCpParents     ( void );
    void      GetCgParents     ( void );

    void      SendReport        ( int sock, report *rep );
    void      ReceiveReport     ( int sock, report *rep );

#endif

#endif

