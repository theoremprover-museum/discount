


/*************************************************************************/
/*                                                                       */
/*   File:        pcl_doio.c                                             */
/*                                                                       */
/*   Autor:       Stephan Schulz                                         */
/*                                                                       */
/*   Inhalt:      Ein- und Ausgaberoutinen fuer PCL                      */
/*                                                                       */
/*   Aenderungen: <1> 02.4.1991 neu                                      */
/*                                                                       */
/*************************************************************************/

#include "pcl_doio.h"




/*----------------------------------------------------------------------------*/
/*                      Globale Variable                                      */
/*----------------------------------------------------------------------------*/



FILE *out = stdout,
     *in = stdin;

BOOL WriteToFile = FALSE,
     ReadFromFile = FALSE,
     Help = FALSE,
     Verbose = FALSE,
     printcomment = FALSE;



/*----------------------------------------------------------------------------*/
/*                 Exportierte Funktionen                                     */
/*----------------------------------------------------------------------------*/


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void EndIo()                                            */
/*                                                                            */
/* Beschreibung     : Schliesst den Ausgabe-Stream (fallst noetig)            */
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
/* FUNCTION         : void ReadFile(char* name,Step_p anchor)                 */
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
/*                                                                            */
/******************************************************************************/


void ReadFile(char* name, Step_p anchor)
{
   OpenInput(name);
   InitScanner(in,name);
   ReadIn(anchor);
   fclose(in);
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void ReadIn(Step_p anchor)                              */
/*                    IN     Step_p anchor                                    */
/*                                                                            */
/* Beschreibung     : Liest PCL ein, haengt neue Schritte an das Ende der bei */
/*                    anchor verankerten Liste an.                            */
/*                                                                            */
/* Globale Variable :                                                         */
/*                                                                            */
/* Seiteneffekte    : Eingabe wird gelesen                                    */
/*                                                                            */
/* Aenderungen      : <1> 16.4.1991 neu                                       */
/*                                                                            */
/******************************************************************************/


void ReadIn(Step_p anchor)
{
   Step_p help;

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
   }
}


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void PrintOut(Step_p anchor)                            */
/*                    IN     Step_p anchor                                    */
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

void PrintOut(Step_p anchor)
{
   Step_p help;

   for(help = anchor->succ; help!=anchor; help = help->succ)
   {
      PrintStep(help);
   }
}

/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void PrintBack(Step_p anchor)                           */
/*                    IN     Step_p anchor                                    */
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

void PrintBack(Step_p anchor)
{
   Step_p help;

   for(help = anchor->pred; help!=anchor; help = help->pred)
   {
      PrintStep(help);
   }
}


/*----------------------------------------------------------------------------*/
/*                         Ende des Files                                     */
/*----------------------------------------------------------------------------*/


