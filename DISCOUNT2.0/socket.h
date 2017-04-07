/*
//======================================================================================
//      Projekt:        TEAM-COMPLETION
//
//      Header:         socket
//
//      Author:         Werner Pitz, 1991
//
//      Beschreibung:   Socket-Kommunikation
//-----------------------------------------------------------------------------
//      $Log: socket.h,v $
//
//      Revision 0.1  1993/03/02  09:55:00  lind
//      Kennung fuer Broadcast-Socket vereinbart
//
//      Revision 0.0  1991/08/09  08:18:19  pitz
//      Initiale Version
//
//======================================================================================
*/

#ifndef     __SOCKET
#define     __SOCKET

/*
//-------------------------------------------------------------------------------------
//    Makrodefinitionen
//-------------------------------------------------------------------------------------
*/

#define          ANSI
#define          INTBUFFER       10

#define          BROADCAST -42

#define          Close(sock) close (sock)

extern long      BytesTransfered;

/*
//-------------------------------------------------------------------------------------
//    Funktionsvereinbarungen
//-------------------------------------------------------------------------------------
*/

#ifdef   ANSI

    int         Socket  ( int *port );
    int         Accept  ( int sock );
    int         Connect ( char *host, int port );

    void        WriteStream ( int sock, void *bptr, int len );
    void        ReadStream  ( int sock, void *bptr, int len );
    void        WriteInt    ( int sock, int value );
    int         ReadInt     ( int sock );
    void        WriteString ( int sock, char *string );
    void        ReadString  ( int sock, char *string, int len );

    void        WriteDouble ( int sock, double *d_zahl );
    double      ReadDouble ( int sock );

#endif

#endif
