/*
//=============================================================================
//      Projekt:        TEAM-COMPLETION
//
//      Modul:          error
//
//      Author:         Werner Pitz, 1991
//
//      Beschreibung:   Dieses Modul uebernimmt die Behandlung
//                      von schwerwiegenden Fehlern.
//-----------------------------------------------------------------------------
//      $Log: error.c,v $
//      Revision 0.0  1991/08/09  08:18:19  pitz
//      Initiale Version
//
//=============================================================================
*/

#include    <stdio.h>

#ifdef ATARI
    #include    <stdlib.h>
#endif

#include    "error.h"


/*
//-----------------------------------------------------------------------------
//      Implementierung
//-----------------------------------------------------------------------------
*/

bool    ParallelMode = false;
bool    SilentMode   = false;
bool    NullMode     = false;


/*
//-----------------------------------------------------------------------------
//  Funktion:       Error
//
//  Parameter:      funcname    Name der unterbrochenen Funktion
//                  errormsg    Klartext-Fehlermeldung
//
//  Beschreibung:   Ausgabe einer Fehlermeldung
//
//  Seiteneffekt:   Programm wird terminiert
//                  Rueckgabewert -1
//-----------------------------------------------------------------------------
*/

#ifdef __GNUC__
volatile
#endif
void    Error ( char *funcname, char *errormsg )
{
    printf ( "\n" );
    printf ( "******\n" );
    printf ( "******  Es ist ein Fehler aufgetreten.\n" );
    printf ( "******\n" );
    printf ( "******  Funktion:     %s\n", funcname );
    printf ( "******  Meldung:      %s\n", errormsg );
    printf ( "******\n\n" );
    flush  ();
/*     if (!(SilentMode || NullMode))
        getchar (); */
    /* Changed by StS: In sequential mode the program should not hang
       in an erroe message but terminate (with errorcode, of course) */
    if(ParallelMode && !(SilentMode || NullMode))
    {
       getchar();
    }
    exit ( -1 );
}
