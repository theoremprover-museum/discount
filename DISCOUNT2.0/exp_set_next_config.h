/*****************************************************************************/
/*                                                                           */
/*  Projekt      : TEAM-COMPLETION                                           */
/*                                                                           */
/*  Modul        : exp_set_next_config                                       */
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

#ifndef  __EXP_SET_NEXT_CONFIG
#define  __EXP_SET_NEXT_CONFIG


/*****************************************************************************/
/*                                                                           */
/*                               Konstanten                                  */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*                          exportierte Variablen                            */
/*                                                                           */
/*****************************************************************************/
short StartDomNr;

/*****************************************************************************/
/*                                                                           */
/*                        exportierte Funktionen                             */
/*                                                                           */
/*****************************************************************************/

void set_parameter ( ExpertConfig *expconfig );

void SetNextConfiguration ( void );

#endif
