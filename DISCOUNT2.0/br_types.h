/*

  Modul:        br_types.h

  benutzt:      -

  includes:     -

  exportiert:   STRINGLEN
                MAXMSGLEN
		ACKNREQUESTS
		ACKNnPACKETS
		TIMES
		PACKETSIZE
		MAXPACKETS
		DATABUFSIZE
		TIMEOUTLENGTH
		MAXRECEIVERS

                tString
		tErrorcode
		tPacketType
		tOpcode
		tMessage
		tReceiver

  privat:       -

  Beschreibung: enthaelt alle neuen Datentypen und alle
                globalen Konstanten

  Autor:        Juergen Lind


  Aenderungen seit 01.03.1993: 

  11.03.1993  tMessage erweitert, CommunicationMessage enthaelt jetzt noch
              eine Komponente, in der die Empfaenger bei der ersten
	      Kontaktaufnahme zum Sender ihre Prozess-Id ablegen

  17.03.1993  Fehlermeldung BF (Buffer flushed) entfernt

  22.03.1993  BROADCASTSOCK und MULTISOCK werden ab sofort frei gewaehlt

*/
#ifndef __TYPES__
#define __TYPES__

#include "complet.h" /* ReproMode bekanntmachen... */

/*
  Konstanten
*/
/*
#define TALK 1
#define STATISTICS 1
*/


#define TRUE 1
#define FALSE 0


#define ERROR   -1
#define NOERROR 0


#define STRINGLEN 80


#define MAXMSGLEN 1472
#define ACKNREQUESTS 2
#define ACKNnPACKETS 100
#define TIMES        4
#define PACKETSIZE   1400
#define MAXPACKETS   TIMES * ACKNnPACKETS
#define DATABUFSIZE  MAXPACKETS*PACKETSIZE

/* Changed from 3 because of t-o-problems... StS */
#define TIMEOUTLENGTH 5

#ifdef PCL
#define RECEIVETIMEOUT 300
#else
#define RECEIVETIMEOUT (ReproMode? 300:20)
#endif



#define MAXRECEIVERS 10


#define INTERFACENAME "lo0"


#define BR_BUFSIZE 16384


/*
  Typdefinitionen
*/

typedef char tString[STRINGLEN];


typedef int tbool;


typedef enum{
	NE,
        IP,
	CGBA,
	COAS,
	CBNAS,
	CGPN,
	CCAS,
	NSH,
	CCTSH,
	ANP,
	CSM,
	CRM,
	EHL,
	COBS,
	CBNBS,
	CGH,
	EGHBN,
	WN,
	CORS,
	CSB,
	CRB,
	EIS,
	TO,
	MACK,
	WT,
	CCO,
	CRSO,
	COOF
} tErrorcode;


typedef enum{
	DataPacket,
	MissingPacket,
	AcknPacket,
	CommunicationMessage
} tPacketType;


typedef enum{
	OK,
	NotOK,
	Nop,
	SendAckn,
	ReadyToReceive,
	EndOfTransmission
} tOpcode;


typedef struct msg{
	tPacketType Type;
	tOpcode     Opcode;
	union cnt{
		struct dtapkt{
			int  BroadcastNumber;
			int  PacketNumber;
			int  DataEnd;
			char Data[PACKETSIZE];
		} DataPacket;

		struct msgpkt{
			int  BroadcastNumber;
			int  PacketNumber;
			char Data[PACKETSIZE];
		} MissingPacket;

		struct acknpkt{
			int  BroadcastNumber;
			int  PacketsMissing[ACKNnPACKETS+1];
		} AcknPacket;

		struct commmsg{
			int     nReceivers;
			int     MissingConnections;
			tString Hosts[MAXRECEIVERS];
			int     Ports[MAXRECEIVERS];
			int     Pid;
		} CommunicationMessage;
	} Contents;
} tMessage;


typedef struct recver{
	tString Host;
	int     Sock;
	int     Port;
	tbool   Ready;
	tbool   Ackned;
} tReceiver;

#endif








