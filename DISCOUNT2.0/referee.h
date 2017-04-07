/*
//=============================================================================
//      Projekt:        TEAM-COMPLETION
//
//      Header:         referee
//
//      Author:         Werner Pitz, 1991
//
//      Beschreibung:   In diesem Modul werden die Referees fuer die
//                      Parallelvervollstaendigung implementiert
//-----------------------------------------------------------------------------
//      $Log: referee.h,v $
//      Revision 0.2  1991/09/19  12:42:18  pitz
//      Parameter fuer EXTENDED-Referee.
//
//      Revision 0.1  1991/08/14  11:58:19  pitz
//      Bewertung von NoReferee auf -10000000 gesetzt.
//
//      Revision 0.0  1991/08/09  08:18:19  pitz
//      Initiale Version
//
//=============================================================================
*/

#ifndef    __REFEREE
#define    __REFEREE

#include    "expert.h"

/*
//-----------------------------------------------------------------------------
//      Bedeutung der Parameter
//-----------------------------------------------------------------------------
*/
/* Die Parameter der verschiedenen Gutachter - egal ob Beurteile - oder */
/* FindeGuteResultate-Funktionen - werden ueber die folgeden Konstanten */
/* angesprochen. Diese koennen ohne Problem erweitert werden. Zu beach- */
/* ten ist, dass die Groesse MAXPARAM nicht ueberschritten werden darf, */
/* falls noetig muss dies Groesse hochgesetzt werden.                   */

/* Parameter fuer die Beurteile-Gutachter-Funktionen :                  */

/* Fuer die statistischen Beurteile-Gutachter, sowohl STATISTIC als auch*/
/* EXTENDED.                                                            */

#define     REF_SIZE_R           0
#define     REF_SIZE_E           1
#define     REF_SIZE_CP          2
#define     REF_SIZE_G           3
#define     REF_SIZE_CG          4
#define     REF_NEW_R            5
#define     REF_RED_R            6
#define     REF_DEL_R            7
#define     REF_NEW_E            8
#define     REF_RED_E            9
#define     REF_DEL_E           10
#define     REF_NEW_CP          11
#define     REF_DEL_CP          12
#define     REF_NEW_G           13
#define     REF_RED_G           14
#define     REF_NEW_CG          15
#define     REF_REDCOUNT        16
/* Wird von allen Beurteile-Gutachter verwendet */
#define     REF_NOMASTER        17

/* Fuer den Gutachter FEELGOOD */
#define     REF_LAST_CP          0
#define     REF_LAST_CG          1
#define     REF_PAST_FAC         2
#define     REF_NEXT_CP          3
#define     REF_NEXT_CG          4
#define     REF_FUTURE_FAC       5
#define     REF_CORRECCT_FAC     6


/* Parameter fuer die Finde-Gute-Resultate-Funktionen :                 */
/* Sowohl fuer LAST als auch STATISTIC                                  */

#define     REF_MAX_RULE         0      /* Maximale Zahl von Regeln,    */
#define     REF_MAX_EQU          1      /* Gleichungen, Zielen, die aus-*/
#define     REF_MAX_GOAL         2      /* gewaehlt werden.             */

#define     REF_RULE_LOW         3      /* Schwellwerte fuer die 3      */
#define     REF_EQU_LOW          4      /* Termpaartypen.               */
#define     REF_GOAL_LOW         5

#define     REF_RED_COUNT        6      /* Anz Reduktionen gesamt.      */
#define     REF_RED_RIGHT        7      /* Reduktionen rechte Seite     */
#define     REF_RED_LEFT         8      /* Reduktionen linke Seite      */
#define     REF_RED_EQU          9      /* Reduktionen von Gleichungen  */
#define     REF_RED_GOAL        10      /* Reduktionen von Zielen       */
#define     REF_SUBSUM          11      /* Anzahl von Subsumptionen     */

#define     REF_CP_COUNT        12      /* Anzahl von Kritischen Paaren */
#define     REF_CG_COUNT        13      /* Anzahl von Kritischen Zielen */


#define     REFFILESUFFIX      "ref"

#define     NOREFEREE           -1


/*
//-----------------------------------------------------------------------------
//      Typdefinitionen
//-----------------------------------------------------------------------------
*/

typedef struct  { bool          master;
                  int           config;
                  long          result;
                  short         masterflag;
                  short         rule_count;
                  short         equ_count;
                  short         goal_count;
                  termpair      *rule [MAXREFRULE];
                  termpair      *equ  [MAXREFEQU];
                  termpair      *goal [MAXREFEQU];
                  long          runtime;
                  long          cputime;
                  #ifdef MEMDEBUG
                    short       debug;
                  #endif
		} report;

/* Datenstruktur fuer das Abspeichern der Daten ueber einen Gutachter.        */
/* Die Belegung der Parameterliste haengt von der betrachteten Funktion ab;   */
/* die Indizierung erfolgt anhand der obigen Konstanten.                      */

typedef struct { char                ga_name[IDENTLENGTH];
                 short               beurteile_ga;
                 ParameterElement    beurt_paramliste[MAXPARAM];
                 short               resultate_ga;
                 ParameterElement    res_paramliste[MAXPARAM];
                 ObjectInFrameInfo   geeignete_exp[MAXEXP_ANZ];
                 short               exp_anz;
                 ObjectInFrameInfo   geeignete_dom[MAXDOM_ANZ];
                 short               dom_anz;
                 double              robust;
                 double              wissen;
                 ObjectInFrameInfo   aehnl_ga[MAXEXP_ANZ];
                 short               aehnl_ga_anz;
               } RefFrame;

/*
//-----------------------------------------------------------------------------
//      Datendefinitionen
//-----------------------------------------------------------------------------
*/

extern  report   referee_report;

extern  RefFrame RefGlobalInfo[MAXEXPERT];
extern  RefFrame DatabaseRef;


/*
//-----------------------------------------------------------------------------
//      Funktiondefinitionen
//-----------------------------------------------------------------------------
*/

#ifdef  ANSI

    long    Referee ( short rtype, short *rparam, short stype, short *sparam );

#endif

#endif
