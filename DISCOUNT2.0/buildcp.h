/*
//=============================================================================
//      Projekt:        TEAM-COMPLETION
//
//      Header:         buildcp
//
//      Author:         Werner Pitz, 1991
//
//      Beschreibung:   Erzeugung kritischer Paare
//-----------------------------------------------------------------------------
//      $Log: buildcp.h,v $
//      Revision 0.2  1991/09/05  14:27:18  pitz
//      special_factor nach cpweight.* ausgelagert.
//      Neue Funktion SpecialWeight in cpweight.* ubernimmt die
//      Bewertung von speziellen kritischen Paaren.
//
//      Revision 0.1  1991/09/05  12:51:28  pitz
//      Unterstuetzung von special_factor implementiert.
//
//      Revision 0.0  1991/08/09  08:18:19  pitz
//      Initiale Version
//
//=============================================================================
*/

#ifndef     __BUILDCP
#define     __BUILDCP

/*
//-----------------------------------------------------------------------------
//      sichtbare Datendeklarationen
//-----------------------------------------------------------------------------
*/

extern  termpair    *prule;
extern  termpair    *pequ;
extern  termpair    *pgoal;
extern  bool        KapurCriteria;


/*
//-----------------------------------------------------------------------------
//      Funktionsdeklarationen
//-----------------------------------------------------------------------------
*/

void    CPRuleRule  ( termpair *rule );
void    CPRuleEqu   ( termpair *equ  );
void    CPEquRule   ( termpair *rule );
void    CPEquEqu    ( termpair *equ  );

void    ParaRule    ( termpair *rule );
void    ParaNewRule ( termpair *goal );
void    ParaEqu     ( termpair *equ );
void    ParaNewEqu  ( termpair *goal );
void    ParaGoal    ( termpair *goal );

bool    UnifyGoal   ( termpair *goal );

#endif

