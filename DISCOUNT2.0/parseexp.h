#ifndef __PARSEEXP
#define __PARSEEXP


#include "complet.h"
#include "database.h"
#include "team.h"






/*****************************************************************************/
/*                                                                           */
/*                               Tydefinitionen                              */
/*                                                                           */
/*****************************************************************************/
/* Der folgende Typ gibt an, ob es sich bei dem gelesenen Expertenrahmen um  */
/* den Rahmen des DATABASE-Experten gehandelt hat (--> DAT ), um den Rahmen  */
/* eines Reduktionsspezialisten (--> RED ) oder um einen "normalen" Experten */
/* (--> EXP ).                                                               */
typedef enum { EXP, DAT, RED } ExpertType;

/*****************************************************************************/
/*                                                                           */
/*                           exportierte Funktionen                          */
/*                                                                           */
/*****************************************************************************/

double exp_10 ( int exp );

ExpertType ParseExpert ( char *filename , ExpertFrame *expert);

#endif
