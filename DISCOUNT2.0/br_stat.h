/*

  Modul:        br_stat.h

  benutzt:      -

  includes:     br_types.h
                br_time.h

		stdio.h

  exportiert:   MAXBROADCASTS
                tStatistic

  privat:       -

  Beschreibung: enthaelt alle neuen Datentypen und alle
                Konstanten die Statistik betreffend
		
  Autor:        Juergen Lind


  Aenderungen seit 01.03.1993: -

*/
#ifndef __STAT__
#define __STAT__


/*
  Include-Dateien
*/
#include <stdio.h>
#include "br_types.h"
#include "br_time.h"



#define MAXBROADCASTS 100

typedef struct stat{
	int    BroadcastCount;
	int    SendCount;
	int    Packets;
	int    AcknReq;
	int    Bytes;
	int    Ints;
	int    Strings;
	int    PacketsLost;
	int    AcknReqLost;
	long   Duration;
	struct brstat{
			tbool Sender;
			int   Packets;
			int   AcknReq;
			int   Bytes;
			int   PacketsLost;
			int   AcknReqLost;
			long  Duration;
			struct recvstat{
				int PacketsLost;
				int AcknReqLost;
			} Receivers[MAXRECEIVERS];
	} Broadcast[MAXBROADCASTS];
} tStatistic;


#endif





