/*-----------------------------------------------------------------------

File  : learn_tsmexperts.h

Author: Stephan Schulz

Contents
 
  Definitions for TSM-based experts and specialized auxialliary
  functions. 

Changes

<1>31.3.1997 new

-----------------------------------------------------------------------*/

#ifndef learn_tsmexperts

#define learn_tsmexperts

#include "learn_tsm.h"
#include "learn_exaselect.h"

/*---------------------------------------------------------------------*/
/*                    Data type declarations                           */
/*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*/
/*                Exported Functions and Variables                     */
/*---------------------------------------------------------------------*/

long   CreateTSMFromKB(TSMDesc_p desc, bool complete);
long   CreateOTSMFromKB(TSMDesc_p desc, bool complete);
double StandardTSMWeight(termpair *tp);
double StandardOTSMWeight(termpair *tp);

#endif

/*---------------------------------------------------------------------*/
/*                        End of File                                  */
/*---------------------------------------------------------------------*/

