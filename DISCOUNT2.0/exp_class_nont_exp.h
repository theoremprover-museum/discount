/*****************************************************************************/
/*                                                                           */
/*  Projekt      : TEAM-COMPLETION                                           */
/*                                                                           */
/*  Modul        : exp_class_nont_exp                                        */
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

#ifndef  __EXP_CLASS_NONT_EXP
#define  __EXP_CLASS_NONT_EXP


/*****************************************************************************/
/*                                                                           */
/*                               Konstanten                                  */
/*                                                                           */
/*****************************************************************************/
/* verwendet in ClassifyNonWinners */
/* siehe dort die Beschreibung der Konstanten  */
#define DOMAIN_SUITED         10
#define WINNER_SUITED          8
#define J_NOT_IN_TEAM          1
#define NOT_JUDGED             2

/*****************************************************************************/
/*                                                                           */
/*                        exportierte Variablen                              */
/*                                                                           */
/*****************************************************************************/
extern  int           domain_suited;
extern  int           winner_suited;

extern  int           j_not_in_team;
extern  int           not_judged;

/*****************************************************************************/
/*                                                                           */
/*                        exportierte Funktionen                             */
/*                                                                           */
/*****************************************************************************/

void initialisiere_eignungsliste ( void );

short exp_aus_eignungsliste_lesen ( void );

void ClassifyNonTeamExperts( void );

#endif
