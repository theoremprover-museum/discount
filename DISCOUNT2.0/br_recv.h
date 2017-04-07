/* Header of br_send.c: T.L. to reach ANSI standard */
/* Adapted to DISCOUNT2.0 4.12.1996 Stephan Schulz  */

#ifndef BR_RECV_H
#define BR_RECV_H


int br_OpenRecvBroadcast (void);
int br_CloseRecvBroadcast (void);
void br_ReadInt (int *val);
void br_ReadLong (long *val);
void br_ReadData (char *buf, int len);

#endif

