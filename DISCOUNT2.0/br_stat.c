/*

  Modul:        br_stat.c

  benutzt:      br_time.c
                br_error.c

  includes:     br_stat.h

  exportiert:   stat_Init ( ... )
                stat_IncPacketCount ( ... )
	        stat_IncBroadcastCount ( ... )
	        stat_PacketLost ( ... )
	        stat_AcknReqLost ( ... )
	        stat_AddInt ( ... )
	        stat_AddString ( ... )
	        stat_AddTime ( ... )
	        stat_ShowStatistic ( ... )

  privat:       -

  Beschreibung: Funktionen zur Erfassung von Datenmengen, Uebertragungs-
                zeiten etc.

  Autor:        Juergen Lind


  Aenderungen seit 01.03.1993: -

*/


/*
  Include-Dateien
*/
#include "br_stat.h"


/*
  externe Variablen
*/
extern tReceiver br_Receivers[MAXRECEIVERS];
extern tString   br_Master;
extern tString   br_Hostname;
extern int       br_nReceivers;
extern int       br_BroadcastNumber;



/*
	Funktion:      stat_Init ( statistic )

	Parameter:     tStatistic* statistic

	Rueckgabewert: -

	benutzt:       -

	Beschreibung:  -initialisiert eine Variable vom Typ
		        tStatistic

*/

void stat_Init ( statistic )

	tStatistic* statistic;

{
	int i;
	int j;


	statistic->BroadcastCount = 0;
	statistic->SendCount      = 0;
	statistic->Packets        = 0;
	statistic->AcknReq        = 0;
	statistic->Bytes          = 0;
	statistic->Ints           = 0;
	statistic->Strings        = 0;
	statistic->PacketsLost    = 0;
	statistic->AcknReqLost    = 0;
	for( i = 0; i < MAXBROADCASTS; i++ ){
		statistic->Broadcast[i].Sender      = FALSE;
		statistic->Broadcast[i].Packets     = 0;
		statistic->Broadcast[i].AcknReq     = 0;
		statistic->Broadcast[i].Bytes       = 0;
		statistic->Broadcast[i].PacketsLost = 0;
		statistic->Broadcast[i].AcknReqLost = 0;
		for( j = 0; j < br_nReceivers; j++ ){
			statistic->Broadcast[i].Receivers[j].PacketsLost = 0;
			statistic->Broadcast[i].Receivers[j].AcknReqLost = 0;

		};
	};

	statistic->Broadcast[0].Sender = TRUE; /* diese Funktion darf nur vom */
														/* Sender aufgerufen werden    */

};



/*
	Funktion:      stat_IncBroadcastCount ( statistic )

	Parameter:     tStatistic* statistic

	Rueckgabewert: -1: Fehler
		 	0: kein Fehler

	benutzt:       -

	Beschreibung:  -erhoeht den Broadcast-Zaehler von statistic

*/

int stat_IncBroadcastCount ( statistic )

	tStatistic* statistic;

{
	statistic->BroadcastCount++;
	if( strcmp( br_Master, br_Hostname ) == 0 ){
		statistic->SendCount++;
	};

	return( NOERROR );
};



/*
	Funktion:      stat_IncPacketCount ( statistic )

	Parameter:     tStatistic* statistic

	Rueckgabewert: -1: Fehler
		 	0: kein Fehler

	benutzt:       -

	Beschreibung:  -erhoeht den Packets-Zaehler von statistic und
		        den des aktuellen Broadcasts
*/

int stat_IncPacketCount ( statistic )

	tStatistic* statistic;

{
	statistic->Packets++;
	statistic->Broadcast[br_BroadcastNumber].Packets++;
	
	statistic->Broadcast[br_BroadcastNumber].Sender = TRUE;
	
	return( NOERROR );
};



/*
	Funktion:      stat_PacketLost ( statistic, hostid )

	Parameter:     tStatistic* statistic
		       int        hostid

	Rueckgabewert: -1: Fehler
			0: kein Fehler

	benutzt:       -

	Beschreibung:  -erhoeht den PacketLost-Zaehler von statistic, den
		        des aktuellen Broadcasts und den des Host mit der
		        entsprechenden Id

*/

int stat_PacketLost ( statistic, hostid )

	tStatistic* statistic;
	int        hostid;

{
	statistic->PacketsLost++;
	statistic->Broadcast[br_BroadcastNumber].PacketsLost++;
	statistic->Broadcast[br_BroadcastNumber].Receivers[hostid].PacketsLost++;

	return( NOERROR );
};



/*
	Funktion:      stat_IncAcknreqCount ( statistic )

	Parameter:     tStatistic* statistic

	Rueckgabewert: -1: Fehler
		 	0: kein Fehler

	benutzt:       -

	Beschreibung:  -erhoeht den AcknReq-Zaehler von statistic und
		        den des aktuellen Broadcasts
*/

int stat_IncAcknReqCount ( statistic )

	tStatistic* statistic;

{
	statistic->AcknReq++;
	statistic->Broadcast[br_BroadcastNumber].AcknReq++;

	return( NOERROR );
};



/*
	Funktion:      stat_AcknReqLost ( statistic, hostid )

	Parameter:     tStatistic* statistic
		       int        hostid

	Rueckgabewert: -1: Fehler
			0: kein Fehler

	benutzt:       -

	Beschreibung:  -erhoeht den AcknReqLost-Zaehler von statistic, den
		        des aktuellen Broadcasts und den des Host mit der
		        entsprechenden Id

*/

int stat_AcknReqLost ( statistic, hostid )

	tStatistic* statistic;
	int        hostid;

{
	statistic->AcknReqLost++;
	statistic->Broadcast[br_BroadcastNumber].AcknReqLost++;
	statistic->Broadcast[br_BroadcastNumber].Receivers[hostid].AcknReqLost++;

	return( NOERROR );
};



/*
	Funktion:      stat_AddInt ( statistic )

	Parameter:     tStatistic* statistic

	Rueckgabewert: -1: Fehler
			0: kein Fehler

	benutzt:       -

	Beschreibung:  -erhoeht den Ints-Zaehler von statistic
		        sowie die Bytes-Zaehler von statistic und dem
		        aktuellen Broadcast

*/

int stat_AddInt ( statistic )

	tStatistic* statistic;

{
	statistic->Bytes += sizeof( int );
	statistic->Ints++;
	statistic->Broadcast[br_BroadcastNumber].Bytes += sizeof( int );

	return( NOERROR );
};



/*
	Funktion:      stat_AddString ( statistic, size )

	Parameter:     tStatistic* statistic
		       int         size;

	Rueckgabewert: -1: Fehler
			0: kein Fehler

	benutzt:       -

	Beschreibung:  -erhoeht den Strings-Zaehler von statistic
		        sowie die Bytes-Zaehler von statistic und dem
		        aktuellen Broadcast

*/

int stat_AddString ( statistic, size )

	tStatistic* statistic;
        int         size;

{
	statistic->Bytes += size;
	statistic->Strings++;
	statistic->Broadcast[br_BroadcastNumber].Bytes += size;

	return( NOERROR );
};



/*
	Funktion:      stat_AddTime ( statistic, watch )

	Parameter:     tStatistic* statistic
	               tTime*      watch;

	Rueckgabewert: -1: Fehler
			0: kein Fehler

	benutzt:       -

	Beschreibung:  -erhoeht die duration-Komponente von statistic und die
	                des aktuellen Broadcasts

*/

int stat_AddTime ( statistic, watch )

	tStatistic* statistic;
        tTime*      watch;

{
	statistic->Duration += watch->elapsed;
	statistic->Broadcast[br_BroadcastNumber].Duration +=watch->elapsed;

	return( NOERROR );
};



/*
        Funktion:      stat_ShowStatistic ( statistic, output )

        Parameter:     tStatistic* statistic
                       char*       output

        Rueckgabewert: -1: Fehler
                        0: kein Fehler

        benutzt:       -

        Beschreibung:  -schreibt die Statistic in das File mit Namen
                        in output. Hat output den Wert NULL, so geht
                        die Ausgabe nach stdout

*/

int stat_ShowStatistic ( statistic, output )

	tStatistic* statistic;
	tString     output;

{
	FILE* out;
	int   savestdout;
	float percentage;
	int   i;
	int   j;


	if( output != NULL ){
		savestdout = dup( 1 );
		out = freopen( output, "w", stdout );
		if( out == NULL ){
			br_Error( "br_statistic.c/stat_ShowStatistic", COOF );
			return( ERROR );
		};
	};

	/*
	  hier kommen die Ausgaben
	*/
	printf( "Host: %s\n\n", br_Hostname );
	printf( "Bytes transferred:            %8d\n", statistic->Bytes );
	printf( "  Integers:                   %8d\n", statistic->Ints );
	printf( "  Strings                     %8d\n", statistic->Strings );
	printf( "Broadcasts:                   %8d\n", statistic->SendCount );
	printf( "Packets sent:                 %8d\n", statistic->Packets );
	printf( "AcknReq sent:                 %8d\n", statistic->AcknReq );

	if( statistic->Packets != 0 )
		percentage = (float)(statistic->PacketsLost) / (float)(statistic->Packets);
	else
		percentage = 0.0;
	printf( "total Number of Packets lost: %8d    (%#06.2f%%)\n",
				statistic->PacketsLost, percentage * 100.0 );

	if( statistic->AcknReq != 0 )
		percentage = (float)(statistic->AcknReqLost) / (float)(statistic->AcknReq);
	else
		percentage = 0.0;
	printf( "total Number of AcknReq lost: %8d    (%#06.2f%%)\n\n",
				statistic->AcknReqLost, percentage * 100.0 );
	printf( "total amount of Time: %4d sec %3d msec\n\n",
				(statistic->Duration) / 1000, (statistic->Duration) % 1000 );
	printf( "-----------------------------------------------------------------------\n\n" );

	for( i = 0; i < MAXBROADCASTS; i++){

		if( statistic->Broadcast[i].Sender ){

			printf( "Broadcast Nr. %-4d\n\n", i );
			printf( "Bytes transferred:      %8d\n", statistic->Broadcast[i].Bytes );
			printf( "Packets sent:           %8d\n", statistic->Broadcast[i].Packets );
			printf( "AcknReq sent:           %8d\n", statistic->Broadcast[i].AcknReq );

			if( statistic->Broadcast[i].Packets != 0 )
				percentage = (float)(statistic->Broadcast[i].PacketsLost)
								 / (float)(statistic->Broadcast[i].Packets);
			else
				percentage = 0.0;
			printf( "Number of Packets lost: %8d    (%#06.2f%%)\n",
				statistic->Broadcast[i].PacketsLost, percentage * 100.0 );

			if( statistic->AcknReq != 0 )
				percentage = (float)(statistic->Broadcast[i].AcknReqLost) / (float)(statistic->Broadcast[i].AcknReq);
			else
				percentage = 0.0;
			printf( "Number of AcknReq lost: %8d    (%#06.2f%%)\n\n",
						statistic->Broadcast[i].AcknReqLost, percentage * 100.0 );

			printf( "     Host               PacketsLost                AcknReqLost\n\n" );
			for( j = 0; j < br_nReceivers; j++ ){

				if( statistic->Broadcast[i].PacketsLost != 0 )
					percentage = (float)(statistic->Broadcast[i].Receivers[j].PacketsLost)
									 / (float)(statistic->Broadcast[i].PacketsLost);
				else
					percentage = 0.0;
				printf( "  %-10.10s      %8d (%#06.2f%%)",
							br_Receivers[j].Host, statistic->Broadcast[i].Receivers[j].PacketsLost, percentage * 100.0 );

				if( statistic->Broadcast[i].AcknReqLost != 0 )
					percentage = (float)(statistic->Broadcast[i].Receivers[j].AcknReqLost)
									 / (float)(statistic->Broadcast[i].AcknReqLost);
				else
					percentage = 0.0;
				printf( "          %8d (%#06.2f%%)\n",
							statistic->Broadcast[i].Receivers[j].AcknReqLost, percentage * 100.0 );
			};

			printf( "\nTime: %4d sec %3d msec\n\n\n",
						(statistic->Broadcast[i].Duration) / 1000, (statistic->Broadcast[i].Duration) % 1000 );

		};
	};

	/*
		Ende der Ausgaben
	*/
	if( output != NULL ){
		if( fclose( out ) == EOF ){
			br_Error( "br_statistic.c/ShowStatistic", CCO );
			return( ERROR );
		};

		if( fdopen( savestdout, "w" ) == (FILE*)NULL ){
			br_Error( "br_statistic.c/ShowStatistic", CRSO );
			return( ERROR );
		};
	};

	return( NOERROR );

};

