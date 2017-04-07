/*

  Modul:        br_error.c

  benutzt:      -

  includes:     br_error.h

  exportiert:   br_Error ( ... )
                br_PrintError ( ... )

                br_Errorcode
                br_Errortext
              
  privat:       -

  Beschreibung: -stellt die Funktionen zur Fehlerbehandlung zur Verfuegung

  Autor:        Juergen Lind


  Aenderungen seit 01.03.1993: 

  02.03.1993  br_PrintError: neue Terminationsprozedur aufrufen

*/


/*
  Include-Dateien
*/
#include <stdio.h>          /* von MK */
#include "br_error.h"


/*
  nicht-exportierte, globale Variablen    MK von T.L. 
*/

static tString br_Errortext;

/*
  exportierte, globale Variablen
*/

tErrorcode br_Errorcode;
const char* ErrorMsgs[]={
        "No Error",
        "Illegal Parameter",
        "Can\' t generate Broadcast-Address",
        "Can\' t open AcceptSock",
        "Can\' t bind name to AcceptSock",
        "Can\' t get port number",
        "Can\' t close AcceptSock",
        "No such Host",
        "Can\' t connect to specified host",
        "Accept not possible",
        "Can\' t send PtP-Message",
        "Can\' t receive PtP-Message",
        "Error in Hostlist",
        "Can\' t open Broadcast socket",
        "Can\' t bind name to Broadcast socket",
        "Can\' t get Hostname",
        "Error in gethostbyname",
        "Wrong Netmask",
        "Can\' t open receiver socket",
        "Can\' t send Broadcast-Message",
        "Can\' t receive Broadcast-Message",
        "Error in Select",
        "Timeout",
        "Missing Acknoledgement",
        "Wrong Messagetype",
        "Can\' t close output file",
        "Can\' t reinstall stdout",
        "Can\' t open output file"
        };

/*
  externe Variablen
*/
void (*Terminate)(void);


/*

  Funktion:      br_Error ( text, code )

  Parameter:     tString    text     vom Anwender bestimmter Zusatztext
                 tErrorcode code     Nummer des Fehlers in 'br_error.h/ErrorMsgs'

  Rueckgabewert: -

  benutzt:       br_error.c/br_Errorcode
                 br_error.c/br_Errortext

  Beschreibung:  -setzt die Variablen 'br_Errorcode' und 'br_Errortext' 

*/

void br_Error ( char *text, tErrorcode code )
{
	
	br_Errorcode = code;
	strcpy ( br_Errortext, text );

} 




/*

  Funktion:      br_PrintError ()

  Parameter:     -

  Rueckgabewert: -

  benutzt:       br_error.c/br_Errorcode
                 br_error.c/br_Errortext
                 
                 br_error.h/ErrorMsgs

  Beschreibung:  -gibt einen Nachricht der Form 
               
                     "Errortext: " ErrorMsgs[Errorcode]
                     "Systemmeldung: perror ()
     
                  aus, 'ErrorMsgs' enthaelt die Fehlermeldungen 
		  des Broadcast-Systems
		 
		  -beendet das Program

*/

void br_PrintError (void)
{

	printf( "\n%s: %s\n", br_Errortext, ErrorMsgs[br_Errorcode] );
	printf(" Check your console for a (probably wrong) system message...\n");
	fprintf(stderr, "\n%s: %s\n", br_Errortext, ErrorMsgs[br_Errorcode] );
        /* fflush(); */ /* removed by MK von T.L. */
	fflush(stderr);

        /* Hier traf ich ein Problem...die eine Haelfte der           */
	/* Fehlermeldung ging auf stdout, die andere auf stderr...ich */
	/* habe es entsprechend ergaenzt...StS                        */
	perror( "Systemmeldung" );
	getchar();
        if (Terminate)
            Terminate ();
	
	exit( -1 );

}




