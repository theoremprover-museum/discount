/*-------------------------------------------------------------------------

File        : pcl_minidoio.h 

Autor       : Stephan Schulz

Inhalt      : Ein- und Ausgaberoutinen fuer PCL, Info-Funktionen 

Aenderungen : <1> 16.7.1991 Uebernahme von pcl_doio.c
              <2>  8.3.1994 Neue Funktion: ProtocolSize()

-------------------------------------------------------------------------*/


#include "pcl_minidoio.h"



/*----------------------------------------------------------------------------*/
/*                      Globale Variable                                      */
/*----------------------------------------------------------------------------*/



FILE *out = stdout,
     *in = stdin;

BOOL WriteToFile = FALSE,
     ReadFromFile = FALSE,
     Help = FALSE,
     Statistics = FALSE,
     Verbose = FALSE,
     printcomment = FALSE;

long PclSteps = 0;
long PclIntermeds = 0;
long ProtocolIntermeds = 0;

/*----------------------------------------------------------------------------*/
/*                 Exportierte Funktionen                                     */
/*----------------------------------------------------------------------------*/


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void EndIo()                                            */
/*                                                                            */
/* Beschreibung     : Schliesst alle IO-Streams.                              */
/*                                                                            */
/* Globale Variable : out, WriteToFile                                        */
/*                                                                            */
/* Seiteneffekte    : s.o.                                                    */
/*                                                                            */
/* Aenderungen      : <1> 17.4.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

void EndIo()
{
   if(WriteToFile)
   {
      fclose(out);
   }
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void OpenOutput(char* name)                             */
/*                    IN    char* name                                        */
/*                                                                            */
/* Beschreibung     : Oeffnet das angegebene File, setzt out                  */
/*                                                                            */
/* Globale Variable : WriteToFile, out                                        */
/*                                                                            */
/* Seiteneffekte    : Oeffnen des Files, setzen von out, WriteToFile          */
/*                                                                            */
/* Aenderungen      : <1> 17.4.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

void OpenOutput(char* name)
{
   out = fopen(name,"w");
   if(!out)
   {
      fprintf(stderr,"ERROR: unable to open file %s...\n",name);
      exit(IO_ERROR);
   }
   WriteToFile = TRUE;
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void OpenInput(char* name)                              */
/*                    IN    char* name                                        */
/*                                                                            */
/* Beschreibung     : Oeffnet das angegebene File, setzt in                   */
/*                                                                            */
/* Globale Variable : WriteToFile, in                                         */
/*                                                                            */
/* Seiteneffekte    : Oeffnen des Files, setzen von in, ReadFromFile          */
/*                                                                            */
/* Aenderungen      : <1> 17.4.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

void OpenInput(char* name)
{
   in = fopen(name,"r");
   if(!in)
   {
      fprintf(stderr,"ERROR: unable to open file %s...\n",name);
      exit(IO_ERROR);
   }
   ReadFromFile = TRUE;
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void ReadFile(char* name,miniStep_p anchor)             */
/*                    IN    char* name                                        */
/*                    IN    Step_p anchor                                     */
/*                                                                            */
/* Beschreibung     : Liest die Daten aus dem angegebenen File, die Beweis-   */
/*                    Schritte werden an die bei anchor verankerte Liste      */
/*                    angehaengt.                                             */
/*                                                                            */
/* Globale Variable : WriteToFile, in                                         */
/*                                                                            */
/* Seiteneffekte    : Lesen der Daten                                         */
/*                                                                            */
/* Aenderungen      : <1> 17.4.1991 neu                                       */
/*                    <2> 17.7.1991 Aenderungen fuer miniextract              */
/*                                                                            */
/******************************************************************************/

void ReadFile(char* name, miniStep_p anchor)
{
   OpenInput(name);
   InitScanner(in,name);
   ReadIn(anchor);
   fclose(in);
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void ReadIn(miniStep_p anchor)                          */
/*                    IN     miniStep_p anchor                                */
/*                                                                            */
/* Beschreibung     : Liest PCL ein, haengt neue Schritte an das Ende der bei */
/*                    anchor verankerten Liste an.                            */
/*                                                                            */
/* Globale Variable :                                                         */
/*                                                                            */
/* Seiteneffekte    : Eingabe wird gelesen                                    */
/*                                                                            */
/* Aenderungen      : <1> 16.4.1991 neu                                       */
/*                    <2> 17.7.1991 Aenderungen fuer miniextract              */
/*                                                                            */
/******************************************************************************/

void ReadIn(miniStep_p anchor)
{
   miniStep_p help;

   NextRealToken();
   while(akttoken.token!=NoToken)
   {
      help = ParseStep();

      if(!printcomment)
      {
         FREE(help->comment);
         help->comment = secure_strdup(NullStr);
      }

      help->pred = anchor->pred;
      help->succ = anchor;
      (anchor->pred)->succ = help;
      anchor->pred = help;
      PclSteps++;
      if(help->type == tes_intermed)
      {
	 PclIntermeds++;
      }
   }
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void PrintOut(miniStep_p anchor)                        */
/*                    IN     miniStep_p anchor                                */
/*                                                                            */
/* Beschreibung     : Gibt die bei anchor verankerte Liste von PCL-Schritten  */
/*                    aus.                                                    */
/*                                                                            */
/* Globale Variable :                                                         */
/*                                                                            */
/* Seiteneffekte    : Ausgabe                                                 */
/*                                                                            */
/* Aenderungen      : <1> 16.4.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

void PrintOut(miniStep_p anchor)
{
   miniStep_p help;

   for(help = anchor->succ; help!=anchor; help = help->succ)
   {
      PrintStep(help);
   }
}

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void PrintBack(Step_p anchor)                           */
/*                    IN     miniStep_p anchor                                */
/*                                                                            */
/* Beschreibung     : Gibt die bei anchor verankerte Liste von PCL-Schritten  */
/*                    in umgekehrter Reihenfolge aus.                         */
/*                                                                            */
/* Globale Variable :                                                         */
/*                                                                            */
/* Seiteneffekte    : Ausgabe                                                 */
/*                                                                            */
/* Aenderungen      : <1> 15.5.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

void PrintBack(miniStep_p anchor)
{
   miniStep_p help;

   for(help = anchor->pred; help!=anchor; help = help->pred)
   {
      PrintStep(help);
   }
}


/*-------------------------------------------------------------------------

FUNCTION         : long ProtocolSize(miniStep_p anchor) 

Beschreibung     : Gibt die Laenge (= Schrittzahl) eines PCL-Protocols
                   zurueck. 

Globale Variable : -

Seiteneffekte    : Variable ProtocolIntermed enthaelt Zahl der
                   Schritte mit Typ tes-intermed.

Aenderungen      : <1> 8.3.1993 neu

-------------------------------------------------------------------------*/

long ProtocolSize(miniStep_p anchor) 
{
   miniStep_p handle;
   long count = 0;

   ProtocolIntermeds = 0;

   for(handle = anchor->succ; handle != anchor; handle = handle->succ)
   {
      count++;
      if(handle->type == tes_intermed)
      {
	 ProtocolIntermeds++;
      }
   }
   return count;
}





/*-----------------------------------------------------------------------*/
/*                        Ende des Files                                 */
/*-----------------------------------------------------------------------*/


