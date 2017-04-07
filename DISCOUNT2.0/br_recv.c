/*

  Modul:        br_recv.c

  benutzt:      br_util.c
                br_error.c

  includes:     br_types.h

                sys/types.h
                sys/param.h
                sys/socket.h
                netinet/in.h
                netdb.h
                net/if.h
                sys/sockio.h (sys/socketio.h under Linux)


  exportiert:   br_OpenRecvBroadcast ( ... )
                br_CloseRecvBroadcast ( ... )
                br_ReadInt ( ... )
                br_ReadLong ( ... )
                br_ReadData ( ... )

  privat:       RecvBroadcast ( ... )
                GetBuffer ( ... )

  Beschreibung: Protokollabwicklung auf Empfaengerseite

  Autor:        Juergen Lind


  Aenderungen seit 01.03.1993: 

  11.03.1993  br_OpenRecvBroadcast umgestellt: Empfaenger schliessen
              sich mittels connect an den Sender an und identifizieren
              sich durch eine Nachricht, die u. a. auch die Prozess-Id
              der Empfaengerprozesse enthaelt

  17.03.1993  ReadData und ReadInt geben keinen Fehlercode mehr zurueck
              sondern brechen das Protokoll direkt ab

  22.03.1993  es werden keine festen Ports mehr verwendet, der Accept-Port
              wird in der Identifikationsnachricht mitgeschickt, die
	      Accept-Ports der anderen kommen dann in einer anderen Nachricht
	      zurueck
	
  08.11.1995  br_ReadLong in Anlehnung an br_ReadInt geschrieben(kopiert) (MK)
  04.12.1996  Uebernahme fuer DISCOUNT 2.0., Aufraeumen (StS)
*/




/*
  Include-Dateien
*/

#include <sys/types.h>
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
#include "br_util.h"            /* added by MK with T.L. to reach ANSI standard */
#include "br_error.h"           /* added by MK with T.L. to reach ANSI standard */
#include "br_recv.h"

#ifdef STATISTICS

#include "br_stat.h"


#endif

#if (defined SVR4) && !(defined LINUX)
#include <sys/systeminfo.h>
#endif

/*
  globale Variablen von br_recv.c
*/

static int   BufferEnd = 0;


/*
  Vorwaertsdeklarationen
*/
static void DoAcknowledge();
static int  RecvBroadcast();
static int  GetBuffer (void);




/*

  Funktion:      br_OpenRecvBroadcast ()

  Parameter:     -

  Rueckgabewert: -1: Fehler
                  0: kein Fehler

  benutzt:       br_util.c/br_GetBroadcastAddress ( ... )
                 br_util.c/br_AcceptPtP ( ... )
                 br_util.c/br_RecvPtP ( ... )

                 br_error.c/br_Error ( ... )
                 br_error.c/br_PrintError ( ... )

                 global/br_Broadcastsock
                 global/BroadccastAddress
                 global/br_Receivers[]
                 global/br_nReceivers
                 global/br_FullConnected
                 global/br_MissingConnections
                 global/br_Hostname
                 global/br_FirstTime
                 global/br_Master
                 global/br_DatabufferPos
                 global/BufferEnd

  Beschreibung:  -oeffnet einen Datagram-Socket zum Empfang von 
                  Broadcastnachrichten (falls nicht schon offen)
                 -br_OpenRecvBroadcast baut schrittweise einen vollstaendig
                  vermaschten Graph von Socketverbindungen zwischen allen
                  Hosts auf (naeheres siehe Dokumentation)
*/

int br_OpenRecvBroadcast (void)

{
   struct sockaddr_in s;
   int                i;
   int                on = 1;
   tMessage           msg;
   tbool              brsockok;
   
   
   if( br_FirstTime )
   {

#if (defined SVR4) && !(defined LINUX)
      sysinfo(SI_HOSTNAME,   br_Hostname, (long)sizeof br_Hostname );
#else
      gethostname(  br_Hostname, sizeof br_Hostname );
#endif
      
      br_Receivers[0].Sock = socket( AF_INET, SOCK_STREAM, 0 );
      if (br_Receivers[0].Sock == 0 )
      {
	 br_Error( "br_recv.c/br_OpenSendBroadcast", CORS );
	 return( ERROR );
      };
      
      if( br_ConnectPtP( br_Receivers[0].Sock, br_Master, br_Receivers[0].Port ) == -1 )
	 return( ERROR );
      
      strcpy( msg.Contents.CommunicationMessage.Hosts[0], br_Hostname );
      msg.Contents.CommunicationMessage.Pid = getpid();
      msg.Contents.CommunicationMessage.Ports[0] = br_AcceptPort;
      
      if( br_SendPtP( br_Receivers[0].Sock, &msg ) == -1 )
	 br_PrintError();
      
      if( br_RecvPtP( br_Receivers[0].Sock, &msg ) == -1 )
	 br_PrintError();
      if( msg.Type != CommunicationMessage ){
	 br_Error( "br_recv.c/br_OpenRecvBroadcast", NE ); 
	 /* 0 replaced by NE by MK with T.L. (C++) */
	 return( ERROR ); 
      };
      
      br_nReceivers = msg.Contents.CommunicationMessage.nReceivers;
      br_MissingConnections = msg.Contents.CommunicationMessage.MissingConnections;
      strcpy( br_Receivers[0].Host, msg.Contents.CommunicationMessage.Hosts[0] );
      
      for( i = 1; i < br_nReceivers; i++ )
      {
	 strcpy( br_Receivers[i].Host, msg.Contents.CommunicationMessage.Hosts[i] );
	 br_Receivers[i].Sock = 0;
	 br_Receivers[i].Ready = FALSE;
	 br_Receivers[i].Port = msg.Contents.CommunicationMessage.Ports[i];
      };
      
      i = br_GetHostId( br_Hostname );
      br_Receivers[i].Sock = -1;
      
      if( br_SetMaster( br_Receivers[0].Host ) == -1)
	 br_PrintError();
      
      br_Broadcastsock = socket( AF_INET, SOCK_DGRAM, 0 );
      if( br_Broadcastsock == -1 ){
	 br_Error( "br_recv.c/br_OpenSendBroadcast", COBS );
	 return( ERROR );
      };
      
      s.sin_family = AF_INET;
      s.sin_addr.s_addr = INADDR_ANY;
      s.sin_port = htons( br_BroadcastPort );
      if( bind( br_Broadcastsock, (struct sockaddr *)&s, sizeof s )
	  == -1 )
      {
	 /* cast by MK with T.L. */
	 msg.Opcode = NotOK;
      }
      else
      {
	 msg.Opcode = OK;
      };
      
      if( br_SendPtP( br_Receivers[0].Sock, &msg ) == -1 )
	 br_PrintError();
      
      if( br_RecvPtP( br_Receivers[0].Sock, &msg ) == -1 )
	 br_PrintError();
      
      if( msg.Opcode == OK )
	 brsockok = TRUE;
      else
	 brsockok = FALSE;
      
      while( !brsockok )
      {
	 if( br_RecvPtP( br_Receivers[0].Sock, &msg ) == -1 )
	    br_PrintError();
	 
	 br_BroadcastPort = msg.Contents.CommunicationMessage.Ports[0];
	 
#ifdef TALK
	 printf( "new BroadcastPort: %d\n", br_BroadcastPort );
#endif
	 
	 s.sin_family = AF_INET;
	 s.sin_addr.s_addr = INADDR_ANY;
	 s.sin_port = htons( br_BroadcastPort );
	 if( bind( br_Broadcastsock, (struct sockaddr *)&s, sizeof s )
	     == -1 )
	 {
	    brsockok = FALSE;
	    msg.Opcode = NotOK;
	 }
	 else{
	    brsockok = TRUE;
	    msg.Opcode = OK;
	 };
	 
	 if( br_SendPtP( br_Receivers[0].Sock, &msg ) == -1 )
	    br_PrintError();
	 
      };
      
      setsockopt( br_Broadcastsock, SOL_SOCKET, SO_BROADCAST, (char *)&on, sizeof on ); /* cast by MK with T.L. */
      
      br_BroadcastAddress.sin_family = AF_INET;
      br_BroadcastAddress.sin_addr.s_addr = br_GetBroadcastAddress( br_Broadcastsock );
      br_BroadcastAddress.sin_port = htons( br_BroadcastPort );
      
      
      br_FirstTime = FALSE;
   }
   
   if( !br_FullConnected )
   {
      i = br_GetHostId( br_Master );
      
      if( br_Receivers[i].Sock == 0 )
      {
	 
	 br_Receivers[i].Sock = socket( AF_INET, SOCK_STREAM, 0 );
	 if (br_Receivers[i].Sock == 0 ){
	    br_Error( "br_recv.c/br_OpenRecvBroadcast", CORS );
	    return( ERROR );
	 };
	 
	 if( br_ConnectPtP( br_Receivers[i].Sock, br_Master, br_Receivers[i].Port ) == -1 )
	    return( ERROR );
	 
	 strcpy( msg.Contents.CommunicationMessage.Hosts[0], br_Hostname );
	 
	 if( br_SendPtP( br_Receivers[i].Sock, &msg ) == -1 )
	    br_PrintError();
	 
	 br_MissingConnections--;
	 if( br_MissingConnections == 0 )
	 {
	    br_FullConnected = TRUE;
	 };
      };
   };
   
   br_DatabufferPos = 0;
   BufferEnd = 0;
   
   return( NOERROR );
   
};




/*
 
  Funktion:      br_CloseRecvBroadcast ()

  Parameter:     -

  Rueckgabewert: -

  benutzt:       br_util.c/br_CloseAcceptSock ( ... )
             
                 global/br_Broadcastsock
                 global/br_Receivers[]

  Beschreibung:  -schliesst den Datagram- und alle Stream-Sockets

*/

int br_CloseRecvBroadcast (void)

{
        int i;


        if( br_CloseAcceptSock() == -1 )
                return( ERROR );

        for( i = 0; i < br_nReceivers; i++ )
                if( br_Receivers[i].Sock > 0 )
                        if( close( br_Receivers[i].Sock ) == -1 )
                                return( ERROR );

        return( NOERROR );

};




/*

  Funktion:      br_ReadInt ( val )

  Parameter:     int* val      Integervariable, in die ein Wert eingelesen wird

  Rueckgabewert: -

  benutzt:       br_error.c/br_PrintError ( ... )

                 br_recv.c/GetBuffer ( ... )

                 global/br_DatabufferPos

  Beschreibung:  -Buffer leer ==> Auf neue Daten warten
                 -liest einen Integer-Wert aus dem Eingabe-Buffer


*/

void br_ReadInt ( int *val )
{
   if( br_DatabufferPos + sizeof(int) > BufferEnd )
   {
      if( GetBuffer() == -1 )
	 br_PrintError ();
      
      memcpy((void*)(char*)val, (void*)
	     (char*)&(br_Databuffer[br_DatabufferPos]),
	     (size_t)sizeof(int));  

      /* bcopy(  (char*)&(br_Databuffer[br_DatabufferPos]), (char*)val, INTSIZE )*/
      br_DatabufferPos += sizeof(int);
      
   }
   
   else
   {
      memcpy((void*)(char*)val, (void*)
	     (char*)&(br_Databuffer[br_DatabufferPos]),
	     (size_t)sizeof(int)) ;
      /*bcopy(  (char*)&(br_Databuffer[br_DatabufferPos]), (char*)val, INTSIZE )*/
      br_DatabufferPos += sizeof(int);
      
   }
};

/*
  
  Funktion:      br_ReadLong ( val )

  Parameter:     long* val      Integervariable, in die ein Wert eingelesen wird
  
  Rueckgabewert: -

  benutzt:       br_error.c/br_PrintError ( ... )

                 br_recv.c/GetBuffer ( ... )

                 global/br_DatabufferPos

  Beschreibung:  -Buffer leer ==> Auf neue Daten warten
                 -liest einen Integer-Wert aus dem Eingabe-Buffer


*/

void br_ReadLong ( long *val )
{
   if( br_DatabufferPos + sizeof(long) > BufferEnd )
   {
      if( GetBuffer() == -1 )
	 br_PrintError ();
      
      memcpy((void*)(char*)val, (void*)
	     (char*)&(br_Databuffer[br_DatabufferPos]),
	     (size_t)sizeof(long));
      /* bcopy(  (char*)&(br_Databuffer[br_DatabufferPos]), (char*)val, LONGSIZE )*/
      
      br_DatabufferPos += sizeof(long);
   }
   else
   {
      memcpy((void*)(char*)val, (void*)
	     (char*)&(br_Databuffer[br_DatabufferPos]),
	     (size_t)sizeof(long));
      /* bcopy(  (char*)&(br_Databuffer[br_DatabufferPos]), (char*)val, LONGSIZE )*/
      br_DatabufferPos += sizeof(long);
      
   }
};



/*

  Funktion:      br_ReadData ( buf, len )

  Parameter:     char* buf      Buffer, in den geschrieben wird
                 int   len      Anzahl der Bytes, die gelesen werden sollen

  Rueckgabewert: -

  benutzt:       br_error.c/br_PrintError ( ... )

                 br_recv.c/GetBuffer ( ... )

                 global/br_DatabufferPos

  Beschreibung:  -Buffer leer ==> Auf neue Daten warten
                 -liest len Bytes aus dem Eingabebuffer


*/

void br_ReadData ( char *buf, int len )
{
   int done = 0;           /* by T.L. 10.08.94 */
   
   /* if( br_DatabufferPos + len > BufferEnd )
      {
      if( GetBuffer () == -1 )
      br_PrintError ();
      #if defined(__SVR4)
      memcpy((void *)(char*)buf, (void *)  (char*)&(br_Databuffer[br_DatabufferPos]), (size_t)len )
      #else
      bcopy(  (char*)&(br_Databuffer[br_DatabufferPos]), (char*)buf, len )
      #endif
      ;
      
      br_DatabufferPos += len;
      
      }
      else{
      #if defined(__SVR4)
      memcpy((void *)(char*)buf, (void *)  (char*)&(br_Databuffer[br_DatabufferPos]), (size_t)len )
      #else
      bcopy(  (char*)&(br_Databuffer[br_DatabufferPos]), (char*)buf, len )
      #endif
      ;
      br_DatabufferPos += len;
      
      } */
   /* corrected by T.L. on 10.08.94 */
   
   if (br_DatabufferPos + len > BufferEnd)
      if( GetBuffer () == -1 )
	 br_PrintError ();
   
   while (len > BufferEnd) 
   {
      memcpy((void*)&(buf[done]), (void*) br_Databuffer,
	     (size_t)BufferEnd);
      /* bcopy( br_Databuffer, &(buf[done]), BufferEnd) */
      
      done += BufferEnd;
      len -= BufferEnd;
      br_DatabufferPos = BufferEnd;
      if( GetBuffer () == -1 )
	 br_PrintError ();
   }
   
   memcpy((void*)&(buf[done]), (void*)
	  &(br_Databuffer[br_DatabufferPos]), (size_t)len); 
   /* bcopy( &(br_Databuffer[br_DatabufferPos]), &(buf[done]), len) */
   
   br_DatabufferPos += len;
}




/*

  Funktion:      GetBuffer ()

  Parameter:     -

  Rueckgabewert: -1: Fehler
                  0: kein Fehler

  benutzt:       br_error.c/br_Error ( ... )
                 br_error.c/br_PrintError ( ... )

                 br_util.c/br_GetConnection ( ... )
                 br_util.c/br_SendPtP ( ... )

                 br_recv.c/RecvBroadcast ( ... )
                 br_recv.c/DoAcknowledge ( ... )

                 global/br_Databuffer
                 global/br_DatabufferPos
                 global/BufferEnd
                 global/br_BroadcastNumber

  Beschreibung:  -empfaengt den Bufferinhalt nach Protokoll

*/

int GetBuffer ()

{
   int      packno = 0;
   tMessage ackn;
   tMessage msg;
   int      pos = 0;
   int      mpos = 0;
   tbool    exit = FALSE;
   int      sock;
   
   
#ifdef TALK
   printf( "entering GetBuffer\n" );
#endif
   sock = br_GetConnection( br_Master );
   if( sock == -1 )
      br_PrintError();
   msg.Type= CommunicationMessage;
   msg.Opcode = ReadyToReceive;
   strcpy( msg.Contents.CommunicationMessage.Hosts[0], br_Hostname );
   if( br_SendPtP( sock, &msg ) == -1 )
      br_PrintError();
   
#ifdef TALK
   printf( "ready to receive\n" );
#endif
   
   ackn.Type = AcknPacket;
   ackn.Contents.AcknPacket.BroadcastNumber = br_BroadcastNumber;
   
   while( !exit )
   {
      
      if( RecvBroadcast( &msg ) == -1 )
	 br_PrintError();                 
      while( msg.Type != DataPacket)
      {
	 if( RecvBroadcast( &msg ) == -1 )
	    br_PrintError();
      };
      
      if( msg.Contents.DataPacket.BroadcastNumber ==
	  br_BroadcastNumber )
      {
	 if( msg.Contents.DataPacket.PacketNumber == packno )
	 {
	    memcpy((void*)&(br_Databuffer[pos]), (void*)
		   &(msg.Contents.DataPacket.Data[0]),
		   (size_t)PACKETSIZE );
	    /* bcopy(  &(msg.Contents.DataPacket.Data[0]),
	       &(br_Databuffer[pos]), PACKETSIZE ) */
	    pos += PACKETSIZE;
	 }
	 else
	 {
	    if( (msg.Contents.DataPacket.PacketNumber > packno) &&
		(msg.Contents.DataPacket.PacketNumber < MAXPACKETS) )
	    {
	       while( packno < msg.Contents.DataPacket.PacketNumber )
	       {
#ifdef TALK
		  printf( "missed packet No.%d/%d\n", br_BroadcastNumber, packno );
#endif
		  ackn.Contents.AcknPacket.PacketsMissing[mpos+1] = packno;
		  mpos++;
		  packno++;
	       };
	       pos = packno * PACKETSIZE;
	       
	       memcpy((void*)&(br_Databuffer[pos]), (void*)
		      &(msg.Contents.DataPacket.Data[0]),
		      (size_t)PACKETSIZE );
	       /* bcopy(  &(msg.Contents.DataPacket.Data[0]), &(br_Databuffer[pos]), PACKETSIZE )*/
	       pos += PACKETSIZE;
	    }
	    else
	    {
	       /* ignore packet */
#ifdef TALK
	       printf( "ignoring packet no. %d/%d\n", br_BroadcastNumber, msg.Contents.DataPacket.PacketNumber );
#endif
	       continue;
	    };
	    
	 };
	 
	 packno++;
	 
	 BufferEnd = msg.Contents.DataPacket.DataEnd;
	 
	 if( msg.Opcode == SendAckn )
	 {
	    ackn.Contents.AcknPacket.PacketsMissing[0] = mpos;
	    mpos = 0;
	    DoAcknowledge( &ackn );
	 };
	 
	 if( msg.Opcode == EndOfTransmission )
	 {
	    ackn.Contents.AcknPacket.PacketsMissing[0] = mpos;
	    mpos = 0;
	    DoAcknowledge( &ackn );
	    exit = TRUE;
	 };
      };
   };
   
   br_BroadcastNumber++;
   
#ifdef STATISTICS
   stat_IncBroadcastCount( &br_BroadcastStatistic );
#endif
   
   br_DatabufferPos = 0;
   return( NOERROR );
}



/*

  Funktion:      DoAcknowledge ( ackn )

  Parameter:     tMessage* ackn     Zeiger auf die vorbereitete Quitung

  Rueckgabewert: -

  benutzt:       br_error.c/br_PrintError ( ... )
      
                 br_util.c/br_GetConnection ( ... )
                 br_util.c/br_RecvPtP ( ... )
                 br_util.c/br_SendPtP ( ... )

  Beschreibung:  -wickelt das Quittieren inklusive dem Empfang der fehlenden
                  Pakete ab

*/

static void DoAcknowledge ( tMessage* ackn )
{
   int       sock;
   int       pos;
   int       i;
   tMessage  missing;
   
   
#ifdef TALK
   printf( "sending ackn BroadcastNumber: %d\n", br_BroadcastNumber );
#endif
   
   sock = br_GetConnection( br_Master );
   if( sock == -1 )
      br_PrintError();
   
   if( br_SendPtP( sock, ackn ) == -1 )
      br_PrintError();
   
   for( i = 1; i <= ackn->Contents.AcknPacket.PacketsMissing[0]; i++ )
   {
      
      if( br_RecvPtP( sock, &missing ) == -1 )
	 br_PrintError();
      while( missing.Type != MissingPacket ){
	 if( br_RecvPtP( sock, &missing ) == -1 )
	    br_PrintError();
      }
      
#ifdef TALK
      printf( "got missing packet No. %d/%d\n", br_BroadcastNumber, missing.Contents.MissingPacket.PacketNumber );
#endif
      
      if( missing.Contents.MissingPacket.BroadcastNumber ==
	  br_BroadcastNumber )
      {
	 pos = missing.Contents.MissingPacket.PacketNumber * PACKETSIZE;
	 
	 memcpy((void*)&( br_Databuffer[pos]), (void*)  &(missing.Contents.MissingPacket.Data[0]), (size_t)PACKETSIZE );
	 /* bcopy(  &(missing.Contents.MissingPacket.Data[0]), &( br_Databuffer[pos]), PACKETSIZE )*/
      };
   };
};



/*

  Funktion:      RecvBroadcast ( msg )

  Parameter:     tMessage* msg     Zeiger auf den Buffer fuer das Datagramm-Paket

  Rueckgabewert: -1: Fehler
                  0: kein Fehler

  benutzt:       br_error.c/br_Error ( ... )

  Beschreibung:  -empfaengt ein Datagram-Paket am Broadcast-Socket

*/

static int RecvBroadcast ( tMessage* msg )
{
   int                n;
   int                len;
   int                addrlen;
   struct sockaddr_in sender;
   
   
   len = sizeof( tMessage );
   addrlen = sizeof( sender );
   
   while( len )
   {
      n = recvfrom( br_Broadcastsock, (char*)msg, sizeof( tMessage ), 0,
		    (struct sockaddr *)&sender, &addrlen ); /* 2nd cast by T.L.  */
      
      if( n == -1)
      {
	 br_Error( "br_recv.c/br_RecvBroadcast", CRB );
	 return( ERROR );
      };
      len -= n;
      /* (char*)msg += n;*/
      msg += n;               /* corrected by T.L. */
   }
   
   return( NOERROR );
   
};



