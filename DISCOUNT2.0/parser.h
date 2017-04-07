/*
//=============================================================================
//      Projekt:        TEAM-COMPLETION
//
//      Header:         parser
//
//      Author:         Werner Pitz, 1991
//
//      Beschreibung:   Einlesen einer Aufgabenstellung
//
//      Funktionen:     Parse       Einlesen einer Spezifiaktion
//-----------------------------------------------------------------------------
//      $Log: parser.h,v $
//      Revision 0.2  1991/09/24  12:18:39  pitz
//      weight_flag ermoeglicht die Bewertung der initialen Gleichungen.
//
//      Revision 0.1  1991/08/09  10:08:01  pitz
//      Die Variable SkolemGoal wird beim Einlesen eines Ziels gesetzt.
//      Sie wird true, falls keine Variable im Ziel vorkommt.
//
//      Revision 0.0  1991/08/09  08:18:19  pitz
//      Initiale Version
//
//=============================================================================
*/

#ifndef __PARSER
#define __PARSER

extern  bool    weight_flag;

/*
//-----------------------------------------------------------------------------
//      Funktionsvereinbarungen
//-----------------------------------------------------------------------------
*/

#ifdef  ANSI

    void         ClearVariables ( void );
    variable     FindVariable   ( char *ident );
    variable     CreateVariable ( char *ident );
    void    Parse      ( char *filename, char *example );
    void    PrintFile  ( char *name );

#endif

#endif
