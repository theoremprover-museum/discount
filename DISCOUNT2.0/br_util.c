/*

  Modul:        br_util.c

  benutzt:      br_error.c

  includes:     br_types.h

  exportiert:   br_GetBroadcastAddress (...)
                br_OpenAcceptSock ( ... )
                br_CloseAcceptSock ( ... )
                br_SendPtP ( ... )
                br_RecvPtP ( ... )
                br_ConnectPtP ( ... )
                br_AcceptPtP ( ... )
	        br_GetHostId ( ... )
	        br_GetConnection ( ... )
	        br_SetMaster ( ... )
              
  privat:       -

  Beschreibung: enthaelt die Funktionen, die von Sender und Empfaenger
                gleichermassen benutzt werden

  Autor:        Juergen Lind


  Aenderungen seit 01.03.1993: 

  22.03.1993  br_OpenMultiSock, br_CloseMultiSock entfernt, ab jetzt werden
              keine festen Ports mehr verwendet

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

#include <errno.h>
#include "br_types.h"
#include "br_error.h"                   /* added by T.L. to reach ANSI standard */


/*
  globale Variablen
*/
/*
  globale Variablen
*/
tReceiver          br_Receivers[MAXRECEIVERS];
tString            br_Master;
tString            br_Hostname;
tbool              br_FirstTime = TRUE;
tbool              br_FullConnected = FALSE;
int                br_MissingConnections;
int                br_nReceivers = 0;
int                br_Broadcastsock = 0;
struct sockaddr_in br_BroadcastAddress;
char               br_Databuffer[DATABUFSIZE];
int                br_DatabufferPos = 0;
int                br_BroadcastNumber = 0;
int                br_AcceptPort;
int                br_AcceptSock;
int                br_BroadcastPort;



/*

  Funktion:      br_OpenAcceptSock ()

  Parameter:     -

  Rueckgabewert: -1: Fehler
                  0: kein Fehler

  benutzt:       br_error.c/br_Error ( ... )

  Beschreibung:  -oeffnet einen Stream-Socket 'br_AcceptSock' und bindet ihn 
                  an einen Port, die Portnummer steht in 'br_AcceptPort'

*/

int br_OpenAcceptSock (void)

{
	struct sockaddr_in s;
	int                length;


	br_AcceptSock = socket( AF_INET, SOCK_STREAM, 0 );
	if( br_AcceptSock == -1 ){
		br_Error( "br_util.c/OpenAcceptSock", COAS );
		return( ERROR );
	}
	
	s.sin_family      = AF_INET;
	s.sin_addr.s_addr = INADDR_ANY;
	s.sin_port        = 0;
	if( bind( br_AcceptSock, (struct sockaddr*)&s, sizeof s ) == -1 ){
		br_Error( "br_util.c/OpenAcceptSock", CBNAS );
		return( ERROR );
	}
	
	listen( br_AcceptSock, 5 );

	length = sizeof( s );
	if( getsockname( br_AcceptSock, (struct sockaddr*)&s, &length ) == -1 ){
		br_Error( "br_util.c/OpenAcceptSock", CGPN );
		return( ERROR );
	}

	br_AcceptPort = ntohs( s.sin_port );
	
	return( NOERROR );

}
	



/*

  Funktion:      br_CloseAcceptSock ()

  Parameter:     -

  Rueckgabewert: -1: Fehler
                  0: kein Fehler

  benutzt:       br_error.c/br_Error ( ... )

  Beschreibung:  -schliesst einen Stream-Socket

*/

int br_CloseAcceptSock (void)

{
	if( close( br_AcceptSock ) == -1 ){
		br_Error( "br_util.c/CloseAcceptSock", CCAS );
		return( ERROR );
	}

	return( NOERROR );

}
	


/*

  Funktion:      br_ConnectPtP ( sock, host, port )

  Parameter:     int   sock      Deskriptor des sockets, der verbunden 
                                 werden soll
                 char* host      Name des Hosts zu dem eine Verbindung
		                 aufgebaut werden soll
		 int   port      Portnummer auf 'host'
		 
  Rueckgabewert: -1: Fehler
                  0: kein Fehler

  benutzt:       br_error.c/br_Error ( ... )

  Beschreibung:  -stellt auf Senderseite die Verbindung zwischen
                  zwei Stream-Sockets her

*/

int br_ConnectPtP (int sock, char *host, int port)
{
   struct sockaddr_in s;
   struct hostent*    hp;
   
   
   s.sin_family = AF_INET;
   hp = gethostbyname( host );
   if( hp == NULL )
   {
      br_Error( "br_util.c/br_ConnectPtP", NSH );
      return( ERROR );
   }
   /* bcopy( (char*)hp->h_addr, (char*)&s.sin_addr, hp->h_length );*/
   memcpy( (char*)&s.sin_addr, (char*)hp->h_addr, hp->h_length );
   s.sin_port = htons( port );
   
   if( connect( sock, (struct sockaddr*)&s, sizeof s ) == -1 )
   {
      br_Error( "br_util.c/br_ConnectPtP", CCTSH );
      return( ERROR );
   }
   
   return( NOERROR );
   
}
		


/*

  Funktion:      br_AcceptPtP ( sock, newsock )

  Parameter:     int  sock       Deskriptor des sockets, an dem auf eingehende
                                 Verbindungswuensche gewartet wird
		 int* newsock    neuer Socket, ueber den empfangen werden kann

  Rueckgabewert: -1: Fehler
                  0: kein Fehler

  benutzt:       br_error.c/br_Error ( ... )

  Beschreibung:  -stellt auf Empfaengerseite die Verbindung zwischen
                  zwei Stream-Sockets her

*/

int br_AcceptPtP (int sock, int *newsock)
{
	*newsock = accept( sock, NULL, NULL );
	if( *newsock == -1 ){
		br_Error( "br_util.c/br_AcceptPtP", ANP );
		return( ERROR );
	}
	
	return( NOERROR );

}



/*

  Funktion:      br_SendPtP ( sock, msg )

  Parameter:     int       sock      Deskriptor des sockets, ueber den die
                                     Nachricht verschickt werden soll
                 tMessage* msg       Zeiger auf die Nachricht

  Rueckgabewert: -1: Fehler
                  0: kein Fehler

  benutzt:       br_error.c/br_Error ( ... )

  Beschreibung:  -schreibt eine Nachricht auf einen Stream-Socket

*/

int br_SendPtP (int sock, tMessage *msg)
{
	int n;
	int len;


	len = sizeof( tMessage );

	while( len ){
		n = write( sock, (char*)msg, len );
		if( n == -1){
			br_Error( "br_util.c/br_SendPtP", CSM );
			return( ERROR );
		}
		len -= n;
		(char*)msg += n;
	}
	
	return( NOERROR );

}




/*

  Funktion:      br_RecvPtP ( sock, msg )

  Parameter:     int       sock      Deskriptor des sockets, an dem eine 
                                     Nachricht empfangen werden soll
                 tMessage* msg       Buffer in den die Nachricht geschrieben
                                     wird

  Rueckgabewert: -1: Fehler
                  0: kein Fehler

  benutzt:       br_error.c/br_Error ( ... )

  Beschreibung:  -empfaengt eine Nachricht an einem Stream-Socket

*/

int br_RecvPtP (int sock, tMessage *msg)
{
	int n;
	int len;


	len = sizeof( tMessage );

	while( len ){
		n = read( sock, (char*)msg, len );
		if( n == -1){
			br_Error( "br_util.c/br_RecvPtP", CRM );
			return( ERROR );
		}
		len -= n;
		(char*)msg += n;
	}
	
	return( NOERROR );

}




/*

  Funktion:      br_GetBroadcastAddress ( sock )

  Parameter:     int sock      Deskriptor des Sockets fuer dessen lokales
                               Netz die Broadcast-Adresse ermittelt werden soll

  Rueckgabewert: Broadcast-Adresse des lokalen Netzes
                 -1: Fehler

  benutzt:       br_error.c/br_Error ( ... )

  Beschreibung:  -ermittelt die Broadcast-Adresse des lokalen Netzes

*/

long br_GetBroadcastAddress (int sock)
{
   tString         hostname;
   char            buffer[BR_BUFSIZE];
   struct hostent* hp;
   int             ret;
   long            value = 0;
   long            netmask = 0;
   struct ifconf   ifc;
   struct ifreq*   ifp;
   
   if( gethostname( hostname,sizeof hostname ) == -1 )
   {
      br_Error( "br_util.c/GetBroadcastAddress", CGH );
      return( ERROR );
   }
   
   hp = gethostbyname( hostname );
   if( hp->h_length > 4 ){
      br_Error( "br_util.c/GetBroadcastAddress", EGHBN );
      return( ERROR );
   }
   /* bcopy( hp->h_addr, (char*)&value, hp->h_length ); */ /* cast by T.L.  */
   memcpy( (char*)&value, hp->h_addr, hp->h_length );      /* cast by T.L.  */
   
   ifc.ifc_len = BR_BUFSIZE;
   ifc.ifc_buf = buffer;
   ioctl( sock, SIOCGIFCONF, (char *) &ifc );
   
   ifp = ifc.ifc_req;
   ret = ifc.ifc_len / sizeof(struct ifreq);
   while( ret-- && (netmask == 0) ) 
   {
      if( strcmp( ifp->ifr_name, INTERFACENAME ) != 0 ) 
      {
	 ioctl( sock, SIOCGIFNETMASK, (char *) ifp );
	 netmask = ((struct sockaddr_in *)(&(ifp->ifr_addr)))
	    ->sin_addr.s_addr;
      }
      ifp++;
   }
   if( netmask == 0 )
   {
      br_Error( "br_util.c/GetBroadcastAddress", WN );
      return( ERROR );
   }
   
   value = (value & netmask) | (0xffffffff & ~netmask); /* brackets by T.L. */
   
   return value;
}



/*

  Funktion:      GetHostId ( host )

  Parameter:     char* host     Name des gewuenschten Hosts

  Rueckgabewert: Index in 'Receivers'

  benutzt:       -

  Beschreibung:  -liefert den Index eines bestimmten Hosts in der
                  Empfaengerliste

*/



int br_GetHostId ( char *host )
{
   int i = 0;
   
   while( i< br_nReceivers )
   {
      if( strcmp( br_Receivers[i].Host, host ) == 0 )
	 return( i );
      else
	 i++;
   }
   
   return( ERROR );
}



/*

  Funktion:      br_GetConnection ( host )

  Parameter:     char* host

  Rueckgabewert: Socketdeskriptor des mit Host verbundenen Sockets
                 -1 im Fehlerfall (d.h wenn noch keine Verbindung besteht)

  benutzt:       br_util.c/br_GetHostId ( ... )

  Beschreibung:  -ermittelt den Deskriptor einer bestimmten Verbindung

*/

int br_GetConnection ( char *host )
{
   int i;
   
   
   i = br_GetHostId( host );
   if( i == -1 )
      return( ERROR );
   else
      return( br_Receivers[i].Sock );
}




/*

  Funktion:      br_SetMaster ( newmaster )

  Parameter:     char* newmaster

  Rueckgabewert: -1: Fehler
                  0: kein Fehler

  benutzt:       global/br_Master

  Beschreibung:  -stellt den Neuen Teamchef ein

*/

int br_SetMaster ( char *newmaster )
{
   int   i;
   tbool ok = FALSE;
   
   
   for( i = 0; i < br_nReceivers; i++ )
   {
      if( strcmp( br_Receivers[i].Host, newmaster ) == 0 )
	 ok = TRUE;
   }
	
   if( !ok )
      return( ERROR );
   
   strcpy( br_Master, newmaster );
   
   return( NOERROR );

}
			


