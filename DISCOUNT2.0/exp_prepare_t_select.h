/*****************************************************************************/
/*                                                                           */
/*  Projekt      : TEAM-COMPLETION                                           */
/*                                                                           */
/*  Modul        : exp_prepare_t_select                                      */
/*                                                                           */
/*  Author       : Martin Kronenburg, 1993                                   */
/*                                                                           */
/*  Beschreibung : In diesem Modul werden alle Funktionen zur Verfuegung ge- */
/*                 stellt, die fuer die Vorbereitung der Expertenauswahl er- */
/*                 forderlich sind.                                          */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*****************************************************************************/

#ifndef  __EXP_PREPARE_T_SELECT
#define  __EXP_PREPARE_T_SELECT


/*****************************************************************************/
/*                                                                           */
/*                               Konstanten                                  */
/*                                                                           */
/*****************************************************************************/
#define NOLOSER        11111  /* Wert von BestLoser, wenn keine Verlierer    */
			      /* vorhanden.                                  */

/*****************************************************************************/
/*                                                                           */
/*                        exportierte Variablenn                             */
/*                                                                           */
/*****************************************************************************/
/* Bewertung des besten Losers; wird bei den Abbruchkriterien benoetigt.     */
extern double     BestLoser;

/*****************************************************************************/
/*                                                                           */
/*                        exportierte Funktionen                             */
/*                                                                           */
/*****************************************************************************/

void initialisiere_bewertungsliste ( void );

bool exp_aus_bewertungsliste_loeschen ( short exp_nr );

short exp_aus_bewertungsliste_lesen ( void );

void PrepareTeamSelection ( void );

#endif
