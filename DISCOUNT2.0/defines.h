/*
//=============================================================================
//      Projekt:        TEAM-COMPLETION
//
//      Header:         defines
//
//      Author:         Werner Pitz, 1991
//
//      Beschreibung:   Dieser Header wird in alle Module eingebunden.
//                      Er enthaelt einige grundsaetzliche Einstellungen und
//                      Definitionen.
//-----------------------------------------------------------------------------
//      $Log: defines.h,v $
//      Revision 0.1  1992/01/28  14:26:35  pitz
//      TIMETAG Makro
//
//      Revision 0.0  1991/08/09  08:18:19  pitz
//      Initiale Version
//
//=============================================================================
*/

#ifndef __DEFINES
#define __DEFINES

#include <sys/param.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>





#define         ANSI

/* #define         PROTOCOL         */
/* #define         SCANCONFIG       */
/* #define         SHOWCP           */
/* #define         NOCPSEQUENCE     */
/* #define         PCL              */
/* #define         NOPAGEOPT        */
/* #define         MEMDEBUG         */
/* #define          TIMETAG         */
 #define         NOHISTORY        
 #define         NOPAGEOPT        


/*
//-------------------------------------------------------------------------------------
//    Typ-Definitionen
//-------------------------------------------------------------------------------------
*/

typedef enum { false, true } bool;      /* Datentyp Boolean             */
typedef signed char          byte;      /* Datentyp Byte                */

#define         CR          '\n'        /* Zeilenvorschub               */
#define         TAB         '\t'        /* Tabulator                    */
#define         BS          '\010'      /* BACKSPACE                    */

#define         LENGTH     1024         /* Maximale Zeilenlaenge        */
#define         IDENTLENGTH  64         /* Maximale Laenge eines Ident. */
#define         MAXLITERAL   64         /* Maximale Laenge eines Strings */

#define         VTSIZE       16         /* Groesse der Hashtables fuer  */
                                        /*   Variablenbaum              */
                                        /*  2er Potenz !                */
#define         VTMASK       15         /* Eins kleiner als VTSIZE      */

#define         SUBSIZE      16         /* Groesse der Hashtables fuer  */
                                        /*   Substitutionen             */
                                        /*  2er Potenz !                */
#define         SUBMASK      15         /* Eins kleiner als VTSIZE      */


#define         LISTALLOC   100         /* Min. Allocierung von         */
                                        /* Listenelementen              */
#define         POLYALLOC   100         /* Min. Allocierung von         */
                                        /* Polynomzellen                */
#define         TREEALLOC   10         /* Min. Allocierung von         */
                                        /* Baumknoten (vartree)         */
#define         SUBSTALLOC  100         /* Min. Allocierung von         */
                                        /* Substitutionszellen          */
#define         TERMALLOC  1000         /* Min. Allocierung von         */
                                        /* Func-Termelementen           */
#define         PAIRALLOC  1000         /* Min. Allocierung von         */
                                        /* Termpaare                    */
#define         VARALLOC   1000         /* Min. Allocierung von         */
                                        /* Var-Termelementen            */

#define         function    long        /* Funktionstyp                 */
#define         variable    long        /* Variablentyp =!= Funktionstyp*/
#define         MAXFUNCTION 100         /* Maximalzahl von Funktionen   */
#define         MAXORDER     10         /* Maximalzahl von Ordnungen    */
#define         MAXARITY     32         /* Maximale Stelligkeit einer Fkt */
#define         MAXAXIOMS   512         /* Maximale Anzahl der Axiome
                                           (wird nur für Beispielauswahl
                                           benoetigt)                   */
                                        /* Erg. von Termvergleichen     */
#define         TLESS        -1         /*   Kleiner                    */
#define         TEQUAL        0         /*   Gleich oder unvergleichbar */
#define         TGREATER      1         /*   Groesser                   */
#define         TANY          4711      /* unvergleichbar               */

#define         KBO_ALPHA     1         /* Gewicht von Variablen in KBO */

#define         MAXPARAXPERT 50         /* Maximalanzahl von Parametern */
                                        /*   fuer einen Experten        */
#define         MAXPARAREF   15         /* Maximalanzahl von Parametern */
                                        /*   fuer einen Referee         */
#define         MAXPARASELECT 15        /* Maximalanzahl von Parametern */
                                        /*   fuer Auswahl-Referee       */
#define         MAXEXPERT    50         /* Maximale Anzahl von Experten */
#define         MAXREDSPEC    3         /* Maximale Anzahl von Reduk-   */
					/* tionsspezialisten.           */
					/* Wird dieser Wert hochgesetzt */
					/* m"ussen neue Konstanten      */
					/* REDUCE_i eingef"uhrt werden  */

#define         MAXREFRULE   10         /* Maximale Anzahl von Regeln   */
                                        /*   die ein Referee auswaehlt. */
#define         MAXREFEQU    10         /* Maximale Anzahl von Gleichungen */
                                        /*   die ein Referee auswaehlt. */

#define         SYNCCODE     0x56781234 /* Synchronisationscode         */

#define         MAXGA_ANZ      10     /* maximale Zahl angegebener GA                */
#define         MAXDOM_ANZ     10     /* maximale Zahl angegebener Domaenene         */
#define         MAXEXP_ANZ     20     /* maximale Zahl von Experten in einer Liste   */


#define         min(x,y)  ((x < y) ? x : y)
#define         max(x,y)  ((x > y) ? x : y)
#define         ABS(x)    (((x)>0)?(x):(-1*(x)))

#define         Dummy(x) 
#define         DEBUG(x) printf (x); flush ()


#define     Malloc(x)   (void *)(malloc (x))
#define     Mfree(x)    (free (x))
#define     flush()     fflush (stdout)

#if !(defined(SVR4)||defined(LINUX))
void        fflush ();
void        printf ();
void        fprintf ();
#endif

#ifdef  SPARC
    #define         ALIGN     8         /* Alignment fur Speicherzugriff*/
#else
    #define         ALIGN     2         /* Alignment fur Speicherzugriff*/
#endif

#endif
