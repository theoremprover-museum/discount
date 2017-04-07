/*

  Modul:        br_send.c

  benutzt:      br_util.c
                br_error.c

  includes:     br_types.h

                defines.h

                sys/types.h
                sys/time.h
                sys/param.h
                sys/socket.h
                netinet/in.h
                netdb.h
                net/if.h
                sys/sockio.h (sys/socketio.h under Linux)

  exportiert:   br_OpenSendBroadcast ( ... )
                br_CloseSendBroadcast ( ... )
                br_ConfigureBroadcast ( ... )
                br_WriteInt ( ... )
                br_WriteLong ( ... )
                br_WriteData ( ... )
                br_FlushBuffer (...)

  privat:       SendBroadcast ( ... )
                ReceiversReady ( ... )
                HandleAckn ( ... )


  Beschreibung: Protokollabwicklung auf Senderseite

  Autor:        Juergen Lind


  Aenderungen seit 01.03.1993: 

  02.03.1993  br_ConfigureBroadcast: anderer Parametertyp fuer Hostliste
              Funktion WriteHost( ... ) geloescht
  
  11.03.1993  br_OpenSendBroadcast umgestellt: Empfaenger schliessen
              sich mittels connect an den Sender an und identifizieren
              sich durch eine Nachricht, die u. a. auch die Prozess-Id
              der Empfaengerprozesse enthaelt

  17.03.1993  WriteData und WriteInt geben keinen Fehlercode mehr zurueck
              sondern brechen das Protokoll direkt ab

  22.03.1993  es werden keine festen Ports mehr verwendet, die Empfaenger
              schicken ihren Accept-Port in der Identifikationsnachricht
	      und der Sender verteilt die Informationen dann weiter
  08.11.1995  br_WriteLong in Anlehnung an br_WriteInt geschrieben (kopiert). (MK)

  04.12.1996  Uebernahme fuer DISCOUNT 2.0 (zwischendurch hat jemand
              einiges mit ANSI und SVR4 gemacht und nicht
	      Dokumentiert! (StS)
*/




/*
  Include-Dateien
*/
#if (defined SVR4) && !(defined LINUX)
#include <sys/systeminfo.h>
#endif /* SVR4  && !LINUX */
#include <stdlib.h>             /* added by T.L. */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <net/if.h>

#ifdef LINUX
#include <sys/socketio.h>
#else
#include <sys/sockio.h>
#endif /* LINUX */

#include "br_types.h"
#include "br_util.h"            /* added by T.L. to reach ANSI standard */
#include "br_error.h"           /* added by T.L. to reach ANSI standard */

#include "defines.h"
#include "scanner.h"
#include "vartree.h"
/*  #include "gmp.h" */
#include "polynom.h"
#include "term.h"
#include "termpair.h"
#include "domain.h"
#include "expert.h"
#include "referee.h"
#include "team.h"
#ifdef STATISTICS

#include "br_stat.h"
#include "br_time.h"
#include "br_error.h"
#include "br_send.h"            /* added by T.L. to reach ANSI standard */

#endif

/* #include "defines.h" */                              /* commented out by T.L. */

/*
  externe Variablen
*/
/* int             Hostpid[MAXEXPERT]; */               
              /* needed for abnormal termination s */ /* now included in br_util.h */


#ifdef STATISTICS

static tTime              br_Watch;

#endif



/*
  Vorwaertsdeklarationen
*/
static tbool ReceiversReady(void);
static int   HandleAckn(void);
static int   SendBroadcast (tMessage *msg);




/*

  Funktion:      br_OpenSendBroadcast ()

  Parameter:     -

  Rueckgabewert: -1: Fehler
                  0: kein Fehler

  benutzt:       br_util.c/br_GetBroadcastAddress( ... )
                 br_util.c/br_ConnectPtP( ... )
                 br_util.c/br_SendPtP( ... )

                 br_error.c/br_Error( ... )
                 br_error.c/br_PrintError( ... )

                 global/br_Broadcastsock
                 global/br_BroadcastAddress
                 global/SendInitDone
                 global/br_Receivers[]
                 global/br_nReceivers
                 global/br_FirstTime
                 global/br_FullConnected
                 global/br_Hostname
                 global/br_Master
                 global/br_DatabufferPos

  Beschreibung:  -oeffnet einen Datagram-Socket zur Versendung von 
                  Broadcastnachrichten (falls nicht schon offen)
                 -schreibt die Broadcast-Adresse des lokalen Netztes
                  nach 'BroadcastAddress'
                 -br_OpenSendBroadcast baut schrittweise einen vollstaendig
                  vermaschten Graph von Socketverbindungen zwischen allen
                  Hosts auf (naeheres siehe Dokumentation)
*/

int br_OpenSendBroadcast (void)

{
   struct sockaddr_in s;
   int                i;
   int                j;
   int                on = 1;
   tMessage           msg;
   tMessage           inmsg;
   int                sock;
   tbool              brsockok;
   
   
   if( br_FirstTime )
   {
      
#ifdef STATISTICS
      stat_Init( &br_BroadcastStatistic );
#endif
      
#if (defined SVR4) && !(defined LINUX)
      sysinfo(SI_HOSTNAME,   br_Hostname, (long)sizeof br_Hostname );
#else
      gethostname(  br_Hostname, sizeof br_Hostname );
#endif
      /* INTSIZE = sizeof( int ); */                          /* T.L. */
      
      br_Receivers[0].Sock = -1;
      
      for( i = 1; i < br_nReceivers; i++ )
      {
	 if( br_AcceptPtP( br_AcceptSock, &sock ) == -1 )
	    br_PrintError();
	 
	 if( br_RecvPtP( sock, &inmsg ) == -1 )
	    return( ERROR );
	 
	 j = br_GetHostId( inmsg.Contents.CommunicationMessage.Hosts[0] );
	 if( j == -1 )
	    return( ERROR );
	 else
	 {
	    br_Receivers[j].Sock = sock;
	    br_Receivers[j].Port = inmsg.Contents.CommunicationMessage.Ports[0];
	    Hostpid[j] = inmsg.Contents.CommunicationMessage.Pid;
	 };
	      
      };
	   
      msg.Type = CommunicationMessage;
      msg.Contents.CommunicationMessage.nReceivers = br_nReceivers;
      msg.Contents.CommunicationMessage.MissingConnections = br_MissingConnections;
      for( i = 0; i < br_nReceivers; i++ )
      {
	 strcpy( msg.Contents.CommunicationMessage.Hosts[i],
		 br_Receivers[i].Host );
	 msg.Contents.CommunicationMessage.Ports[i] = br_Receivers[i].Port;
      };
	   
      for( i = 1; i < br_nReceivers; i++ )
      {
	 if( br_SendPtP( br_Receivers[i].Sock, &msg ) == -1 )
	    return( ERROR );
      };
	   
      br_Broadcastsock = socket( AF_INET, SOCK_DGRAM, 0 );
      if( br_Broadcastsock == -1 )
      {
	 br_Error( "br_send.c/br_OpenSendBroadcast", COBS );
	 return( ERROR );
      };
	   
      s.sin_family = AF_INET;
      s.sin_addr.s_addr = INADDR_ANY;
      s.sin_port = htons( br_BroadcastPort );
      if( bind( br_Broadcastsock, (struct sockaddr *)&s, sizeof s) == -1 )
      {
	 /* cast by T.L. */
	 brsockok = FALSE;
      }
      else{
	 brsockok = TRUE;
      };
	   
      for( i = 1; i < br_nReceivers; i++ )
      {
	 if( br_RecvPtP( br_Receivers[i].Sock, &msg ) == -1 )
	    return( ERROR );
	 if( msg.Opcode == NotOK )
	 {
	    brsockok = FALSE;
	 };
      };
	   
      if( brsockok )
	 msg.Opcode = OK;
      else
	 msg.Opcode = NotOK;
	   
      for( i = 1; i < br_nReceivers; i++ )
      {
	 if( br_SendPtP( br_Receivers[i].Sock, &msg ) == -1 )
	    return( ERROR );
      };
	   
      while( !brsockok )
      {
	 
	 br_BroadcastPort = 1500 + (rand() % 4321 );
	 
#ifdef TALK
	 printf( "new BroadcastPort: %d\n", br_BroadcastPort );
#endif
	 
	 s.sin_family = AF_INET;
	 s.sin_addr.s_addr = INADDR_ANY;
	 s.sin_port = htons( br_BroadcastPort );
	 if( bind( br_Broadcastsock, (struct sockaddr *)&s,
		   sizeof s ) == -1 )
	 { /* cast by T.L. */
	    brsockok = FALSE;
	 }
	 else
	 {
	    msg.Type = CommunicationMessage;
	    msg.Contents.CommunicationMessage.Ports[0] = br_BroadcastPort;
		 
	    for( i = 1; i < br_nReceivers; i++ )
	    {
	       if( br_SendPtP( br_Receivers[i].Sock, &msg ) == -1 )
		  return( ERROR );
	    };
		 
	    brsockok = TRUE;
	    for( i = 1; i < br_nReceivers; i++ )
	    {
	       if( br_RecvPtP( br_Receivers[i].Sock, &msg ) == -1 )
		  return( ERROR );
	       if( msg.Opcode == NotOK ) 
		  brsockok = FALSE;
	    };
	 };
      };
	   
      setsockopt( br_Broadcastsock, SOL_SOCKET, SO_BROADCAST, (char*)&on, sizeof on );/* cast by T.L. */
	   
      br_BroadcastAddress.sin_family = AF_INET;
      br_BroadcastAddress.sin_addr.s_addr = br_GetBroadcastAddress( br_Broadcastsock );
      br_BroadcastAddress.sin_port = htons( br_BroadcastPort );
	   
      br_FirstTime = FALSE;
      br_FullConnected = TRUE;
   }
	
   if( !br_FullConnected )
   {
	   
      while( br_MissingConnections != 0 )
      {
	      
	 if( br_AcceptPtP( br_AcceptSock, &sock ) == -1 )
	    br_PrintError();
	      
	 if( br_RecvPtP( sock, &inmsg ) == -1 )
	    return( ERROR );
	      
	 j = br_GetHostId( inmsg.Contents.CommunicationMessage.Hosts[0] );
	 if( j == -1 )
	    return( ERROR );
	 else
	    br_Receivers[j].Sock = sock;
	      
	 br_MissingConnections--;
      }
	   
      br_FullConnected = TRUE;
	   
   }
	
   br_DatabufferPos = 0;
	
   return( NOERROR );
}



/*

  Funktion:      br_CloseSendBroadcast ()

  Parameter:     -

  Rueckgabewert: -1: Fehler
                  0: kein Fehler

  benutzt:       br_util.c/br_CloseAcceptSock ( ... )
             
                 global/br_Broadcastsock
                 global/br_Receivers[]

  Beschreibung: -schliesst den Broadcast- und alle Stream-Sockets

*/

int br_CloseSendBroadcast (void)

{
        int i;


        if( br_CloseAcceptSock() == -1 )
                return( ERROR );

        for( i = 0; i < br_nReceivers; i++ )
                if( br_Receivers[i].Sock > 0 )
                        if( close( br_Receivers[i].Sock ) == -1 )
                                return( ERROR );

        return( NOERROR );

}



/*

  Funktion:      br_ConfigureBroadcast ( hosts, num )

  Paramater:     char hosts[MAXEXPERT][MAXHOSTNAMELEN]  Array von Hostnamen
                 int  num                               Anzahl der Hosts in 
                                                        der Liste

  Rueckgabewert: -1: Fehler
                  0: kein Fehler

  benutzt:       br_error.c/br_Error ( ... )

                 global/br_nReceivers
                 global/br_Receivers[]
                 global/br_Master
                 global/br_MissingConnections

  Beschreibung:  -initialisiert die Liste der Broadcast-Teilnehmer

*/

int br_ConfigureBroadcast (char (*hosts)[MAXHOSTNAMELEN], int num)
{
        int i;

	srand( getpid() );
	br_BroadcastPort = 1500 + (rand() % 4321 );
 
        br_nReceivers = num; 
        br_MissingConnections = br_nReceivers - 2;

        for( i = 0; i < br_nReceivers; i++ ){
                strcpy( br_Receivers[i].Host, hosts[i] );
                if( br_Receivers[i].Host == NULL ){
                        br_Error( "br_send.c/br_ConfigureBroadcast", EHL );
                        return( ERROR );
                }
                br_Receivers[i].Sock = 0;
                br_Receivers[i].Ready = FALSE;
                br_Receivers[i].Ackned = FALSE;
        }

        if( br_SetMaster( br_Receivers[0].Host ) == -1)
                br_PrintError();

        return( NOERROR );

}



/*

  Funktion:      br_WriteInt ( val )

  Parameter:     int val      Wert, der in den Buffer geschrieben wird

  Rueckgabewert: -

  benutzt:       br_error.c/br_Error ( ... )
                 br_error.c/br_PrintError ( ... )

                 br_send.c/br_FlushBuffer ( ... )

                 global/br_DatabufferPos

  Beschreibung:  -schreibt einen Integer-Wert in den Ausgabe-Buffer
                 -verschickt den Bufferinhalt falls noetig

                 ACHTUNG: Falls -1 zurueckgegeben wird, dann wurde die
                          Integer NICHT mehr mit verschickt, aber in den
                                  =====
                          Ausgabebuffer geschrieben
*/

void br_WriteInt ( int val )
{
#ifdef STATISTICS
   stat_AddInt( &br_BroadcastStatistic );
#endif
   
   if( (br_DatabufferPos + sizeof(int)) < DATABUFSIZE )
   {             
      memcpy((void*)(char*)&(br_Databuffer[br_DatabufferPos]), (void*)
	     (char*)&val, (size_t)sizeof(int) );
      
/* bcopy(  (char*)&val, (char*)&(br_Databuffer[br_DatabufferPos]),
   sizeof(int) ) */
      
      br_DatabufferPos += sizeof(int);
   }
   else
   {
      if( br_FlushBuffer () == -1 )
	 br_PrintError ();
      
      memcpy((void*)(char*)&(br_Databuffer[br_DatabufferPos]), (void*)
	     (char*)&val, (size_t)sizeof(int) );
      /*bcopy(  (char*)&val,
	(char*)&(br_Databuffer[br_DatabufferPos]), sizeof(int) ) */
      br_DatabufferPos += sizeof(int);
   }
}

/*

  Funktion:      br_WriteLong ( val ) (MK)

  Parameter:     long val      Wert, der in den Buffer geschrieben wird

  Rueckgabewert: -

  benutzt:       br_error.c/br_Error ( ... )
                 br_error.c/br_PrintError ( ... )

                 br_send.c/br_FlushBuffer ( ... )

                 global/br_DatabufferPos

  Beschreibung:  -schreibt einen Long-Wert in den Ausgabe-Buffer
                 -verschickt den Bufferinhalt falls noetig

                 ACHTUNG: Falls -1 zurueckgegeben wird, dann wurde die
                          Long NICHT mehr mit verschickt, aber in den
                                  =====
                          Ausgabebuffer geschrieben
*/

void br_WriteLong ( long val )
{
#ifdef STATISTICS
   stat_AddInt( &br_BroadcastStatistic );
#endif

   if( (br_DatabufferPos + sizeof(long)) < DATABUFSIZE )
   {
      memcpy((void *)(char*)&(br_Databuffer[br_DatabufferPos]),
	     (void*)  (char*)&val, (size_t)sizeof(long));

      /* bcopy(  (char*)&val, (char*)&(br_Databuffer[br_DatabufferPos]), sizeof(long) ) */

      br_DatabufferPos += sizeof(long);
   }
   else
   {
      if( br_FlushBuffer () == -1 )
	 br_PrintError ();
      
      memcpy((void *)(char*)&(br_Databuffer[br_DatabufferPos]),
	     (void*)  (char*)&val, (size_t)sizeof(long) );

      /* bcopy(  (char*)&val, (char*)&(br_Databuffer[br_DatabufferPos]), sizeof(long) )*/
      
      br_DatabufferPos += sizeof(long);
   }
}



/*

  Funktion:      br_WriteData ( buf, len )

  Parameter:     char* buf      Buffer, der in den Ausgabebuffer uebertragen wird
                 int   len      Laenge von buf

  Rueckgabewert: -

  benutzt:       br_error.c/br_Error ( ... )
                 br_error.c/br_PrintError ( ... )

                 br_send.c/br_FlushBuffer ( ... )

                 global/br_DatabufferPos

  Beschreibung:  -uebertraegt len Bytes in den Ausgabebuffer
                 -verschickt den Bufferinhalt falls noetig
                 
                 ACHTUNG: Falls -1 zurueckgegeben wird, dann wurde
                          buf NICHT mehr mit verschickt, aber in den
                                 =====
                          Ausgabebuffer geschrieben
*/

void br_WriteData ( char* buf, int len )
{
        int done = 0;           /* by T.L. 10.08.94 */

#ifdef STATISTICS
        stat_AddString( &br_BroadcastStatistic, len );
#endif

/*        if( (br_DatabufferPos + len) < DATABUFSIZE ){

                bcopy( buf, &(br_Databuffer[br_DatabufferPos]), len );
                br_DatabufferPos += len;
        }
        else{
                if( br_FlushBuffer () == -1 )
                        br_PrintError ();

                bcopy( buf, &(br_Databuffer[br_DatabufferPos]), len );
                br_DatabufferPos += len;
        }
}*/ /* replaced by Thomas Leopold on 10.08.94 because only 1 buffer overflow han
dled in this procedure --> ERROR */

        if ((br_DatabufferPos + len) >= DATABUFSIZE)
                if( br_FlushBuffer () == -1 )
                        br_PrintError ();

        while (len > DATABUFSIZE) 
	{
	   memcpy((void*)br_Databuffer, (void*) &(buf[done]),
		  (size_t)DATABUFSIZE);
	   /*bcopy( &(buf[done]), br_Databuffer, DATABUFSIZE)*/
	   
	   done += DATABUFSIZE;
	   len -= DATABUFSIZE;
	   br_DatabufferPos = DATABUFSIZE;
	   if( br_FlushBuffer () == -1 )
	      br_PrintError ();
        }
	memcpy((void*)&(br_Databuffer[br_DatabufferPos]), (void*)
	       &(buf[done]), (size_t)len);
	/* bcopy( &(buf[done]), &(br_Databuffer[br_DatabufferPos]), len) */
        br_DatabufferPos += len;
}



/*

  Funktion:      br_FlushBuffer ()

  Parameter:     -

  Rueckgabewert: -1: Fehler
                 0 : kein Fehler

  benutzt:       br_error.c/br_Error ( ... )
                 br_error.c/br_PrintError ( ... )

                 br_send.c/SendBroadcast ( ... )
                 br_send.c/HandleAckn ( ... )
                 br_send.c/ReceiversReady ( ... )

                 global/br_DatabufferPos
                 global/br_BroadcastNumber
                 global/br_Receivers[]

  Beschreibung:  -verschickt den Bufferinhalt nach Protokoll

*/

int br_FlushBuffer (void)
   
{
   tMessage msg;
   int      packno = 0;
   int      pos = 0;
   tbool    acknok;
   tbool    exit = FALSE;
   int      i;
   tbool    reqackn = FALSE;
   
   
#ifdef STATISTICS
   br_StartWatch( &br_Watch );
#endif
   
   if( br_DatabufferPos == 0 )
      return( NOERROR );
   
   if( !ReceiversReady() )
      br_PrintError();
   
#ifdef TALK
   printf( "All receivers ready\n" );
#endif
   
   while( !exit )
   {
      msg.Type = DataPacket;
      msg.Opcode = Nop;
      msg.Contents.DataPacket.BroadcastNumber = br_BroadcastNumber;
      msg.Contents.DataPacket.PacketNumber = packno;
      msg.Contents.DataPacket.DataEnd = br_DatabufferPos;
      
      memcpy((void*)&(msg.Contents.DataPacket.Data[0]), (void*)
	     &(br_Databuffer[pos]), (size_t)PACKETSIZE ); 
      /* bcopy(  &(br_Databuffer[pos]), &(msg.Contents.DataPacket.Data[0]), PACKETSIZE )*/
      
      pos += PACKETSIZE;
      
      if( pos >= br_DatabufferPos )
      {
	 msg.Opcode = EndOfTransmission;
	 exit = TRUE;
	 reqackn = TRUE;
#ifdef TALK
	 printf(" reached Databuffer end\n");
	 printf(" requesting ackn\n" );
#endif
#ifdef STATISTICS
	 stat_IncAcknReqCount( &br_BroadcastStatistic );
#endif
      }
      else
	 if( ((packno+1) % ACKNnPACKETS) == 0 )
	 {
	    msg.Opcode = SendAckn;
	    reqackn = TRUE;
#ifdef TALK
	    printf(" n Packets transmitted \n");
	    printf(" requesting ackn\n" );
#endif
#ifdef STATISTICS
	    stat_IncAcknReqCount( &br_BroadcastStatistic );
#endif
	 }
      
      if( SendBroadcast( &msg ) == -1 )
	 br_PrintError();
      
      if( reqackn )
      {
	 reqackn = FALSE;
	 
	 i = 0;
	 acknok = FALSE;
	 while( (i <= ACKNREQUESTS) && !acknok )
	 { 
	    if( HandleAckn() == -1 )
	    {
	       if( br_Errorcode == TO )
	       {
		  /*
		    ein Timeout ist aufgetreten, die letzte
		    Nachricht wird wiederholt
		    */
#ifdef TALK 
		  printf( "AcknReq timeouted\n" );
		  printf( "repeating last message\n" );
#endif
		  
		  if( SendBroadcast( &msg ) == -1 )
		     br_PrintError();
		  i++;
	       }
	       else
	       {
		  br_PrintError();
	       }
	    }
	    else
	    {
#ifdef TALK
	       printf( "all ackn ok\n" );
#endif
	       acknok = TRUE;
	       
	    }
	 }
	 
	 if( !acknok )
	 {
	    br_Error( "br_send.c/FlushBuffer", MACK );
	    return( ERROR );
	 }
	 
	 for( i = 0; i < br_nReceivers; i++ )
	 {
	    br_Receivers[i].Ackned = FALSE;
	    
	 }
      }
      
      packno++;
   }
   
   for( i = 0; i < br_nReceivers; i++ )
   {
      br_Receivers[i].Ready = FALSE;
   }
   
#ifdef STATISTICS
   br_StoppWatch( &br_Watch );
   stat_AddTime( &br_BroadcastStatistic, &br_Watch );
#endif
   br_BroadcastNumber++;
#ifdef STATISTICS
   stat_IncBroadcastCount( &br_BroadcastStatistic );
#endif
   
   br_DatabufferPos = 0;
   
   return( NOERROR );
}

                

/*

  Funktion:      HandleAckn ()

  Parameter:     -

  Rueckgabewert: -1: Fehler
                  0: kein Fehler

  benutzt:       br_error.c/br_Error ( ... )
                 br_error.c/br_PrintError ( ... )

                 br_util.c/br_RecvPtP ( ... )
                 br_util.c/br_SendPtP ( ... )

                 global/br_Receivers[]

  Beschreibung:  -sammelt Quittungen der Clients ein und uebertraegt ggf.
                  noch fehlende Pakete

*/

static int HandleAckn (void)


{
   int            i;
   int            j;
   tMessage       ackn;
   tMessage       missing;
   int            n;
   int            pos;
   struct timeval timeout;
   fd_set         client;
   
   
   for( i = 0; i< br_nReceivers; i++ )
   {
      if( (br_Receivers[i].Sock != -1) && (!br_Receivers[i].Ackned) )
      {
	 FD_ZERO( &client );
	 FD_SET( br_Receivers[i].Sock, &client );
	 
	 timeout.tv_sec = TIMEOUTLENGTH;
	 timeout.tv_usec = 0;
	 
	 n = select( br_Receivers[i].Sock+1, &client, (fd_set*)0, (fd_set*)0, &timeout );
	 
	 if( n == 0 )
	 {
	    /*
	      Timeout
	      */
#ifdef STATISTICS
	    stat_AcknReqLost( &br_BroadcastStatistic, i );
#endif
	    br_Error( "br_send.c/HandleAckn", TO );
	    return( ERROR );
	 }
	 
	 if( n == -1 )
	 {
	    br_Error( "br_send.c/HandleAckn", EIS );
	    return( ERROR );
	 }
	 
	 if( br_RecvPtP( br_Receivers[i].Sock, &ackn ) == -1 )
	    br_PrintError();
	 while( ackn.Type != AcknPacket )
	 {
	    if( br_RecvPtP( br_Receivers[i].Sock, &ackn ) == -1 )
	       br_PrintError();
	 }
	 
	 for( j = 1; j <= ackn.Contents.AcknPacket.PacketsMissing[0];
	      j++ )
	 {
	    missing.Type = MissingPacket;
	    pos = ackn.Contents.AcknPacket.PacketsMissing[j] * PACKETSIZE;
	    
	    memcpy((void*)&(missing.Contents.MissingPacket.Data[0]),
		   (void*)  &(br_Databuffer[pos]), (size_t)PACKETSIZE);
/* bcopy(  &(br_Databuffer[pos]), &(missing.Contents.MissingPacket.Data[0]), PACKETSIZE ) */
	    
	    missing.Contents.MissingPacket.BroadcastNumber = br_BroadcastNumber;
	    missing.Contents.MissingPacket.PacketNumber = ackn.Contents.AcknPacket.PacketsMissing[j];
	    
#ifdef TALK
	    printf( "retransmitting packet No. %d/%d\n", br_BroadcastNumber, ackn.Contents.AcknPacket.PacketsMissing[j] );
#endif
#ifdef STATISTICS
	    stat_PacketLost( &br_BroadcastStatistic, i );
	    stat_IncPacketCount( &br_BroadcastStatistic );
#endif
	    
	    if( br_SendPtP( br_Receivers[i].Sock, &missing ) == -1 )
	       br_PrintError();
	    
	 }
	 
	 br_Receivers[i].Ackned = TRUE;
      }
   }
   
   return( NOERROR );
   
}



/*

  Funktion:      SendBroadcast ( msg )

  Parameter:     tMessage* msg      Zeiger auf die zu verschickende Nachricht

  Rueckgabewert: -1: Fehler
                  0: kein Fehler

  benutzt:       br_error.c/br_Error ( ... )

  Beschreibung:  -sendet ein Datagram-Paket ueber den Broadcast-Socket

*/

int SendBroadcast ( tMessage *msg )
{
        int n;
        int len;


#ifdef STATISTICS
        stat_IncPacketCount( &br_BroadcastStatistic );
#endif

        len = sizeof( tMessage );

        while( len ){
                n = sendto( br_Broadcastsock, (char*)msg, sizeof( tMessage ), 0,
                            (struct sockaddr *)&br_BroadcastAddress, sizeof( br_BroadcastAddress ) ); /* 2nd cast by T.L. */

                if( n == -1){
                        br_Error( "br_send.c/br_SendBroadcast", CSB );
                        return( ERROR );
                }
                len -= n;
                /*(char*)msg += n;*/
		msg += n;               /* corrected by T.L. */
        }
        
        return( NOERROR );

}



/*

  Funktion:      ReceiversReady ()

  Parameter:     -

  Rueckgabewert: tbool             TRUE falls alle Empfaenger aufnahmebereit
                                   sind

  benutzt:       br_error.c/br_Error ( ... )
                 br_error.c/br_PrintError ( ... )

                 br_util.c/br_GetHostId ( ... )

                 global/br_Receivers[]


  Beschreibung:  -wartet bis alle Empfaenger Aufnahmebereitschaft signalisiert
                  haben oder bis ein Timeout eingetreten ist

*/

static tbool ReceiversReady (void)

{
        int            readycount = 0;
        int            maxdfpl = 0;
        fd_set         recvs;
        struct timeval timeout;
        tMessage       msg;
        int            i;
        int            n;
        
        
        while( readycount < (br_nReceivers - 1) ){
                FD_ZERO( &recvs );
                for( i = 0; i < br_nReceivers; i++){
                        if( br_Receivers[i].Sock > 0 )
                                FD_SET( br_Receivers[i].Sock, &recvs );

                        if( br_Receivers[i].Sock > maxdfpl )
                                maxdfpl = br_Receivers[i].Sock;
                }

                timeout.tv_sec = RECEIVETIMEOUT; 
                timeout.tv_usec = 0;
/* This timeout lead to problems with large examples... 
                 n = select(maxdfpl+1, &recvs, (fd_set*)0, 
                            (fd_set*)0, &timeout );
*/
                 n = select(maxdfpl+1, &recvs, (fd_set*)0, 
                            (fd_set*)0, NULL );

                if( n == -1 ){
                        br_Error( "br_send.c/ReceiversReady", EIS );
                        return( FALSE );
                }
                if( n == 0 ){
                        br_Error( "br_send.c/ReceiversReady", TO );
                        return( FALSE );
                }

                for( i = 0; i< br_nReceivers; i++ ){
                        if( br_Receivers[i].Sock != -1 )
                                if( FD_ISSET( br_Receivers[i].Sock, &recvs ) ){
                                        if (br_RecvPtP( br_Receivers[i].Sock, &msg ) == -1 )
                                                br_PrintError();
                                        while( msg.Type != CommunicationMessage )
                                        if (br_RecvPtP( br_Receivers[i].Sock, &msg ) == -1 )
                                                br_PrintError();
                                        
                                        if( msg.Opcode != ReadyToReceive )
                                                return( ERROR );

                                        n = br_GetHostId( msg.Contents.CommunicationMessage.Hosts[0] );
                                        br_Receivers[n].Ready = TRUE;
                                        readycount++;
                                }
                };
        };

        return( TRUE );

          
};
