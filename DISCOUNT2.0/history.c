/*
//=============================================================================
//      Projekt:        TEAM-COMPLETION
//
//      Modul:          history
//
//      Author:         Werner Pitz, 1991
//
//      Beschreibung:   Verwaltung der Geschichte aller Prozesses.
//-----------------------------------------------------------------------------
//      $Log: history.c,v $
//      Revision 0.1  1991/08/19  09:49:40  pitz
//      Fehlermeldungen mit Dateiangabe.
//
//      Revision 0.0  1991/08/09  08:18:19  pitz
//      Initiale Version
//
//=============================================================================
*/

#include    <stdio.h>
#include    <string.h>

#ifndef    ATARI
    #include    <sys/param.h>
    #include    <sys/types.h>
    #include    <sys/socket.h>
    #include    <netinet/in.h>
    #include    <netdb.h>
#endif

#ifdef    ATARI
    #include    <stdlib.h>
    #include    <ext.h>
    
    void    bcopy ( void*, void*, long );
#endif

#include    "defines.h"
#include    "error.h"
#include    "vartree.h"
#include    "polynom.h"
#include    "term.h"
#include    "termpair.h"
#include    "subst.h"
#include    "domain.h"
#include    "referee.h"
#include    "history.h"
#include    "scanner.h"
#include    "expert.h"
#include    "team.h"


/*
//-----------------------------------------------------------------------------
//      Typen- und Datendefinitionen
//-----------------------------------------------------------------------------
*/

typedef struct Tentry   { report          Report;
                          struct Tentry   *next;    } Entry;


static Entry   *History [MAXEXPERT]     = { NULL };
static Entry   *Last    [MAXEXPERT]     = { NULL };



/*
//-----------------------------------------------------------------------------
//  Funktion:       AddReport
//
//  Parameter:      process     Nummer des Prozesses der
//                  rep         Eintrag fuer diesen Prozess
//-----------------------------------------------------------------------------
*/

void    AddReport ( short process, report *rep )
{
    Entry       *ptr;
    termpair    *pair;
    short       oldtype;
    int         i;

    if ((ptr = Malloc (sizeof (Entry))) == NULL)
        Error ( __FILE__ ": "  "AddReport", "Zuwenig Speicher." );

    bcopy ((char *)rep, (char*)(&(ptr->Report)), sizeof (report));
    ptr->next = NULL;

    for (i = 0; i < rep->rule_count; i++)
    {
        pair = rep->rule[i];
        oldtype = pair->type;
        pair->type = RULE;
        ptr->Report.rule[i] = (termpair *)sprinttpair ( pair );
        pair->type = oldtype;
    }
    for (i = 0; i < rep->equ_count; i++)
    {
        pair = rep->equ[i];
        oldtype = pair->type;
        pair->type = EQUATION;
        ptr->Report.equ[i] = (termpair *)sprinttpair ( pair );
        pair->type = oldtype;
    }

    if (Last[process])
        Last[process] = Last[process]->next = ptr;
    else
        History[process] = Last[process] = ptr;
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       PrintReport
//
//  Parameter:      rep     Zeiger auf auszugebenden Report
//
//  Beschreibung:   Ausgabe eines Reports
//-----------------------------------------------------------------------------
*/


void    PrintReport ( report *rep )
{
    short   i;

    if (rep->masterflag)
    {
        printf ( "    ***********************\n" );
        printf ( "    ****  TEAM-Master  ****\n" );
        printf ( "    ***********************\n\n" );
    }

    printf ( "    Laufzeit: %6ld.%03ld Sekunden.\n",   rep->runtime/1000, rep->runtime%1000 );
    printf ( "    CPU-Zeit: %6ld.%03ld Sekunden. (Auslastung %2ld %%)\n\n", 
                        rep->cputime/1000, rep->cputime%1000, 
                        (rep->cputime *100) / rep->runtime );

    SetConfiguration ( false );

    if (rep->rule_count)
    {
        printf ( "    Ausgewaehlte Regeln:\n" );
        for (i = 0; i < rep->rule_count; i++ )
            printf ( "    %2d. %s\n", i+1, (char *)rep->rule[i] );
        printf ( "\n" );
    }

    if (rep->equ_count)
    {
        printf ( "    Ausgewaehlte Gleichungen:\n" );
        for (i = 0; i < rep->equ_count; i++ )
            printf ( "    %2d. %s\n", i+1, (char *)rep->equ[i] );
        printf ( "\n" );
    }

    flush ();
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       PrintHistory
//
//  Parameter:      -keine-
//
//  Beschreibung:   Ausgabe einer Fehlermeldung
//-----------------------------------------------------------------------------
*/

void    PrintHistory ( void )
{
    #ifndef NOHISTORY

    Entry   *ptr [MAXEXPERT];
    short   phase = 0;
    short   i;

    for (i = 0; i < MAXEXPERT; i++)
        ptr[i] = History[i];

    while (ptr[0])
    {
      printf ( "================================================================================\n\n" );
        printf ( "Phase        %4d\n", phase );

        phase++;
        for (i = 0; i < MAXEXPERT; i++)
        {

            if (ptr[i])
            {
                printf ( "\n-- Prozess %d ---------------------------------------------------\n\n", i );
                PrintReport (& (ptr[i]->Report) );
            }
            ptr[i] = (ptr[i]) ? ptr[i]->next
                              : NULL;
        }

        printf ( "\n" );
    }
      printf ( "================================================================================\n\n" );

    #endif
}
