/* Header of br_send.c: T.L. to reach ANSI standard */
/* Adapted to DISCOUNT2.0 4.12.1996 Stephan Schulz  */

#ifndef BR_SEND_H
#define BR_SEND_H

/* #include "systeminclude.h"  */

int br_OpenSendBroadcast (void);
int br_CloseSendBroadcast (void);
int br_ConfigureBroadcast (char (*hosts)[MAXHOSTNAMELEN], int num);
void br_WriteInt (int val);
void br_WriteLong (long val);
void br_WriteData (char *buf, int len);
int br_FlushBuffer (void);

#endif

