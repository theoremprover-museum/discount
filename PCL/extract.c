

/*************************************************************************/
/*                                                                       */
/*   File:        extract.c                                              */
/*                                                                       */
/*   Autor:       Stephan Schulz                                         */
/*                                                                       */
/*   Inhalt:      Hauptprogramm zum extrahieren von Beweisen             */
/*                                                                       */
/*   Aenderungen: <1> 17.4.1991 neu                                      */
/*                                                                       */
/*************************************************************************/



#include "pcl_doio.h"
#include "pcl_analyse.h"


/*----------------------------------------------------------------------------*/
/*                      Globale Variable                                      */
/*----------------------------------------------------------------------------*/

StepCell anchor;


/*----------------------------------------------------------------------------*/
/*                       Forward-Deklarationen                                */
/*----------------------------------------------------------------------------*/

void main(int argc,char* argv[]);

void InitIo(int argc,char* argv[],Step_p anchor);

void PrintInfo();

/*----------------------------------------------------------------------------*/
/*                      Funktionen                                            */
/*----------------------------------------------------------------------------*/


/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void main(int argc,char* argv[])                        */
/*                    IN     int argc                                         */
/*                    IN     char* argv[]                                     */
/*                                                                            */
/* Beschreibung     : Steuerung des Programmlaufs.                            */
/*                                                                            */
/* Globale Variable : anchor                                                  */
/*                                                                            */
/* Seiteneffekte    : Bearbeitung                                             */
/*                                                                            */
/* Aenderungen      : <1> 17.4.1991  neu                                      */
/*                                                                            */
/******************************************************************************/


void main(int argc,char* argv[])
{
   anchor.pred = &anchor;
   anchor.succ = &anchor;

   InitIo(argc,argv,&anchor);

   ExtractProof(&anchor);

   PrintOut(&anchor);

   EndIo();
}












/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void InitIo(int argc,char* argv[],Step_p anchor)        */
/*                    IN     int argc                                         */
/*                    IN     char* argv[]                                     */
/*                    IN     Step_p anchor                                    */
/*                                                                            */
/* Beschreibung     : Liest die in der Kommandozeile angegebenen Files und    */
/*                    setzt das Ausgabefile.                                  */
/*                                                                            */
/* Globale Variable : out, in, WriteToFile, ReadFromFile                      */
/*                                                                            */
/* Seiteneffekte    : Lesen der Eingabe, setzen von out.                      */
/*                                                                            */
/* Aenderungen      : <1> 17.4.1991 neu                                       */
/*                    <2> 17.5.1991 Support fuer Luxus-Optionen (-v, -c, -h)  */
/*                    <3> 09.4.1993 Option -i                                 */
/*                                                                            */
/******************************************************************************/



void InitIo(int argc,char* argv[],Step_p anchor)
{
   int  f;

   for(f = 1; f<argc; f++)
   {    /* Teste, ob -o Option gesetzt und korrekt, teste, ob illegale Optionen */
      if(strcmp(argv[f],"-o")==0)
      {
         if(f==argc-1)
         {
            fprintf(stderr,"ERROR: -o expects filename... \n");
            exit(OPTION_ERROR);
         }
         else if(WriteToFile)
         {
            fprintf(stderr,"ERROR: No more than one -o Option... \n");
            exit(OPTION_ERROR);
         }
         else
         {
            OpenOutput(argv[++f]);
         }
      }
      else if(strcmp(argv[f],"-v")==0)
      {
         Verbose = TRUE;
      }
      else if(strcmp(argv[f],"-h")==0)
      {
         Help = TRUE;
      }
      else if(strcmp(argv[f],"-c")==0)
      {
         printcomment = TRUE;
      }
      else if(strcmp(argv[f],"-i")==0)
      {
         ExtIntermed = TRUE;
      }
      else if(strcmp(argv[f],"-l")==0)
      {
         ExtLast = TRUE;
      }
      else if(*(argv[f])=='-')
      {
         fprintf(stderr,"ERROR: unknown option %s...\n",argv[f]);
         exit(OPTION_ERROR);
      }
   }

   if(Help)
   {
      PrintInfo();
      exit(NO_ERROR);
   }

   InitPrinter(out);

   for(f = 1; f<argc; f++)
   {   /* Files oeffnen und bearbeiten  */
      if (strcmp(argv[f],"-o")==0)
      {
         f++;
      }
      else if(*(argv[f])=='-')
      {   /* Optionen ueberspringen  */
      }
      else
      {
         
         VERBOSE(fprintf(stderr,"Reading %s...\n",argv[f]));

         ReadFile(argv[f],anchor);
         
         VERBOSE(fprintf(stderr,"...done.\n"));
      }
   }

   if(!ReadFromFile)
   {   /* lese Standard-Eingabe  */

      VERBOSE(fprintf(stderr,"Reading stdin...\n"));

      InitScanner(in,"stdin");

      ReadIn(anchor);

      VERBOSE(fprintf(stderr,"...done.\n"));
   }
}




/******************************************************************************/
/*                                                                            */
/* FUNCTION         : void PrintInfo()                                        */
/*                                                                            */
/* Beschreibung     : Gibt Informationen ueber das Programm aus.              */
/*                                                                            */
/* Globale Variable : -                                                       */
/*                                                                            */
/* Seiteneffekte    : -                                                       */
/*                                                                            */
/* Aenderungen      : <1> 17.5.1991 neu                                       */
/*                                                                            */
/******************************************************************************/

void PrintInfo()
{
   fprintf(stderr,"\n\n   extract 1.1  vom 16.1.1992");
   fprintf(stderr,"\n\n   Usage: extract [-v] [-h] [-c] [- o outfile] [infile1 ... infileN]");
   fprintf(stderr,"\n\n   Geschrieben von Stephan Schulz");
   fprintf(stderr,"\n\n   extract generiert aus eine PCL-Eingabe eine PCL-Ausgabe,");
   fprintf(stderr,"\n   die nur noch die fuer das Ergebnis notwendigen Schritte");
   fprintf(stderr,"\n   enthaelt. Wird kein Eingabefile angegeben, so wird die");
   fprintf(stderr,"\n   Standard-Eingabe gelesen, mehrere Eingabefiles werden wie");
   fprintf(stderr,"\n   eine zusammenhaengende Datei behandelt.");
   fprintf(stderr,"\n\n    OPTIONEN:");
   fprintf(stderr,"\n\n    -v          Ausgabe von Ablaufinformationen");
   fprintf(stderr,"\n    -h          Ausgabe der Programminformationen, Abbruch.");
   fprintf(stderr,"\n    -c          Uebernahme von Kommentaren im Eingabefile.");
   fprintf(stderr,"\n    -i          Extraktion auch nach den Schritten mit den Typen");
   fprintf(stderr,"\n                tes-intermed, tes-intermedgoal und");
   fprintf(stderr,"\n                crit-intermedgoal, die als Zwischenergebnisse");
   fprintf(stderr,"\n                beim parallelen Beweisen mit dem DISCOUNT-System");
   fprintf(stderr,"\n                entstehen.");
   fprintf(stderr,"\n    -l          Bedingungslose Extraktion des letzen Schrittes");
   fprintf(stderr,"\n                des PCL-Protokols. Dieser wird exakt so");
   fprintf(stderr,"\n                behandelt, als waere er vom Typ tes-final.");
   fprintf(stderr,"\n    -o outfile  Schreibe Ausgabe in outfile. Entfaellt diese");
   fprintf(stderr,"\n                Option, so wird auf die Standard-Ausgabe ge-");
   fprintf(stderr,"\n                schrieben.\n\n\n");
}



/*----------------------------------------------------------------------------*/
/*                         Ende des Files                                     */
/*----------------------------------------------------------------------------*/


