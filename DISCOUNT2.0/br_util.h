/* Header of br_util.c: T.L. to reach ANSI standard */
/* Adapted to DISCOUNT2.0 4.12.1996 Stephan Schulz  */

#ifndef BR_UTIL_H
#define BR_UTIL_H

#include <netinet/in.h>
#include "br_types.h"

/* global vars */
extern tReceiver          br_Receivers[];		/* Data field of all participating hosts */
extern tString            br_Master;			/* actual master */
extern tString            br_Hostname;			/* name of host itself */
extern tbool              br_FirstTime;			/* TRUE, if host has not been master yet */
extern tbool              br_FullConnected;		/* TRUE, if host is connected to all other */
extern int                br_MissingConnections;	/* number of connections not established yet */
extern int                br_nReceivers;		/* number of participating hosts */
extern int                br_Broadcastsock;		/* broadcast socket */
extern struct sockaddr_in br_BroadcastAddress;		/* broadcast address */
extern char               br_Databuffer[];		/* data buffer */
extern int                br_DatabufferPos;		/* reading/writing position in data buffer */
extern int                br_BroadcastNumber;		/* broadcast counter */
extern int                br_BroadcastPort;		/* broadcast port */
extern int                br_AcceptPort;		/* port for accepting messages */
extern int                br_AcceptSock;		/* socket for accepting messages */

/* functions */
long br_GetBroadcastAddress (int sock);
int br_OpenAcceptSock (void);
int br_CloseAcceptSock (void);
int br_SendPtP (int sock, tMessage *msg);
int br_RecvPtP (int sock, tMessage *msg);
int br_ConnectPtP (int sock, char *host, int port);
int br_AcceptPtP (int sock, int *newsock);
int br_GetHostId (char *host);
int br_GetConnection (char *host);
int br_SetMaster (char *newmaster);

#endif
