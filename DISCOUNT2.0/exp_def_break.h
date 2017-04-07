/*****************************************************************************/
/*                                                                           */
/*  Projekt      : TEAM-COMPLETION                                           */
/*                                                                           */
/*  Modul        : exp_def_break                                             */
/*                                                                           */
/*  Author       : Martin Kronenburg, 1993                                   */
/*                                                                           */
/*  Beschreibung : In diesem Modul werden alle Funktionen zur Verfuegung ge- */
/*                 stellt, die fuer die Klassifizierung der Experten, die im */
/*                 letzten Zyklus eingesetz waren, benoetigt werden.         */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*****************************************************************************/

#ifndef  __EXP_DEF_BREAK
#define  __EXP_DEF_BREAK


/*****************************************************************************/
/*                                                                           */
/*                               Konstanten                                  */
/*                                                                           */
/*****************************************************************************/
/* verwendet in DefineBreakOffs */
#define MIN_INSPECTED_X        4
#define EXP_PART               4
#define EXTRA_EXP1             2
#define LOSER_PART             2
#define EXTRA_EXP2             2
#define LOSER_PERCENT          0.8
#define EXTRA_EXP3             1
#define MIN_BETTER_EXP         2
#define DIFFERENCE_TO_LOSER  800

/* verwendet in loser_ganz_schlecht */
#define DIFF_PERCENT         0.2

/*****************************************************************************/
/*                                                                           */
/*                        exportierte Variablen                              */
/*                                                                           */
/*****************************************************************************/
extern  int           min_inspected_x;
extern  int           exp_part;
extern  int           extra_exp1;
extern  int           loser_part;
extern  int           extra_exp2;
extern  double        loser_percent;
extern  int           extra_exp3;
extern  int           min_better_exp;

/* Die folgenden Variablen werden in DefineBreakOffs erlaeutert              */
extern short          MaxLookAtExp;
extern short          MaxBetterExp;

/* Anzahl der Experten, die bisher beurteilt wurden.                         */
extern short          LookAtExp;

/* Anzahl der Experten, die bisher beurteilt wurden und besser als die Loser */
/* waren.                                                                    */
extern short          BetterExp;

/*****************************************************************************/
/*                                                                           */
/*                        exportierte Funktionen                             */
/*                                                                           */
/*****************************************************************************/

void abbruch_daten_aktualisieren ( double bewertung );

bool TestFurtherExperts ( void );

void DefineBreakOffs ( void );

#endif
