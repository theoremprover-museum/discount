/*
//======================================================================================
//      Projekt:        TEAM-COMPLETION
//
//      Modul:          socket
//
//      Author:         Werner Pitz, 1991
//
//      Beschreibung:   Socket-Kommunikation
//-----------------------------------------------------------------------------
//      $Log: socket.c,v $
//
//      Revision 0.4  1993/03/17  13:22:12  lind
//      WriteXX, ReadXX liefern keine Fehlercodes mehr
//      => Abfrage entfernt
//
//      Revision 0.3  1993/03/12  14:52:01  lind
//      Socket, Connect, Accept entfernt
//
//      Revision 0.2  1993/03/02  09:52:23  lind
//      Erweiterung auf Broadcasting implementiert
//
//      Revision 0.1  1991/08/19  09:50:23  pitz
//      Fehlermeldungen mit Dateiangabe.
//
//      Revision 0.0  1991/08/09  08:18:19  pitz
//      Initiale Version
//
//======================================================================================
*/

#include    <stdio.h>
#include    <string.h>

#include    <sys/param.h>
#include    <sys/types.h>
#include    <sys/socket.h>
#include    <netinet/in.h>
#include    <netdb.h>


#include    "defines.h"
#include    "error.h"
#include    "socket.h"

#include    "br_types.h"


long        BytesTransfered = 0;

extern tErrorcode br_Errorcode;



/*
//=============================================================================
//      Datenuebertragung
//=============================================================================
*/


/*
//-----------------------------------------------------------------------------
//  Funktion:       WriteStream
//
//  Parameter:      sock    Kennung des Sockets, auf den geschrieben 
//                          werden soll.
//                          ist die Socketkennung BROADCAST, so werden
//                          die Daten an alle Teammitglieder verschickt
//                  bptr    Bufferbereich, in dem die zu sendende
//                          Information steht.
//                  len     Laenge des zu sendenden Datenblocks.
//
//  Beschreibung:   Senden auf eine Socket
//-----------------------------------------------------------------------------
*/

void    WriteStream ( int sock, void *bptr, int len )
{
    int     xlen;


    BytesTransfered += len;

    if( sock == BROADCAST ){
        
        br_WriteData( (char*)bptr, len );
    
    }
    else{
        
        while (len)
        {
            xlen = write ( sock, bptr, len );
            if (xlen < 0)
                Error ( __FILE__ ": "  "WriteStream", "Fehler bei Uebertragung." );
            bptr += xlen;
            len  -= xlen;
        }
    
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       ReadStream
//
//  Parameter:      sock    Kennung des Sockets, von dem empfangen
//                          werden soll.
//                  bptr    Bufferbereich, in dem die zu empfangenen 
//                          Daten abgelegt werden.
//                  len     Laenge der zu empfangenen Daten.
//
//  Beschreibung:   Empfangen von einem Socket.
//-----------------------------------------------------------------------------
*/

void    ReadStream  ( int sock, void *bptr, int len )
{
    int     xlen;

    bzero ( bptr, len );

    if( sock == BROADCAST ){
        
        br_ReadData( (char*)bptr, len );
    
    }
    else{

        while (len)
        {
            xlen = read ( sock, bptr, len );
            if (xlen < 0)
                Error ( __FILE__ ": "  "ReadStream", "Fehler bei Uebertragung." );
            bptr += xlen;
            len  -= xlen;
        }
    
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       WriteInt
//
//  Parameter:      sock    Kennung des Socket
//                          Kennung BROADCAST verschickt an alle
//                  value   Wert, der versendet werden soll.
//
//  Beschreibung:   Senden eines Integerwertes.
//-----------------------------------------------------------------------------
*/

void    WriteInt ( int sock, int value )
{
    int     len;
    int     xlen;
    int*    bptr;

    
    len = sizeof(int);
    BytesTransfered += len;

    if( sock == BROADCAST ){
        
        br_WriteInt( value );
    
    }
    else{
            
        bptr   = &value;

        while (len)
        {
            xlen = write ( sock, (char*)bptr, len );
            if (xlen < 0)
                Error ( __FILE__ ": "  "WriteStream", "Fehler bei Uebertragung." );
            bptr += xlen;
            len  -= xlen;
        }

    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       ReadInt
//
//  Parameter:      sock    Kennung des Socket
//
//  Rueckgabe:      Empfagener Wert
//
//  Beschreibung:   Empfangen eines Integerwertes.
//-----------------------------------------------------------------------------
*/

int     ReadInt ( int sock )
{
    int     value;
    int     xlen;
    int     len;
    int*    bptr;


    if( sock == BROADCAST ){
        
        br_ReadInt( &value );
        return( value );
        
    }
    else{

        len =  sizeof(int);
        bptr = &value;
        bzero ( bptr, len );

        while (len)
        {
            xlen = read ( sock, (char*)bptr, len );
            if (xlen < 0)
                Error ( __FILE__ ": "  "ReadStream", "Fehler bei Uebertragung." );
            bptr += xlen;
            len  -= xlen;
        }

        return( value );
    }
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       WriteString
//
//  Parameter:      sock    Kennung des Socket
//
//  Beschreibung:   Senden eines Strings.
//-----------------------------------------------------------------------------
*/

void    WriteString ( int sock, char *string )
{
    int     len;

    len = strlen ( string );
    WriteInt ( sock, len );
    WriteStream ( sock, string, len+1 );
}


/*
//-----------------------------------------------------------------------------
//  Funktion:       ReadString
//
//  Parameter:      sock    Kennung des Socket
//                  string  Bufferbereich fuer empfangenen String
//                  len     Laenge des Bufferbereichs
//
//  Beschreibung:   Empfangen eines Integerwertes.
//-----------------------------------------------------------------------------
*/

void    ReadString  ( int sock, char *string, int len )
{
    int     xlen;

    xlen = ReadInt (sock);
    if (xlen > len)
        Error ( __FILE__ ": "  "ReadString", "Buffer zu klein !" );

    ReadStream ( sock, string, xlen+1 );
}


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  WriteDouble                                              */
/*                                                                           */
/*  Parameter    :  Kennung des Sockets, auf den geschrieben werden soll     */
/*                  Pointer auf double-Zahl, die geschrieben werden soll     */
/*                                                                           */
/*  Returnwert   :  keine                                                    */
/*                                                                           */
/*  Beschreibung :  Die double-Zahl wird als stream gesendet.                */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

void WriteDouble ( int sock, double *d_zahl )
{
  WriteStream( sock, (void *)d_zahl, sizeof( double ) );
} /* Ende von WriteDouble */


/*****************************************************************************/
/*                                                                           */
/*  Funktion     :  ReadDouble                                               */
/*                                                                           */
/*  Parameter    :  Kennung des Sockets, von dem gelesen werden soll         */
/*                                                                           */
/*  Returnwert   :  gelesene double-Zahl                                     */
/*                                                                           */
/*  Beschreibung :  Die double-Zahl wird als Stream gesendet und dann in eine*/
/*                  double-Zahl umgewandel.                                  */
/*                                                                           */
/*  Globale Var. :  keine                                                    */
/*                                                                           */
/*  Externe Var. :  keine                                                    */
/*                                                                           */
/*****************************************************************************/

double ReadDouble ( int sock )
{
  double d_zahl;

  ReadStream( sock, (void *)&(d_zahl), sizeof( double ) );
  return (  d_zahl  );
} /* Ende von ReadDouble */
