/*****************************************************************************/
/*                                                                           */
/*  Projekt      : TEAM-COMPLETION                                           */
/*                                                                           */
/*  Modul        : comphase                                                  */
/*                                                                           */
/*  Author       : Martin Kronenburg, 1993                                   */
/*                                                                           */
/*  Beschreibung : Kommunikation zwischen Leiter und anderen Experten        */
/*                 waehrend eines Teammeatings.                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*                                                                           */
/*****************************************************************************/

#ifndef __COMPHASE
#define __COMPHASE

/*****************************************************************************/
/*                                                                           */
/*                          Funktionsdefinitionen                            */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/*                                                                           */
/*                        exportierte Funktionen                             */
/*                                                                           */
/*****************************************************************************/

void ReceiveJudges ( void );
void SendJudges ( void );

void SendLeaderInfo ( void );
void ReceiveLeaderInfo ( void );

void ReceiveResults ( void );
void SendResults ( void );

void SendWinnerNo ( void );
void ReceiveWinnerNo ( void );

void SendCycleTime ( void );
void ReceiveCycleTime ( void );

void DistributeTeam ( void );
void ReceiveNextConfiguration ( void );


#endif
