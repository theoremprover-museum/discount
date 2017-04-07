#ifndef __PARSEREF
#define __PARSEREF


#include "defines.h"
#include "scanner.h"


/*****************************************************************************/
/*                                                                           */
/*                           externe Variablen                               */
/*                                                                           */
/*****************************************************************************/

extern bool LeiterVorhanden;

/*****************************************************************************/
/*                                                                           */
/*                           exportierte Funktionen                          */
/*                                                                           */
/*****************************************************************************/

void ParseRef ( char *filename , RefFrame *ref);

#endif
