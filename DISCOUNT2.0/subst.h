/*
//=============================================================================
//      Projekt:        TEAM-COMPLETION
//
//      Header:         subst
//
//      Author:         Werner Pitz, 1991
//
//      Beschreibung:   Substitutionen
//
//      Datentypen:     subst
//-----------------------------------------------------------------------------
//      $Log: subst.h,v $
//      Revision 0.1  1991/08/21  07:40:28  pitz
//      Kopieren von Substitutionen durch copysubst.
//
//      Revision 0.0  1991/08/09  08:18:19  pitz
//      Initiale Version
//
//=============================================================================
*/

#ifndef __SUBST
#define __SUBST


#include "termpair.h"

/*
//-----------------------------------------------------------------------------
//      Typendeklaration
//-----------------------------------------------------------------------------
*/

typedef struct substcell { variable         code;
                           term             *tptr;
                           struct substcell *lson;
                           struct substcell *rson;
                           #ifdef MEMDEBUG
                             short          debug;
                           #endif
                                                        } subst;


/*
//-----------------------------------------------------------------------------
//      Funktionsvereinbarungen
//-----------------------------------------------------------------------------
*/

#ifdef  ANSI

    void    initsubst (void);

    subst   *newsubst ( variable code, term *t );
    void    deletesubst ( subst *cell );
    void    deletematch ( subst *cell );

    term    *dassoc ( subst *sigma, variable code );
    term    *assoc ( subst *sigma, variable code );

    term    *substitute ( subst *sigma, term *t );
    term    *dsubstitute ( subst *sigma, term **t );

    bool    combine ( subst **sigma, subst *tau );
    void    addsubst ( subst **sigma, variable vcode, term *t );

    void    printsubst ( subst *sigma );

    bool    unify ( term *t1, term *t2, subst **sigma );
    bool    match ( term *t1, term *t2, subst **sigma );
    bool    goal_in_goal_match( termpair *goal1, termpair *goal2 );

    subst   *copysubst ( subst *sigma );

    void    ReorgSubst ( bool silence );

#endif

#endif
