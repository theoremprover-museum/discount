/*
//=============================================================================
//      Projekt:        TEAM-COMPLETION
//
//      Modul:          team
//
//      Author:         Werner Pitz, 1991
//
//      Beschreibung:   Verwaltung eines Teams
//-----------------------------------------------------------------------------
//      $Log: team.h,v $
//      Revision 0.2  1991/08/14  13:19:27  pitz
//      Teamprotokoll vor Uebertragung von Regeln, Gleichungen,
//      kritischen Paaren und Zielen.
//
//      Revision 0.1  1991/08/14  11:56:31  pitz
//      Direkter Wechsel zwischen Spezialisten ermoeglicht.
//
//      Revision 0.0  1991/08/09  08:18:19  pitz
//      Initiale Version
//
//=============================================================================
*/

#ifndef     __TEAM
#define     __TEAM

#include    "defines.h"
#include    "newmem.h"
#include    "domain.h"
#include    "expert.h"


/*
//-----------------------------------------------------------------------------
//      Konstanten fuer Modus
//-----------------------------------------------------------------------------
*/

#define     SINGLE              0x0001
#define     MASTER              0x0002
#define     EXPERT              0x0004
#define     SPECIAL             0x0008
#define     REFEREE             0x0010

/*
//-----------------------------------------------------------------------------
//      Typdefinition
//-----------------------------------------------------------------------------
*/

/* Dies ist ein allgemeiner Typ zum Anlegen einer Tabelle fuer eine bestimmte*/
/* Menge von Objekten, z.B. : Domaenen, Experten, Gutachter, Ordnungen, ...  */
typedef struct { char     obj_name[IDENTLENGTH];
                 short    obj_nr;
               } ObjEntry;

/*
//-----------------------------------------------------------------------------
//      aktuelle Einstellung
//-----------------------------------------------------------------------------
*/

extern short    MODUS;
extern char*    log_base;
extern char     logfile[MAXPATHLEN];

extern FILE*    log_f;


extern bool     PlanDocum;
extern FILE*    plan_doc;

/**************************************************/
/* Variablen f"ur den lobalen Zustand des Systems */
/**************************************************/
extern int      Input;
extern int      Hostpid  [MAXEXPERT];
extern short    CycleCount;
extern char     Execfile [MAXPATHLEN];

extern short    ThisHost ;
extern short    MasterHost;
extern short    WinnerHost;

extern long     GesamtGroesse;

/*********************************************************************/
/* Variablen, in denen der Experte seine aktuelle Einstellung ablegt */
/*********************************************************************/
extern ExpertConfig    OwnConfig;

extern short           beurteile_ga_param[MAXPARAM];
extern short           resultate_ga_param[MAXPARAM];

/*******************************************************************************/
/* Variablen, die mit Informationen aus dem Konfigurationsfile versorgt werden */
/*******************************************************************************/
extern short         HostCount;

extern char          DataDir[MAXPATHLEN];
extern char          ExpertDir[MAXPATHLEN];
extern char          RefereeDir[MAXPATHLEN];
extern char          DomainDir[MAXPATHLEN];
extern char          ProtocolDir[MAXPATHLEN];

extern short         ExpertCount;
extern short         RedSpecCount;

extern long          CycleTime[MAXCYCLE];

extern pairset       ReceivedEq;
extern pairset       ReceivedGoals;

/*
//-----------------------------------------------------------------------------
//      Funktionsvereinbarungen
//-----------------------------------------------------------------------------
*/


void    SetRefParams     ( void );
void    SetConfiguration ( bool silence );

void    StartXpert       ( char *chost, char *cfgfile );
void    StartTeam        ( char *cfgfile, char *problemfile );

void    StartCompletion  ( void );

void    TerminateTeam    ( void );


#endif
